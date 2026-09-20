#pragma once

#include <cstdint>

namespace fmm::domain {

enum class ImportError : std::uint8_t {
  FileNotFound,
  AccessDenied,
  UnsupportedFormat,
  ExceedsSizeLimit,
  StagingFailed,
  InvalidChecksum
};

} // namespace fmm::domain
