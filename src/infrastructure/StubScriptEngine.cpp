#include "infrastructure/StubScriptEngine.hpp"

namespace fmm::infrastructure {

auto StubScriptEngine::evaluateGamePlugin(const std::string& script_content) const
    -> std::expected<domain::GameDefinition, core::GamePluginError> {
  if (script_content.empty()) {
    return std::unexpected(core::GamePluginError{.code = core::GamePluginErrorCode::InvalidFormat,
                                                 .message = "Script content cannot be empty"});
  }

  domain::GameCapabilities caps;
  caps.supports_load_order = true;
  caps.supports_plugins = true;
  caps.supports_script_extender = true;

  domain::GameDefinition def{
      .identity = domain::GameIdentity("stub_game", "Stub Game"),
      .capabilities = caps,
      .executable_name = "stub_game.exe",
      .mod_directory_name = "Mods",
      .required_tools = {},
  };
  return def;
}

} // namespace fmm::infrastructure
