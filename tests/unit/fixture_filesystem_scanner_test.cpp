#include "infrastructure/FixtureFilesystemScanner.hpp"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>

namespace {
void create_dummy_file(const std::filesystem::path& p) {
  std::ofstream f(p);
  f << "dummy";
}
} // namespace

TEST_CASE("FixtureFilesystemScanner correctly identifies mod directories",
          "[infrastructure][scanner]") {
  fmm::infrastructure::FixtureFilesystemScanner scanner;

  auto temp_dir = std::filesystem::temp_directory_path() / "fmm_test_staging";
  std::filesystem::create_directories(temp_dir);

  SECTION("Returns DirectoryNotFound for missing staging directory") {
    auto result = scanner.scanDirectory(temp_dir / "nonexistent");
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == fmm::core::ScanError::DirectoryNotFound);
  }

  SECTION("Scans valid directories and ignores files") {
    auto mod1 = temp_dir / "TestMod1";
    auto mod2 = temp_dir / "TestMod2";
    auto loose_file = temp_dir / "ignore_me.txt";

    std::filesystem::create_directories(mod1);
    std::filesystem::create_directories(mod2);
    create_dummy_file(loose_file);

    auto result = scanner.scanDirectory(temp_dir);
    REQUIRE(result.has_value());

    auto mods = result.value();
    REQUIRE(mods.size() == 2);

    bool found1 = false;
    bool found2 = false;
    for (const auto& mod : mods) {
      if (mod.name() == "TestMod1") {
        found1 = true;
      }
      if (mod.name() == "TestMod2") {
        found2 = true;
      }
    }
    REQUIRE(found1);
    REQUIRE(found2);

    std::filesystem::remove_all(mod1);
    std::filesystem::remove_all(mod2);
    std::filesystem::remove(loose_file);
  }

  std::filesystem::remove_all(temp_dir);
}
