#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace fmm::domain {

struct StagingPolicy {
  static constexpr std::uintmax_t DEFAULT_MAX_SIZE = 10ULL * 1024ULL * 1024ULL * 1024ULL;
  std::uintmax_t max_archive_size_bytes{DEFAULT_MAX_SIZE};
  bool require_checksum{true};
  std::vector<std::string> allowed_extensions{".zip", ".7z", ".rar", ".tar", ".gz"};
};

} // namespace fmm::domain
