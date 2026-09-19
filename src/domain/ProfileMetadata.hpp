#pragma once

#include "domain/ProfileIdentity.hpp"

#include <cstdint>
#include <expected>
#include <string>

namespace fmm::domain {

struct ProfileMetadata {
  ProfileIdentity identity;
  int schema_version = 0;
  std::string description;
};

enum class ProfilePersistenceError : std::uint8_t {
  FileNotFound,
  AccessDenied,
  InvalidFormat,
  UnsupportedSchemaVersion,
  WriteFailed
};

class ProfilePersistencePort {
public:
  virtual ~ProfilePersistencePort() = default;

  virtual auto load(const ProfileIdentity& identity)
      -> std::expected<ProfileMetadata, ProfilePersistenceError> = 0;
  virtual auto save(const ProfileMetadata& metadata)
      -> std::expected<void, ProfilePersistenceError> = 0;
};

} // namespace fmm::domain
