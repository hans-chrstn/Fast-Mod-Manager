#pragma once

#include "core/IDependencyValidator.hpp"

namespace fmm::infrastructure {

class DependencyValidatorImpl : public core::IDependencyValidator {
public:
  [[nodiscard]] auto getMissingDependencies() const -> std::vector<core::DependencyIssue> override;
};

} // namespace fmm::infrastructure
