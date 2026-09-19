#include "infrastructure/GameManagerImpl.hpp"

#include <algorithm>
#include <filesystem>

namespace fmm::infrastructure {

GameManagerImpl::GameManagerImpl(std::shared_ptr<core::IScriptEngine> script_engine,
                                 std::string plugins_directory)
    : m_script_engine(std::move(script_engine)), m_plugins_directory(std::move(plugins_directory)) {

  std::error_code error_code;
  if (std::filesystem::exists(m_plugins_directory, error_code) &&
      std::filesystem::is_directory(m_plugins_directory, error_code)) {
    for (const auto& entry : std::filesystem::directory_iterator(m_plugins_directory, error_code)) {
      if (entry.is_directory(error_code)) {
        auto init_path = entry.path() / "init.lua";
        if (std::filesystem::exists(init_path, error_code) &&
            std::filesystem::is_regular_file(init_path, error_code)) {
          m_adapters.push_back(
              std::make_shared<LuaGameAdapter>(m_script_engine, init_path.string()));
        }
      }
    }
  }
}

auto GameManagerImpl::getAvailableGames() const -> std::vector<domain::GameIdentity> {
  std::vector<domain::GameIdentity> games;
  games.reserve(m_adapters.size());
  std::ranges::transform(m_adapters, std::back_inserter(games),
                         [](const auto& adapter) -> auto { return adapter->getIdentity(); });
  return games;
}

auto GameManagerImpl::getActiveAdapter() const
    -> std::expected<std::shared_ptr<core::IGameAdapter>, core::GameManagerError> {
  if (m_active_adapter) {
    return m_active_adapter;
  }
  return std::unexpected(core::GameManagerError::GameNotFound);
}

void GameManagerImpl::setActiveGame(domain::GameIdentity identity) {
  auto adapter_it = std::ranges::find_if(m_adapters, [&identity](const auto& adapter) -> bool {
    return adapter->getIdentity() == identity;
  });

  if (adapter_it != m_adapters.end()) {
    m_active_adapter = *adapter_it;
  } else {
    m_active_adapter = nullptr;
  }
}

} // namespace fmm::infrastructure
