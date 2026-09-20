#pragma once

#include <string>
#include <vector>

namespace fmm::core {

struct GameFsPlan {
  std::string profile_id;
  std::string target_game_directory;
  std::vector<std::string> deployed_files;
};

} // namespace fmm::core
