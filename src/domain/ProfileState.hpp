#pragma once

#include "domain/PackageId.hpp"

#include <string>
#include <vector>

namespace fmm::domain {

struct ProfileState {
  std::string selected_game_id;
  std::vector<PackageId> enabled_packages;
  std::vector<std::string> load_order;
};

} // namespace fmm::domain
