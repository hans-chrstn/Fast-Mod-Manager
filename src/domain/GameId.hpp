#pragma once

#include <compare>
#include <string>
#include <utility>

namespace fmm::domain {

class GameId {
public:
  explicit GameId(std::string identifier) : m_identifier(std::move(identifier)) {}

  [[nodiscard]] auto value() const noexcept -> const std::string& { return m_identifier; }

  auto operator==(const GameId&) const -> bool = default;
  auto operator<=>(const GameId&) const = default;

private:
  std::string m_identifier;
};

} // namespace fmm::domain
