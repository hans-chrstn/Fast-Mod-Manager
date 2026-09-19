#include "infrastructure/StubScriptEngine.hpp"

namespace fmm::infrastructure {

auto StubScriptEngine::evaluate(const std::string& script) const
    -> std::expected<std::string, std::string> {
  if (script.empty()) {
    return std::unexpected("Empty script");
  }
  return "Evaluated: " + script;
}

} // namespace fmm::infrastructure
