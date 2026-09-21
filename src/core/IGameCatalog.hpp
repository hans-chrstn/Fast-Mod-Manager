#pragma once

#include "domain/GameDefinition.hpp"
#include "domain/GameIdentity.hpp"

#include <optional>
#include <vector>

namespace fmm::core {

class IGameCatalog {
public:
  virtual ~IGameCatalog() = default;

  virtual void registerGame(domain::GameDefinition definition) = 0;
  virtual void clear() = 0;

  [[nodiscard]] virtual auto getAvailableGames() const -> std::vector<domain::GameIdentity> = 0;
  [[nodiscard]] virtual auto getGameDefinition(const domain::GameId& game_id) const
      -> std::optional<domain::GameDefinition> = 0;
};

} // namespace fmm::core
