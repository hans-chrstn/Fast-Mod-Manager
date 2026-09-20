#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace fmm::domain {

struct ProfileRuntimeState {
  bool is_mounted = false;
  std::optional<std::string> active_namespace_generation_id;
  std::optional<std::uint64_t> active_mount_handle;
};

} // namespace fmm::domain
