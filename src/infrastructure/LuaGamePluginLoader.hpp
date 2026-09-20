#pragma once

#include "core/IGamePluginLoader.hpp"
#include "core/IScriptEngine.hpp"

namespace fmm::infrastructure {

class LuaGamePluginLoader : public core::IGamePluginLoader {
public:
  explicit LuaGamePluginLoader(std::shared_ptr<core::IScriptEngine> script_engine);

  [[nodiscard]] auto loadPlugin(const std::string& script_path) const
      -> std::shared_ptr<core::IGameAdapter> override;

private:
  std::shared_ptr<core::IScriptEngine> m_script_engine;
};

} // namespace fmm::infrastructure
