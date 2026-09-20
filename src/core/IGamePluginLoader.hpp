#pragma once

#include "core/GamePluginError.hpp"
#include "domain/GameDefinition.hpp"

#include <expected>
#include <string>

namespace fmm::core {

class IGamePluginLoader {
public:
  virtual ~IGamePluginLoader() = default;

  [[nodiscard]] virtual auto loadPlugin(const std::string& script_path) const
      -> std::expected<domain::GameDefinition, GamePluginError> = 0;
};

} // namespace fmm::core
