#pragma once

#include "domain/GameCapabilities.hpp"
#include "domain/GameDefinition.hpp"
#include "domain/GameIdentity.hpp"

#include <vector>

namespace fmm::core {

class IGameAdapter {
public:
  virtual ~IGameAdapter() = default;

  [[nodiscard]] virtual auto getIdentity() const -> domain::GameIdentity = 0;
  [[nodiscard]] virtual auto getCapabilities() const -> domain::GameCapabilities = 0;
  [[nodiscard]] virtual auto getGameDefinition() const -> domain::GameDefinition = 0;
};

} // namespace fmm::core
