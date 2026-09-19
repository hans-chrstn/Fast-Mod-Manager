#pragma once

#include <string>
#include <vector>

namespace fmm::core {

struct DependencyIssue {
  std::string name;
  std::string description;
  std::string resolution_hint;
};

class IDependencyValidator {
public:
  virtual ~IDependencyValidator() = default;

  [[nodiscard]] virtual auto getMissingDependencies() const -> std::vector<DependencyIssue> = 0;
};

} // namespace fmm::core
