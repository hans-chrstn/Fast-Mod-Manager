#pragma once

#include "domain/GameId.hpp"
#include "domain/PackageId.hpp"
#include "domain/PluginId.hpp"

#include <optional>
#include <vector>

namespace fmm::domain {

struct ProfileState {
  std::optional<GameId> selected_game_id;
  std::vector<PackageId> enabled_packages;
  std::vector<PluginId> load_order;
};

} // namespace fmm::domain
