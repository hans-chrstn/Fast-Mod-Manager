#include "infrastructure/StagingRepositoryImpl.hpp"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <random>

using namespace fmm::domain;
using namespace fmm::application::ports;
using namespace fmm::infrastructure;

namespace {
auto create_temp_dir() -> std::filesystem::path {
  std::random_device random_dev;
  std::mt19937 gen(random_dev());
  constexpr int min_rand = 10000;
  constexpr int max_rand = 99999;
  std::uniform_int_distribution<> dis(min_rand, max_rand);
  auto path =
      std::filesystem::temp_directory_path() / ("fmm_test_staging_" + std::to_string(dis(gen)));
  std::filesystem::create_directories(path);
  return path;
}

auto create_dummy_file(const std::filesystem::path& path, std::size_t size) -> void {
  std::ofstream out(path, std::ios::binary);
  std::vector<char> data(size, 'A');
  out.write(data.data(), static_cast<std::streamsize>(data.size()));
}
} // namespace

TEST_CASE("StagingRepositoryImpl enforces policy and stages files", "[infrastructure][staging]") {
  auto temp_dir = create_temp_dir();
  StagingRepositoryImpl repo(temp_dir / "staging");

  SECTION("Staging non-existent file returns FileNotFound") {
    auto result = repo.stagePackage(temp_dir / "missing.zip", StagingPolicy{});
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == ImportError::FileNotFound);
  }

  SECTION("Staging oversized file returns ExceedsSizeLimit") {
    auto file_path = temp_dir / "large.zip";
    create_dummy_file(file_path, 2048);

    StagingPolicy policy;
    policy.max_archive_size_bytes = 1024; // 1 KB max
    policy.allowed_extensions = {".zip"};

    auto result = repo.stagePackage(file_path, policy);
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == ImportError::ExceedsSizeLimit);
  }

  SECTION("Staging file with unsupported extension returns UnsupportedFormat") {
    auto file_path = temp_dir / "mod.exe";
    create_dummy_file(file_path, 1024);

    StagingPolicy policy;
    policy.allowed_extensions = {".zip", ".7z"};

    auto result = repo.stagePackage(file_path, policy);
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == ImportError::UnsupportedFormat);
  }

  SECTION("Successfully stages valid file and generates checksum") {
    auto file_path = temp_dir / "mod.zip";
    create_dummy_file(file_path, 1024);

    StagingPolicy policy;
    policy.require_checksum = true;
    policy.allowed_extensions = {".zip"};

    auto result = repo.stagePackage(file_path, policy);
    REQUIRE(result.has_value());

    const auto& installed = result.value();
    REQUIRE(installed.metadata().name == "mod");
    REQUIRE(installed.location().staging_path.has_value());
    REQUIRE(installed.location().object_store_ref.has_value());
    if (installed.location().staging_path) {
      REQUIRE(std::filesystem::exists(*installed.location().staging_path));
    } else {
      FAIL("Missing staging path");
    }
  }

  std::filesystem::remove_all(temp_dir);
}
