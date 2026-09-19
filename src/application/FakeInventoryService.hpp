#pragma once

#include "application/InventoryService.hpp"
#include "core/IModScanner.hpp"

#include <filesystem>
#include <memory>

namespace application {

class FakeInventoryService : public InventoryService {
public:
  explicit FakeInventoryService(std::shared_ptr<fmm::core::IModScanner> scanner)
      : m_scanner(std::move(scanner)) {}

  [[nodiscard]] auto
  getInventory(const std::stop_token& stoken = {},
               const std::function<void(int, const std::string&)>& progress_callback = {}) const
      -> std::vector<fmm::domain::ModIdentity> override {
    auto fixture_path = std::filesystem::temp_directory_path() / "fmm_fixture_mods";
    std::filesystem::create_directories(fixture_path);

    std::filesystem::create_directories(fixture_path / "Unofficial Patch");
    std::filesystem::create_directories(fixture_path / "High Res Textures");
    std::filesystem::create_directories(fixture_path / "UI Overhaul");
    std::filesystem::create_directories(fixture_path / "Alternate Start");

    constexpr int dummy_mod_count = 100;
    for (int i = 0; i < dummy_mod_count; ++i) {
      std::filesystem::create_directories(fixture_path / ("Dummy Mod " + std::to_string(i)));
    }

    auto result = m_scanner->scanDirectory(fixture_path, stoken, progress_callback);
    if (result.has_value()) {
      return result.value();
    }
    return {};
  }

private:
  std::shared_ptr<fmm::core::IModScanner> m_scanner;
};

} // namespace application
