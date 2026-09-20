#include "application/GameCatalogImpl.hpp"

#include <algorithm>
#include <iterator>

namespace fmm::application {

void GameCatalogImpl::registerGame(domain::GameDefinition definition) {
  m_definitions.push_back(std::move(definition));
}

void GameCatalogImpl::clear() { m_definitions.clear(); }

auto GameCatalogImpl::getAvailableGames() const -> std::vector<domain::GameIdentity> {
  std::vector<domain::GameIdentity> games;
  games.reserve(m_definitions.size());
  std::ranges::transform(m_definitions, std::back_inserter(games),
                         [](const auto& def) -> auto { return def.identity; });
  return games;
}

auto GameCatalogImpl::getGameDefinition(const domain::GameIdentity& identity) const
    -> std::optional<domain::GameDefinition> {
  auto iterator = std::ranges::find_if(
      m_definitions, [&identity](const auto& def) -> bool { return def.identity == identity; });
  if (iterator != m_definitions.end()) {
    return *iterator;
  }
  return std::nullopt;
}

} // namespace fmm::application
