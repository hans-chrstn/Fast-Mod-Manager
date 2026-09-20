#pragma once

#include <map>
#include <string>
#include <vector>

namespace fmm::core {

struct LaunchPlan {
  std::string executable_path;
  std::vector<std::string> arguments;
  std::string working_directory;
  std::map<std::string, std::string> environment;
};

} // namespace fmm::core
