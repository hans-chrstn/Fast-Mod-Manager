#pragma once

#include <compare>
#include <string>

namespace fmm::domain {

class GameIdentity {
public:
  GameIdentity(std::string game_id, std::string name)
      : m_id(std::move(game_id)), m_name(std::move(name)) {}

  [[nodiscard]] auto id() const -> const std::string& { return m_id; }
  [[nodiscard]] auto name() const -> const std::string& { return m_name; }

  auto operator<=>(const GameIdentity&) const = default;

private:
  std::string m_id;
  std::string m_name;
};

} // namespace fmm::domain
