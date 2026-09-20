#pragma once

#include "domain/GameDefinition.hpp"
#include "domain/InstalledPackage.hpp"
#include "domain/ProfileState.hpp"

#include <utility>
#include <vector>

namespace fmm::domain {

class PlannerInput final {
public:
  PlannerInput(ProfileState profile_state, GameDefinition game_definition,
               std::vector<InstalledPackage> installed_packages)
      : m_profile_state(std::move(profile_state)), m_game_definition(std::move(game_definition)),
        m_installed_packages(std::move(installed_packages)) {}

  [[nodiscard]] auto profileState() const noexcept -> const ProfileState& {
    return m_profile_state;
  }

  [[nodiscard]] auto gameDefinition() const noexcept -> const GameDefinition& {
    return m_game_definition;
  }

  [[nodiscard]] auto installedPackages() const noexcept -> const std::vector<InstalledPackage>& {
    return m_installed_packages;
  }

private:
  ProfileState m_profile_state;
  GameDefinition m_game_definition;
  std::vector<InstalledPackage> m_installed_packages;
};

} // namespace fmm::domain
