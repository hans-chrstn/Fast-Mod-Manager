#pragma once

#include "core/IActiveGameSelection.hpp"

namespace fmm::application {

class ActiveGameSelectionImpl : public core::IActiveGameSelection {
public:
  void setActiveGame(std::optional<domain::GameIdentity> identity) override;
  [[nodiscard]] auto getActiveGame() const
      -> std::expected<domain::GameIdentity, core::ActiveGameError> override;

private:
  std::optional<domain::GameIdentity> m_active_game;
};

} // namespace fmm::application
