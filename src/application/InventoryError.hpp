#pragma once

#include <cstdint>

namespace fmm::application {

enum class InventoryError : std::uint8_t {
  SourceUnavailable,
  PermissionDenied,
  Cancelled,
  CorruptMetadata,
  Internal
};

} // namespace fmm::application
