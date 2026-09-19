#include "infrastructure/JsonProfilePersistence.hpp"

#include <QDir>
#include <QFile>
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <random>

using namespace fmm::domain;
using namespace fmm::infrastructure;

namespace {
auto create_temp_dir() -> std::filesystem::path {
  std::random_device random_dev;
  std::mt19937 gen(random_dev());
  constexpr int min_rand = 10000;
  constexpr int max_rand = 99999;
  std::uniform_int_distribution<> dis(min_rand, max_rand);
  auto path =
      std::filesystem::temp_directory_path() / ("fmm_test_persistence_" + std::to_string(dis(gen)));
  std::filesystem::create_directories(path);
  return path;
}
} // namespace

TEST_CASE("JsonProfilePersistence schema and storage", "[infrastructure][persistence]") {
  auto temp_dir = create_temp_dir();
  JsonProfilePersistence persistence(temp_dir);
  auto identity = ProfileIdentity::create("TestProfile").value();

  SECTION("Loading non-existent file returns FileNotFound") {
    auto result = persistence.load(identity);
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == ProfilePersistenceError::FileNotFound);
  }

  SECTION("Saving and loading successfully preserves schema") {
    ProfileMetadata meta{.identity = identity,
                         .schema_version = JsonProfilePersistence::CURRENT_SCHEMA_VERSION,
                         .description = "My test description"};
    auto save_result = persistence.save(meta);
    REQUIRE(save_result.has_value());

    auto load_result = persistence.load(identity);
    REQUIRE(load_result.has_value());
    REQUIRE(load_result->identity == meta.identity);
    REQUIRE(load_result->schema_version == meta.schema_version);
    REQUIRE(load_result->description == meta.description);
  }

  SECTION("Loading malformed JSON returns InvalidFormat") {
    std::filesystem::create_directories(temp_dir / "TestProfile");
    std::ofstream out(temp_dir / "TestProfile" / "metadata.json");
    out << "{ invalid json ";
    out.close();

    auto result = persistence.load(identity);
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == ProfilePersistenceError::InvalidFormat);
  }

  SECTION("Loading unsupported schema version returns UnsupportedSchemaVersion") {
    std::filesystem::create_directories(temp_dir / "TestProfile");
    std::ofstream out(temp_dir / "TestProfile" / "metadata.json");
    out << R"({ "schema_version": 999, "description": "Future" })";
    out.close();

    auto result = persistence.load(identity);
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == ProfilePersistenceError::UnsupportedSchemaVersion);
  }

  std::filesystem::remove_all(temp_dir);
}
