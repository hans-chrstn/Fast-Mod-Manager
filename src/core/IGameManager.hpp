#pragma once

#include "IGameAdapter.hpp"
#include "domain/GameIdentity.hpp"

#include <cstdint>
#include <expected>
#include <memory>
#include <vector>

namespace fmm::core {

enum class GameManagerError : std::uint8_t { GameNotFound, InvalidAdapter };

class IGameManager {
public:
  virtual ~IGameManager() = default;

  [[nodiscard]] virtual auto getAvailableGames() const -> std::vector<domain::GameIdentity> = 0;
  [[nodiscard]] virtual auto getActiveAdapter() const
      -> std::expected<std::shared_ptr<IGameAdapter>, GameManagerError> = 0;
};

} // namespace fmm::core
