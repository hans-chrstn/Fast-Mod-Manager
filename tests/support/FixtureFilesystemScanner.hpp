#pragma once

#include "core/IModScanner.hpp"

#include <filesystem>
#include <vector>

namespace fmm::test_support {

class FixtureFilesystemScanner final : public core::IModScanner {
public:
  FixtureFilesystemScanner() = default;
  ~FixtureFilesystemScanner() override = default;

  [[nodiscard]] auto
  scanDirectory(const std::filesystem::path& stagingDirectory, const std::stop_token& stoken = {},
                const std::function<void(int, const std::string&)>& progress_callback = {}) const
      -> std::expected<std::vector<domain::InstalledPackage>, core::ScanError> override;
};

} // namespace fmm::test_support
