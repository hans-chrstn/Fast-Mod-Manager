#include "FixtureFilesystemScanner.hpp"

#include <chrono>
#include <future>
#include <system_error>
#include <thread>

namespace fmm::infrastructure {

namespace {

auto count_directories(const std::filesystem::path& stagingDirectory, const std::stop_token& stoken)
    -> std::expected<int, core::ScanError> {
  std::error_code error_code;
  int total = 0;
  for (const auto& entry : std::filesystem::directory_iterator(stagingDirectory, error_code)) {
    if (stoken.stop_requested()) {
      return std::unexpected(core::ScanError::Cancelled);
    }
    if (entry.is_directory(error_code)) {
      total++;
    }
  }
  return total;
}

auto simulate_interruptible_delay(const std::stop_token& stoken, int delay_ms) -> bool {
  std::promise<void> promise;
  auto future = promise.get_future();
  std::stop_callback stop_cb(stoken, [&promise]() -> void { promise.set_value(); });

  return future.wait_for(std::chrono::milliseconds(delay_ms)) == std::future_status::ready;
}

auto process_entry(const std::filesystem::directory_entry& entry, int processed,
                   int total_directories, const std::stop_token& stoken,
                   const std::function<void(int, const std::string&)>& progress_callback)
    -> std::expected<std::optional<domain::InstalledPackage>, core::ScanError> {
  auto name = entry.path().filename().string();
  if (progress_callback) {
    constexpr int max_percentage = 100;
    int percentage = total_directories > 0 ? (processed * max_percentage) / total_directories : 0;
    progress_callback(percentage, "Scanning " + name);
  }

  constexpr int simulation_delay_ms = 50;
  if (simulate_interruptible_delay(stoken, simulation_delay_ms)) {
    return std::unexpected(core::ScanError::Cancelled);
  }

  domain::PackageId package_id(name);
  domain::PackageMetadata metadata{.name = name, .version = "1.0", .source = "fixture"};
  domain::PackageLocation location{.staging_path = entry.path(), .object_store_ref = std::nullopt};

  return domain::InstalledPackage(std::move(package_id), std::move(metadata), std::move(location));
}

} // namespace

auto FixtureFilesystemScanner::scanDirectory(
    const std::filesystem::path& stagingDirectory, const std::stop_token& stoken,
    const std::function<void(int, const std::string&)>& progress_callback) const
    -> std::expected<std::vector<domain::InstalledPackage>, core::ScanError> {
  if (!std::filesystem::exists(stagingDirectory) ||
      !std::filesystem::is_directory(stagingDirectory)) {
    return std::unexpected(core::ScanError::DirectoryNotFound);
  }

  if (progress_callback) {
    progress_callback(0, "Counting directories...");
  }

  auto count_result = count_directories(stagingDirectory, stoken);
  if (!count_result.has_value()) {
    return std::unexpected(count_result.error());
  }
  int total_directories = count_result.value();

  std::vector<domain::InstalledPackage> mods;
  int processed = 0;
  std::error_code error_code;

  for (const auto& entry : std::filesystem::directory_iterator(stagingDirectory, error_code)) {
    if (stoken.stop_requested()) {
      return std::unexpected(core::ScanError::Cancelled);
    }
    if (error_code) {
      return std::unexpected(core::ScanError::PermissionDenied);
    }

    if (!entry.is_directory(error_code)) {
      continue;
    }

    auto process_result =
        process_entry(entry, processed, total_directories, stoken, progress_callback);
    if (!process_result.has_value()) {
      return std::unexpected(process_result.error());
    }
    if (process_result.value().has_value()) {
      mods.push_back(std::move(process_result.value().value()));
    }
    processed++;
  }

  if (progress_callback) {
    constexpr int final_percentage = 100;
    progress_callback(final_percentage, "Scan complete");
  }

  if (error_code) {
    return std::unexpected(core::ScanError::Unknown);
  }

  return mods;
}

} // namespace fmm::infrastructure
