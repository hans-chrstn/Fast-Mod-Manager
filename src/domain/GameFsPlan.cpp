#include "domain/GameFsPlan.hpp"

#include <utility>

namespace fmm::domain {

GameFsPlan::GameFsPlan(ProfileIdentity profile_identity,
                       NamespaceCompilationResult namespace_result)
    : m_profile_identity(std::move(profile_identity)),
      m_namespace_result(std::move(namespace_result)) {}

auto GameFsPlan::profileIdentity() const noexcept -> const ProfileIdentity& {
  return m_profile_identity;
}

auto GameFsPlan::namespaceResult() const noexcept -> const NamespaceCompilationResult& {
  return m_namespace_result;
}

} // namespace fmm::domain
