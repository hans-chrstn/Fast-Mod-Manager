#include "infrastructure/JsonProfilePersistence.hpp"

#include <QByteArray>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace fmm::infrastructure {

JsonProfilePersistence::JsonProfilePersistence(std::filesystem::path storage_directory)
    : m_storage_directory(std::move(storage_directory)) {}

auto JsonProfilePersistence::loadMetadata(const domain::ProfileIdentity& identity)
    -> std::expected<domain::ProfileMetadata, application::ports::ProfilePersistenceError> {
  auto file_path = m_storage_directory / std::string(identity.name()) / "metadata.json";

  QFile file(QString::fromStdString(file_path.string()));
  if (!file.exists()) {
    return std::unexpected(application::ports::ProfilePersistenceError::FileNotFound);
  }

  if (!file.open(QIODevice::ReadOnly)) {
    return std::unexpected(application::ports::ProfilePersistenceError::AccessDenied);
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonParseError parse_error;
  QJsonDocument doc = QJsonDocument::fromJson(data, &parse_error);

  if (parse_error.error != QJsonParseError::NoError || !doc.isObject()) {
    return std::unexpected(application::ports::ProfilePersistenceError::InvalidFormat);
  }

  QJsonObject obj = doc.object();

  if (!obj.contains("schema_version") || !obj["schema_version"].isDouble()) {
    return std::unexpected(application::ports::ProfilePersistenceError::InvalidFormat);
  }

  int version = obj["schema_version"].toInt();
  if (version > CURRENT_SCHEMA_VERSION || version <= 0) {
    return std::unexpected(application::ports::ProfilePersistenceError::UnsupportedSchemaVersion);
  }

  if (!obj.contains("description") || !obj["description"].isString()) {
    return std::unexpected(application::ports::ProfilePersistenceError::InvalidFormat);
  }

  domain::ProfileMetadata metadata{.identity = identity,
                                   .schema_version = version,
                                   .description = obj["description"].toString().toStdString()};

  return metadata;
}

auto JsonProfilePersistence::saveMetadata(const domain::ProfileMetadata& metadata)
    -> std::expected<void, application::ports::ProfilePersistenceError> {
  auto profile_dir = m_storage_directory / std::string(metadata.identity.name());

  std::error_code error_code;
  std::filesystem::create_directories(profile_dir, error_code);
  if (error_code) {
    return std::unexpected(application::ports::ProfilePersistenceError::WriteFailed);
  }

  auto file_path = profile_dir / "metadata.json";
  QFile file(QString::fromStdString(file_path.string()));

  if (!file.open(QIODevice::WriteOnly)) {
    return std::unexpected(application::ports::ProfilePersistenceError::AccessDenied);
  }

  QJsonObject obj;
  obj["schema_version"] = metadata.schema_version;
  obj["description"] = QString::fromStdString(metadata.description);

  QJsonDocument doc(obj);
  if (file.write(doc.toJson()) == -1) {
    return std::unexpected(application::ports::ProfilePersistenceError::WriteFailed);
  }

  return {};
}

auto JsonProfilePersistence::loadState(const domain::ProfileIdentity& identity)
    -> std::expected<domain::ProfileState, application::ports::ProfilePersistenceError> {
  auto file_path = m_storage_directory / std::string(identity.name()) / "state.json";

  QFile file(QString::fromStdString(file_path.string()));
  if (!file.exists()) {
    return std::unexpected(application::ports::ProfilePersistenceError::FileNotFound);
  }

  if (!file.open(QIODevice::ReadOnly)) {
    return std::unexpected(application::ports::ProfilePersistenceError::AccessDenied);
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonParseError parse_error;
  QJsonDocument doc = QJsonDocument::fromJson(data, &parse_error);

  if (parse_error.error != QJsonParseError::NoError || !doc.isObject()) {
    return std::unexpected(application::ports::ProfilePersistenceError::InvalidFormat);
  }

  QJsonObject obj = doc.object();
  domain::ProfileState state;

  if (obj.contains("selected_game_id") && obj["selected_game_id"].isString()) {
    auto selected_game_id = obj["selected_game_id"].toString().toStdString();
    if (!selected_game_id.empty()) {
      state.selected_game_id = domain::GameId(std::move(selected_game_id));
    }
  }

  return state;
}

auto JsonProfilePersistence::saveState(const domain::ProfileIdentity& identity,
                                       const domain::ProfileState& state)
    -> std::expected<void, application::ports::ProfilePersistenceError> {
  auto profile_dir = m_storage_directory / std::string(identity.name());

  std::error_code error_code;
  std::filesystem::create_directories(profile_dir, error_code);
  if (error_code) {
    return std::unexpected(application::ports::ProfilePersistenceError::WriteFailed);
  }

  auto file_path = profile_dir / "state.json";
  QFile file(QString::fromStdString(file_path.string()));

  if (!file.open(QIODevice::WriteOnly)) {
    return std::unexpected(application::ports::ProfilePersistenceError::AccessDenied);
  }

  QJsonObject obj;
  obj["selected_game_id"] = state.selected_game_id.has_value()
                                ? QString::fromStdString(state.selected_game_id->value())
                                : QString{};

  QJsonDocument doc(obj);
  if (file.write(doc.toJson()) == -1) {
    return std::unexpected(application::ports::ProfilePersistenceError::WriteFailed);
  }

  return {};
}

} // namespace fmm::infrastructure
