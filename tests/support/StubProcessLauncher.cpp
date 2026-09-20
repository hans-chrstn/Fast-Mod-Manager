#include "support/StubProcessLauncher.hpp"

namespace fmm::test_support {

auto StubProcessLauncher::launch(const std::string& executable_path,
                                 [[maybe_unused]] const std::vector<std::string>& arguments,
                                 [[maybe_unused]] const std::string& working_directory) const
    -> std::expected<void, core::ProcessLauncherError> {
  if (executable_path.empty()) {
    return std::unexpected(core::ProcessLauncherError::ExecutableNotFound);
  }

  if (executable_path == "/bin/false") {
    return std::unexpected(core::ProcessLauncherError::ExecutionFailed);
  }

  return {};
}

} // namespace fmm::test_support
