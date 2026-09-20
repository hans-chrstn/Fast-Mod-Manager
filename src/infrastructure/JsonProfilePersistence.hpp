#pragma once

#include "application/ports/IProfileRepository.hpp"
#include "domain/ProfileMetadata.hpp"
#include "domain/ProfileState.hpp"

#include <filesystem>

namespace fmm::infrastructure {

class JsonProfilePersistence final : public application::ports::IProfileRepository {
public:
  static constexpr int CURRENT_SCHEMA_VERSION = 1;

  explicit JsonProfilePersistence(std::filesystem::path storage_directory);

  auto loadMetadata(const domain::ProfileIdentity& identity)
      -> std::expected<domain::ProfileMetadata,
                       application::ports::ProfilePersistenceError> override;

  auto loadState(const domain::ProfileIdentity& identity)
      -> std::expected<domain::ProfileState, application::ports::ProfilePersistenceError> override;

  auto saveMetadata(const domain::ProfileMetadata& metadata)
      -> std::expected<void, application::ports::ProfilePersistenceError> override;

  auto saveState(const domain::ProfileIdentity& identity, const domain::ProfileState& state)
      -> std::expected<void, application::ports::ProfilePersistenceError> override;

private:
  std::filesystem::path m_storage_directory;
};

} // namespace fmm::infrastructure
