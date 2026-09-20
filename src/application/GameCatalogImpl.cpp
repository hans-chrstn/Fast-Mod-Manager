#include "application/GameCatalogImpl.hpp"

#include <algorithm>
#include <iterator>

namespace fmm::application {

void GameCatalogImpl::registerAdapter(std::shared_ptr<core::IGameAdapter> adapter) {
  if (adapter) {
    m_adapters.push_back(std::move(adapter));
  }
}

void GameCatalogImpl::clear() { m_adapters.clear(); }

auto GameCatalogImpl::getAvailableGames() const -> std::vector<domain::GameIdentity> {
  std::vector<domain::GameIdentity> games;
  games.reserve(m_adapters.size());
  std::ranges::transform(m_adapters, std::back_inserter(games),
                         [](const auto& adapter) -> auto { return adapter->getIdentity(); });
  return games;
}

auto GameCatalogImpl::getAdapter(const domain::GameIdentity& identity) const
    -> std::shared_ptr<core::IGameAdapter> {
  auto iterator = std::ranges::find_if(m_adapters, [&identity](const auto& adapter) -> bool {
    return adapter->getIdentity() == identity;
  });
  if (iterator != m_adapters.end()) {
    return *iterator;
  }
  return nullptr;
}

} // namespace fmm::application
