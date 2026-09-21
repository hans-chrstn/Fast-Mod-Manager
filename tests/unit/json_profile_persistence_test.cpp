#include "infrastructure/JsonProfilePersistence.hpp"

#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
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
    auto result = persistence.loadMetadata(identity);
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == fmm::application::ports::ProfilePersistenceError::FileNotFound);
  }

  SECTION("Saving and loading successfully preserves schema") {
    ProfileMetadata meta{.identity = identity,
                         .schema_version = JsonProfilePersistence::CURRENT_SCHEMA_VERSION,
                         .description = "My test description"};
    auto save_result = persistence.saveMetadata(meta);
    REQUIRE(save_result.has_value());

    auto load_result = persistence.loadMetadata(identity);
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

    auto result = persistence.loadMetadata(identity);
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == fmm::application::ports::ProfilePersistenceError::InvalidFormat);
  }

  SECTION("Loading unsupported schema version returns UnsupportedSchemaVersion") {
    std::filesystem::create_directories(temp_dir / "TestProfile");
    std::ofstream out(temp_dir / "TestProfile" / "metadata.json");
    out << R"({ "schema_version": 999, "description": "Future" })";
    out.close();

    auto result = persistence.loadMetadata(identity);
    REQUIRE(!result.has_value());
    REQUIRE(result.error() ==
            fmm::application::ports::ProfilePersistenceError::UnsupportedSchemaVersion);
  }

  SECTION("Saving and loading state successfully preserves state") {
    ProfileState state;
    state.selected_game_id = GameId{"test_game"};
    auto save_result = persistence.saveState(identity, state);
    REQUIRE(save_result.has_value());

    auto load_result = persistence.loadState(identity);
    REQUIRE(load_result.has_value());
    REQUIRE(load_result->selected_game_id == GameId{"test_game"});
  }

  SECTION("State JSON keeps the selected game string representation") {
    ProfileState state{.selected_game_id = GameId{"test_game"}};
    REQUIRE(persistence.saveState(identity, state).has_value());

    QFile file(QString::fromStdString((temp_dir / "TestProfile" / "state.json").string()));
    REQUIRE(file.open(QIODevice::ReadOnly));
    const auto document = QJsonDocument::fromJson(file.readAll());
    REQUIRE(document.isObject());
    REQUIRE(document.object()["selected_game_id"].isString());
    REQUIRE(document.object()["selected_game_id"].toString() == "test_game");
  }

  SECTION("Unselected state preserves the empty string representation") {
    REQUIRE(persistence.saveState(identity, ProfileState{}).has_value());

    QFile file(QString::fromStdString((temp_dir / "TestProfile" / "state.json").string()));
    REQUIRE(file.open(QIODevice::ReadOnly));
    const auto document = QJsonDocument::fromJson(file.readAll());
    REQUIRE(document.isObject());
    REQUIRE(document.object()["selected_game_id"].isString());
    REQUIRE(document.object()["selected_game_id"].toString().isEmpty());

    auto load_result = persistence.loadState(identity);
    REQUIRE(load_result.has_value());
    REQUIRE_FALSE(load_result->selected_game_id.has_value());
  }

  SECTION("Missing and empty selected game values load as no selection") {
    const auto profile_dir = temp_dir / "TestProfile";
    std::filesystem::create_directories(profile_dir);

    {
      std::ofstream out(profile_dir / "state.json");
      out << R"({})";
    }
    auto missing_result = persistence.loadState(identity);
    REQUIRE(missing_result.has_value());
    REQUIRE_FALSE(missing_result->selected_game_id.has_value());

    {
      std::ofstream out(profile_dir / "state.json");
      out << R"({ "selected_game_id": "" })";
    }
    auto empty_result = persistence.loadState(identity);
    REQUIRE(empty_result.has_value());
    REQUIRE_FALSE(empty_result->selected_game_id.has_value());
  }

  std::filesystem::remove_all(temp_dir);
}
