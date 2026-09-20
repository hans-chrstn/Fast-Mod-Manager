#pragma once

#include "application/InventoryService.hpp"
#include "core/IModScanner.hpp"

#include <expected>
#include <filesystem>
#include <memory>

namespace fmm::application {

class FakeInventoryService : public InventoryService {
public:
  explicit FakeInventoryService(std::shared_ptr<fmm::core::IModScanner> scanner)
      : m_scanner(std::move(scanner)) {}

  [[nodiscard]] auto
  getInventory(const std::stop_token& stoken = {},
               const std::function<void(int, const std::string&)>& progress_callback = {}) const
      -> std::expected<std::vector<fmm::domain::InstalledPackage>, InventoryError> override {
    auto fixture_path = std::filesystem::temp_directory_path() / "fmm_fixture_mods";
    std::error_code error_code;
    std::filesystem::create_directories(fixture_path, error_code);
    if (error_code) {
      return std::unexpected(InventoryError::Internal);
    }

    std::filesystem::create_directories(fixture_path / "Unofficial Patch", error_code);
    std::filesystem::create_directories(fixture_path / "High Res Textures", error_code);
    std::filesystem::create_directories(fixture_path / "UI Overhaul", error_code);
    std::filesystem::create_directories(fixture_path / "Alternate Start", error_code);

    constexpr int dummy_mod_count = 100;
    for (int i = 0; i < dummy_mod_count; ++i) {
      std::filesystem::create_directories(fixture_path / ("Dummy Mod " + std::to_string(i)),
                                          error_code);
    }

    auto result = m_scanner->scanDirectory(fixture_path, stoken, progress_callback);
    if (!result.has_value()) {
      switch (result.error()) {
      case fmm::core::ScanError::DirectoryNotFound:
        return std::unexpected(InventoryError::SourceUnavailable);
      case fmm::core::ScanError::PermissionDenied:
        return std::unexpected(InventoryError::PermissionDenied);
      case fmm::core::ScanError::Cancelled:
        return std::unexpected(InventoryError::Cancelled);
      case fmm::core::ScanError::Unknown:
      default:
        return std::unexpected(InventoryError::Internal);
      }
    }
    return result.value();
  }

private:
  std::shared_ptr<fmm::core::IModScanner> m_scanner;
};

} // namespace fmm::application
