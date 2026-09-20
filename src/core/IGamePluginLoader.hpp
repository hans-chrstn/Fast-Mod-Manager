#pragma once

#include "core/IGameAdapter.hpp"

#include <memory>
#include <string>

namespace fmm::core {

class IGamePluginLoader {
public:
  virtual ~IGamePluginLoader() = default;

  [[nodiscard]] virtual auto loadPlugin(const std::string& script_path) const
      -> std::shared_ptr<IGameAdapter> = 0;
};

} // namespace fmm::core
