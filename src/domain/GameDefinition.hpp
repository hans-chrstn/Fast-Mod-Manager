#pragma once

#include <string>
#include <vector>

namespace fmm::domain {

struct GameDefinition {
  std::string game_id;
  std::string name;
  std::string executable_name;
  std::string mod_directory_name;
  std::vector<std::string> required_tools;
};

} // namespace fmm::domain
