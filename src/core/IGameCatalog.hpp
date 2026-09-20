#pragma once

#include "core/IGameAdapter.hpp"
#include "domain/GameIdentity.hpp"

#include <memory>
#include <vector>

namespace fmm::core {

class IGameCatalog {
public:
  virtual ~IGameCatalog() = default;

  virtual void registerAdapter(std::shared_ptr<IGameAdapter> adapter) = 0;
  virtual void clear() = 0;

  [[nodiscard]] virtual auto getAvailableGames() const -> std::vector<domain::GameIdentity> = 0;
  [[nodiscard]] virtual auto getAdapter(const domain::GameIdentity& identity) const
      -> std::shared_ptr<IGameAdapter> = 0;
};

} // namespace fmm::core
