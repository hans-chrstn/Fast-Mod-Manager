#pragma once

#include "core/IGamePluginLoader.hpp"
#include "core/IScriptEngine.hpp"

#include <memory>

namespace fmm::infrastructure {

class LuaGamePluginLoader : public core::IGamePluginLoader {
public:
  explicit LuaGamePluginLoader(std::shared_ptr<core::IScriptEngine> script_engine);

  [[nodiscard]] auto loadPlugin(const std::string& script_path) const
      -> std::expected<domain::GameDefinition, core::GamePluginError> override;

private:
  std::shared_ptr<core::IScriptEngine> m_script_engine;
};

} // namespace fmm::infrastructure
