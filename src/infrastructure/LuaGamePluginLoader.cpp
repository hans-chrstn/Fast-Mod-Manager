#include "infrastructure/LuaGamePluginLoader.hpp"

#include "infrastructure/LuaGameAdapter.hpp"

namespace fmm::infrastructure {

LuaGamePluginLoader::LuaGamePluginLoader(std::shared_ptr<core::IScriptEngine> script_engine)
    : m_script_engine(std::move(script_engine)) {}

auto LuaGamePluginLoader::loadPlugin(const std::string& script_path) const
    -> std::shared_ptr<core::IGameAdapter> {
  return std::make_shared<LuaGameAdapter>(m_script_engine, script_path);
}

} // namespace fmm::infrastructure
