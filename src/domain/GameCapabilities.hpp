#pragma once

#include <compare>

namespace fmm::domain {

struct GameCapabilities {
  bool supports_load_order{false};
  bool supports_plugins{false};
  bool supports_script_extender{false};

  auto operator<=>(const GameCapabilities&) const = default;
};

} // namespace fmm::domain
