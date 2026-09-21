#pragma once

#include "core/IGameCatalog.hpp"

#include <vector>

namespace fmm::application {

class GameCatalogImpl : public core::IGameCatalog {
public:
  void registerGame(domain::GameDefinition definition) override;
  void clear() override;

  [[nodiscard]] auto getAvailableGames() const -> std::vector<domain::GameIdentity> override;
  [[nodiscard]] auto getGameDefinition(const domain::GameId& game_id) const
      -> std::optional<domain::GameDefinition> override;

private:
  std::vector<domain::GameDefinition> m_definitions;
};

} // namespace fmm::application
