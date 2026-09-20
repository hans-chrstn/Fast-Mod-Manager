#pragma once

#include "core/IGameCatalog.hpp"

#include <vector>

namespace fmm::application {

class GameCatalogImpl : public core::IGameCatalog {
public:
  void registerAdapter(std::shared_ptr<core::IGameAdapter> adapter) override;
  void clear() override;

  [[nodiscard]] auto getAvailableGames() const -> std::vector<domain::GameIdentity> override;
  [[nodiscard]] auto getAdapter(const domain::GameIdentity& identity) const
      -> std::shared_ptr<core::IGameAdapter> override;

private:
  std::vector<std::shared_ptr<core::IGameAdapter>> m_adapters;
};

} // namespace fmm::application
