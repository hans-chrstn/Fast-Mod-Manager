#include "application/ActiveGameSelectionImpl.hpp"

namespace fmm::application {

void ActiveGameSelectionImpl::setActiveGame(std::optional<domain::GameIdentity> identity) {
  m_active_game = std::move(identity);
}

auto ActiveGameSelectionImpl::getActiveGame() const
    -> std::expected<domain::GameIdentity, core::ActiveGameError> {
  if (m_active_game) {
    return *m_active_game;
  }
  return std::unexpected(core::ActiveGameError::NoActiveGame);
}

} // namespace fmm::application
