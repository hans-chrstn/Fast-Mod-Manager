#pragma once

#include "core/IScriptEngine.hpp"

namespace fmm::infrastructure {

class StubScriptEngine : public core::IScriptEngine {
public:
  [[nodiscard]] auto evaluate(const std::string& script) const
      -> std::expected<std::string, std::string> override;
};

} // namespace fmm::infrastructure
