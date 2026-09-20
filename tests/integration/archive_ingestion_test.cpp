#include "domain/VirtualTree.hpp"
#include "infrastructure/LibArchiveReader.hpp"
#include "infrastructure/StagingRepositoryImpl.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <filesystem>

using namespace fmm::domain;
using namespace fmm::application::ports;
using namespace fmm::infrastructure;
using namespace fmm::core;

namespace {
auto get_fixtures_dir() -> std::filesystem::path {
  const char* env_dir = std::getenv("FIXTURES_DIR");
  if (env_dir != nullptr) {
    return {env_dir};
  }
  return std::filesystem::current_path() / "fixtures" / "corpus";
}

auto get_staging_dir() -> std::filesystem::path {
  auto path = std::filesystem::temp_directory_path() / "fmm_integration_staging";
  std::filesystem::create_directories(path);
  return path;
}
} // namespace

TEST_CASE("Archive Ingestion Integration - Valid Simple", "[integration][ingestion]") {
  auto fixtures = get_fixtures_dir();
  auto staging = get_staging_dir();
  auto target_zip = fixtures / "valid_simple.zip";

  if (!std::filesystem::exists(target_zip)) {
    WARN("Fixture not found, skipping: " << target_zip.string());
    return;
  }

  StagingRepositoryImpl staging_repo(staging);
  StagingPolicy policy;

  auto stage_result = staging_repo.stagePackage(target_zip, policy);
  REQUIRE(stage_result.has_value());

  const auto& installed = stage_result.value();
  REQUIRE(installed.location().staging_path.has_value());

  if (installed.location().staging_path) {
    LibArchiveReader reader;
    auto tree_result = reader.readTree(*installed.location().staging_path);
    REQUIRE(tree_result.has_value());

    auto tree = tree_result.value();
    bool has_test_txt = false;
    bool has_texture_dds = false;
    for (const auto& child : tree.root().children) {
      if (child.name == "test.txt") {
        has_test_txt = true;
      } else if (child.name == "textures") {
        for (const auto& sub : child.children) {
          if (sub.name == "texture.dds") {
            has_texture_dds = true;
          }
        }
      }
    }
    REQUIRE(has_test_txt);
    REQUIRE(has_texture_dds);
  } else {
    FAIL("Missing staging path");
  }

  std::filesystem::remove_all(staging);
}

TEST_CASE("Archive Ingestion Integration - Corrupt Magic", "[integration][ingestion]") {
  auto fixtures = get_fixtures_dir();
  auto staging = get_staging_dir();
  auto target_zip = fixtures / "corrupt_magic.zip";

  if (!std::filesystem::exists(target_zip)) {
    WARN("Fixture not found, skipping: " << target_zip.string());
    return;
  }

  StagingRepositoryImpl staging_repo(staging);
  auto stage_result = staging_repo.stagePackage(target_zip, StagingPolicy{});
  REQUIRE(stage_result.has_value());

  const auto& installed = stage_result.value();

  if (installed.location().staging_path) {
    LibArchiveReader reader;
    auto tree_result = reader.readTree(*installed.location().staging_path);
    REQUIRE(!tree_result.has_value());
    REQUIRE(tree_result.error() == ArchiveError::CorruptHeader);
  } else {
    FAIL("Missing staging path");
  }

  std::filesystem::remove_all(staging);
}

TEST_CASE("Archive Ingestion Integration - Traversal Malicious", "[integration][ingestion]") {
  auto fixtures = get_fixtures_dir();
  auto staging = get_staging_dir();
  auto target_zip = fixtures / "traversal_malicious.zip";

  if (!std::filesystem::exists(target_zip)) {
    WARN("Fixture not found, skipping: " << target_zip.string());
    return;
  }

  StagingRepositoryImpl staging_repo(staging);
  auto stage_result = staging_repo.stagePackage(target_zip, StagingPolicy{});
  REQUIRE(stage_result.has_value());

  const auto& installed = stage_result.value();

  if (installed.location().staging_path) {
    LibArchiveReader reader;
    auto tree_result = reader.readTree(*installed.location().staging_path);

    if (tree_result.has_value()) {
      REQUIRE(tree_result.value().root().children.empty());
    } else {
      REQUIRE(tree_result.error() == ArchiveError::InvalidFormat);
    }
  } else {
    FAIL("Missing staging path");
  }

  std::filesystem::remove_all(staging);
}

TEST_CASE("Archive Ingestion Integration - Oversized Mock", "[integration][ingestion]") {
  auto fixtures = get_fixtures_dir();
  auto staging = get_staging_dir();
  auto target_zip = fixtures / "oversized_mock.zip";

  if (!std::filesystem::exists(target_zip)) {
    WARN("Fixture not found, skipping: " << target_zip.string());
    return;
  }

  StagingRepositoryImpl staging_repo(staging);
  StagingPolicy policy;
  auto stage_result = staging_repo.stagePackage(target_zip, policy);
  REQUIRE(!stage_result.has_value());
  REQUIRE(stage_result.error() == ImportError::ExceedsSizeLimit);

  std::filesystem::remove_all(staging);
}
