#pragma once

#include "domain/ProfileMetadata.hpp"

#include <filesystem>

namespace fmm::infrastructure {

class JsonProfilePersistence final : public domain::ProfilePersistencePort {
public:
  static constexpr int CURRENT_SCHEMA_VERSION = 1;

  explicit JsonProfilePersistence(std::filesystem::path storage_directory);

  auto load(const domain::ProfileIdentity& identity)
      -> std::expected<domain::ProfileMetadata, domain::ProfilePersistenceError> override;
  auto save(const domain::ProfileMetadata& metadata)
      -> std::expected<void, domain::ProfilePersistenceError> override;

private:
  std::filesystem::path m_storage_directory;
};

} // namespace fmm::infrastructure
