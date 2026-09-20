#pragma once

#include "core/GamePluginError.hpp"
#include "domain/GameDefinition.hpp"

#include <expected>
#include <string>

namespace fmm::core {

class IScriptEngine {
public:
  virtual ~IScriptEngine() = default;

  [[nodiscard]] virtual auto evaluateGamePlugin(const std::string& script_content) const
      -> std::expected<domain::GameDefinition, GamePluginError> = 0;
};

} // namespace fmm::core
