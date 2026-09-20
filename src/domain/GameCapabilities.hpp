#pragma once

#include <algorithm>
#include <compare>
#include <string>
#include <vector>

namespace fmm::domain {

struct GameCapabilities {
  std::string filesystem_semantics{"windows"};
  std::vector<std::string> required_features;
  std::vector<std::string> preferred_features;

  [[nodiscard]] auto hasRequiredFeature(const std::string& feature) const -> bool {
    return std::ranges::find(required_features, feature) != required_features.end();
  }

  [[nodiscard]] auto hasPreferredFeature(const std::string& feature) const -> bool {
    return std::ranges::find(preferred_features, feature) != preferred_features.end();
  }

  auto operator<=>(const GameCapabilities&) const = default;
};

} // namespace fmm::domain
