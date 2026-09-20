#pragma once

#include "domain/ProfileIdentity.hpp"
#include "domain/ProfileMetadata.hpp"
#include "domain/ProfileState.hpp"

#include <cstdint>
#include <expected>

namespace fmm::application::ports {

enum class ProfilePersistenceError : std::uint8_t {
  FileNotFound,
  AccessDenied,
  InvalidFormat,
  UnsupportedSchemaVersion,
  WriteFailed
};

class IProfileRepository {
public:
  virtual ~IProfileRepository() = default;

  virtual auto loadMetadata(const domain::ProfileIdentity& identity)
      -> std::expected<domain::ProfileMetadata, ProfilePersistenceError> = 0;

  virtual auto loadState(const domain::ProfileIdentity& identity)
      -> std::expected<domain::ProfileState, ProfilePersistenceError> = 0;

  virtual auto saveMetadata(const domain::ProfileMetadata& metadata)
      -> std::expected<void, ProfilePersistenceError> = 0;

  virtual auto saveState(const domain::ProfileIdentity& identity, const domain::ProfileState& state)
      -> std::expected<void, ProfilePersistenceError> = 0;
};

} // namespace fmm::application::ports
