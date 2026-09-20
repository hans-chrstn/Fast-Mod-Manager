#pragma once

#include "domain/GameCapabilities.hpp"
#include "domain/GameIdentity.hpp"

#include <string>
#include <vector>

namespace fmm::domain {

struct GameDefinition {
  GameIdentity identity;
  GameCapabilities capabilities;
  std::string executable_name;
  std::string mod_directory_name;
  std::vector<std::string> required_tools;
};

} // namespace fmm::domain
