#pragma once

#include "domain/CompiledNamespace.hpp"
#include "domain/ProfileIdentity.hpp"

namespace fmm::domain {

class GameFsPlan final {
public:
  GameFsPlan(ProfileIdentity profile_identity, NamespaceCompilationResult namespace_result);

  [[nodiscard]] auto profileIdentity() const noexcept -> const ProfileIdentity&;

  [[nodiscard]] auto namespaceResult() const noexcept -> const NamespaceCompilationResult&;

  auto operator==(const GameFsPlan&) const -> bool = default;

private:
  ProfileIdentity m_profile_identity;
  NamespaceCompilationResult m_namespace_result;
};

} // namespace fmm::domain
