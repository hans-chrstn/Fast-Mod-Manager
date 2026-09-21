#pragma once

#include "domain/GameId.hpp"

#include <compare>
#include <string>
#include <utility>

namespace fmm::domain {

class GameIdentity {
public:
  GameIdentity(GameId game_id, std::string name)
      : m_id(std::move(game_id)), m_name(std::move(name)) {}

  [[nodiscard]] auto id() const noexcept -> const GameId& { return m_id; }
  [[nodiscard]] auto name() const noexcept -> const std::string& { return m_name; }

  auto operator<=>(const GameIdentity&) const = default;

private:
  GameId m_id;
  std::string m_name;
};

} // namespace fmm::domain
