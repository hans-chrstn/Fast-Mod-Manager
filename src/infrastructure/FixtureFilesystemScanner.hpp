#pragma once

#include "core/IModScanner.hpp"

#include <filesystem>
#include <vector>

namespace fmm::infrastructure {

class FixtureFilesystemScanner final : public core::IModScanner {
public:
  FixtureFilesystemScanner() = default;
  ~FixtureFilesystemScanner() override = default;

  [[nodiscard]] auto scanDirectory(const std::filesystem::path& stagingDirectory) const
      -> std::expected<std::vector<domain::ModIdentity>, core::ScanError> override;
};

} // namespace fmm::infrastructure
