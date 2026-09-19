#include "infrastructure/StubScriptEngine.hpp"

namespace infrastructure {

auto StubScriptEngine::evaluate(const std::string& script) const
    -> std::expected<std::string, std::string> {
  if (script.empty()) {
    return std::unexpected("Empty script provided.");
  }

  return "Executed: " + script;
}

} // namespace infrastructure
