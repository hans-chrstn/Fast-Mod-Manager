#pragma once

#include "domain/ProfileIdentity.hpp"

#include <string>

namespace fmm::domain {

struct ProfileMetadata {
  ProfileIdentity identity;
  int schema_version = 0;
  std::string description;
};

} // namespace fmm::domain
