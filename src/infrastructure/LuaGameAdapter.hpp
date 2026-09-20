#pragma once

#include "core/IGameAdapter.hpp"
#include "core/IScriptEngine.hpp"

#include <memory>
#include <string>

namespace fmm::infrastructure {

class LuaGameAdapter : public core::IGameAdapter {
public:
  LuaGameAdapter(std::shared_ptr<core::IScriptEngine> script_engine, std::string lua_script_path);

  [[nodiscard]] auto getIdentity() const -> domain::GameIdentity override;
  [[nodiscard]] auto getCapabilities() const -> domain::GameCapabilities override;
  [[nodiscard]] auto getGameDefinition() const -> domain::GameDefinition override;

private:
  std::shared_ptr<core::IScriptEngine> m_script_engine;
  std::string m_lua_script_path;
};

} // namespace fmm::infrastructure
