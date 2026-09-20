#pragma once

#include "domain/GameIdentity.hpp"

#include <cstdint>
#include <expected>
#include <optional>

namespace fmm::core {

enum class ActiveGameError : std::uint8_t { NoActiveGame };

class IActiveGameSelection {
public:
  virtual ~IActiveGameSelection() = default;

  virtual void setActiveGame(std::optional<domain::GameIdentity> identity) = 0;
  [[nodiscard]] virtual auto getActiveGame() const
      -> std::expected<domain::GameIdentity, ActiveGameError> = 0;
};

} // namespace fmm::core
