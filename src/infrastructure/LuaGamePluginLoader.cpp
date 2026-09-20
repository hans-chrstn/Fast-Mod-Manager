#include "infrastructure/LuaGamePluginLoader.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

namespace fmm::infrastructure {

LuaGamePluginLoader::LuaGamePluginLoader(std::shared_ptr<core::IScriptEngine> script_engine)
    : m_script_engine(std::move(script_engine)) {}

auto LuaGamePluginLoader::loadPlugin(const std::string& script_path) const
    -> std::expected<domain::GameDefinition, core::GamePluginError> {
  std::ifstream file(script_path);
  if (!file.is_open()) {
    return std::unexpected(
        core::GamePluginError{.code = core::GamePluginErrorCode::FileNotFound,
                              .message = "Failed to open game plugin script: " + script_path});
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string script_content = buffer.str();

  return m_script_engine->evaluateGamePlugin(script_content);
}

} // namespace fmm::infrastructure
