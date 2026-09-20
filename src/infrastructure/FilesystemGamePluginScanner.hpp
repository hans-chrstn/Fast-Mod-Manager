#pragma once

#include "core/IGamePluginScanner.hpp"

namespace fmm::infrastructure {

class FilesystemGamePluginScanner : public core::IGamePluginScanner {
public:
  [[nodiscard]] auto discoverPlugins(const std::string& directory) const
      -> std::vector<std::string> override;
};

} // namespace fmm::infrastructure
