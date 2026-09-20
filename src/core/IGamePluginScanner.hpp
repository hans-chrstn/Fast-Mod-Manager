#pragma once

#include <string>
#include <vector>

namespace fmm::core {

class IGamePluginScanner {
public:
  virtual ~IGamePluginScanner() = default;

  [[nodiscard]] virtual auto discoverPlugins(const std::string& directory) const
      -> std::vector<std::string> = 0;
};

} // namespace fmm::core
