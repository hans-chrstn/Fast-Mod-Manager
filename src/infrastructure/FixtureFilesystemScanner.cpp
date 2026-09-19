#include "FixtureFilesystemScanner.hpp"

#include <system_error>

namespace fmm::infrastructure {

auto FixtureFilesystemScanner::scanDirectory(const std::filesystem::path& stagingDirectory) const
    -> std::expected<std::vector<domain::ModIdentity>, core::ScanError> {
  if (!std::filesystem::exists(stagingDirectory)) {
    return std::unexpected(core::ScanError::DirectoryNotFound);
  }
  if (!std::filesystem::is_directory(stagingDirectory)) {
    return std::unexpected(core::ScanError::DirectoryNotFound);
  }

  std::vector<domain::ModIdentity> mods;
  std::error_code error_code;

  for (const auto& entry : std::filesystem::directory_iterator(
           stagingDirectory, std::filesystem::directory_options::skip_permission_denied,
           error_code)) {
    if (error_code) {
      return std::unexpected(core::ScanError::PermissionDenied);
    }

    if (entry.is_directory(error_code)) {
      auto name = entry.path().filename().string();
      auto mod_identity = domain::ModIdentity::create(name, entry.path());
      if (mod_identity.has_value()) {
        mods.push_back(std::move(mod_identity.value()));
      }
    }
  }

  if (error_code) {
    return std::unexpected(core::ScanError::Unknown);
  }

  return mods;
}

} // namespace fmm::infrastructure
