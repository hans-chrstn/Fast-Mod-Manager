#include "infrastructure/LuaGameAdapter.hpp"

#include <filesystem>

namespace fmm::infrastructure {

LuaGameAdapter::LuaGameAdapter(std::shared_ptr<core::IScriptEngine> script_engine,
                               std::string lua_script_path)
    : m_script_engine(std::move(script_engine)), m_lua_script_path(std::move(lua_script_path)) {}

auto LuaGameAdapter::getIdentity() const -> domain::GameIdentity {
  std::string game_id = "unknown";
  std::filesystem::path script_path(m_lua_script_path);
  if (script_path.has_parent_path()) {
    game_id = script_path.parent_path().filename().string();
  }
  return {game_id, game_id};
}

auto LuaGameAdapter::getCapabilities() const -> domain::GameCapabilities {
  domain::GameCapabilities caps;
  caps.supports_load_order = true;
  caps.supports_plugins = true;
  caps.supports_script_extender = true;
  return caps;
}

auto LuaGameAdapter::getGameDefinition() const -> domain::GameDefinition {
  domain::GameDefinition def;
  def.game_id = getIdentity().id();
  def.name = "Unknown Game";
  def.executable_name = def.game_id + ".exe";
  def.mod_directory_name = "Mods";
  return def;
}

} // namespace fmm::infrastructure
