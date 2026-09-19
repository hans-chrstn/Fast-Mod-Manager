#pragma once

#include "core/IGameManager.hpp"
#include "core/IScriptEngine.hpp"
#include "infrastructure/LuaGameAdapter.hpp"

#include <memory>
#include <string>
#include <vector>

namespace fmm::infrastructure {

class GameManagerImpl : public core::IGameManager {
public:
  explicit GameManagerImpl(std::shared_ptr<core::IScriptEngine> script_engine,
                           std::string plugins_directory);

  [[nodiscard]] auto getAvailableGames() const -> std::vector<domain::GameIdentity> override;
  [[nodiscard]] auto getActiveAdapter() const
      -> std::expected<std::shared_ptr<core::IGameAdapter>, core::GameManagerError> override;

  void setActiveGame(domain::GameIdentity identity);

private:
  std::shared_ptr<core::IScriptEngine> m_script_engine;
  std::string m_plugins_directory;
  std::vector<std::shared_ptr<LuaGameAdapter>> m_adapters;
  std::shared_ptr<core::IGameAdapter> m_active_adapter;
};

} // namespace fmm::infrastructure
