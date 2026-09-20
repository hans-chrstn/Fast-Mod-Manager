#include "infrastructure/StagingRepositoryImpl.hpp"

#include <QCryptographicHash>
#include <QFile>
#include <QString>
#include <algorithm>
#include <chrono>
#include <random>
#include <system_error>

namespace fmm::infrastructure {

StagingRepositoryImpl::StagingRepositoryImpl(std::filesystem::path staging_root)
    : m_staging_root(std::move(staging_root)) {
  std::error_code error_code;
  std::filesystem::create_directories(m_staging_root, error_code);
}

auto StagingRepositoryImpl::stagePackage(const std::filesystem::path& source_path,
                                         const domain::StagingPolicy& policy,
                                         const ProgressCallback& progress)
    -> std::expected<domain::InstalledPackage, domain::ImportError> {

  if (!std::filesystem::exists(source_path)) {
    return std::unexpected(domain::ImportError::FileNotFound);
  }

  std::error_code error_code;
  auto file_size = std::filesystem::file_size(source_path, error_code);
  if (error_code) {
    return std::unexpected(domain::ImportError::AccessDenied);
  }

  if (file_size > policy.max_archive_size_bytes) {
    return std::unexpected(domain::ImportError::ExceedsSizeLimit);
  }

  auto extension = source_path.extension().string();
  std::ranges::transform(extension, extension.begin(), [](unsigned char character) -> char {
    return static_cast<char>(std::tolower(character));
  });

  bool is_allowed =
      std::ranges::any_of(policy.allowed_extensions,
                          [&](const std::string& allowed) -> bool { return extension == allowed; });

  if (!is_allowed) {
    return std::unexpected(domain::ImportError::UnsupportedFormat);
  }

  QFile input_file(QString::fromStdString(source_path.string()));
  if (!input_file.open(QIODevice::ReadOnly)) {
    return std::unexpected(domain::ImportError::AccessDenied);
  }

  std::random_device random_dev;
  std::mt19937 generator(random_dev());
  std::uniform_int_distribution<std::uint64_t> distribution;
  std::string unique_id =
      std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "_" +
      std::to_string(distribution(generator));

  auto destination_dir = m_staging_root / unique_id;
  std::filesystem::create_directories(destination_dir, error_code);
  if (error_code) {
    return std::unexpected(domain::ImportError::StagingFailed);
  }

  auto destination_path = destination_dir / source_path.filename();
  QFile output_file(QString::fromStdString(destination_path.string()));
  if (!output_file.open(QIODevice::WriteOnly)) {
    std::filesystem::remove_all(destination_dir, error_code);
    return std::unexpected(domain::ImportError::StagingFailed);
  }

  QCryptographicHash hash(QCryptographicHash::Sha256);
  constexpr qint64 BUFFER_SIZE = 1024LL * 1024LL;
  qint64 total_read = 0;

  while (!input_file.atEnd()) {
    QByteArray buffer = input_file.read(BUFFER_SIZE);
    if (buffer.isEmpty()) {
      break;
    }

    if (policy.require_checksum) {
      hash.addData(buffer);
    }

    qint64 written = output_file.write(buffer);
    if (written != buffer.size()) {
      input_file.close();
      output_file.close();
      std::filesystem::remove_all(destination_dir, error_code);
      return std::unexpected(domain::ImportError::StagingFailed);
    }

    total_read += buffer.size();
    if (progress) {
      progress(static_cast<std::uintmax_t>(total_read), file_size);
    }
  }

  input_file.close();
  output_file.close();

  std::string final_checksum;
  if (policy.require_checksum) {
    final_checksum = hash.result().toHex().toStdString();
  }

  domain::PackageId package_id{unique_id};
  domain::PackageMetadata metadata{
      .name = source_path.stem().string(), .version = "1.0.0", .source = "staged"};
  domain::PackageLocation location{.staging_path = destination_path,
                                   .object_store_ref = final_checksum.empty()
                                                           ? std::nullopt
                                                           : std::make_optional(final_checksum)};

  return domain::InstalledPackage(std::move(package_id), std::move(metadata), std::move(location));
}

} // namespace fmm::infrastructure
