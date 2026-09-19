#include "infrastructure/StubProcessLauncher.hpp"

namespace infrastructure {

auto StubProcessLauncher::launch(const std::string& executable_path,
                                 const std::vector<std::string>& /*arguments*/,
                                 const std::string& /*working_directory*/) const
    -> std::expected<void, core::ProcessLauncherError> {
  if (executable_path.empty()) {
    return std::unexpected(core::ProcessLauncherError::ExecutableNotFound);
  }

  if (executable_path == "/bin/false") {
    return std::unexpected(core::ProcessLauncherError::ExecutionFailed);
  }

  return {}; // Success
}

} // namespace infrastructure
