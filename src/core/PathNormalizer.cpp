#include "core/PathNormalizer.hpp"

#include <algorithm>
#include <string_view>

namespace fmm::core {

auto PathNormalizer::normalizeArchiveEntry(const std::string& raw_path)
    -> std::expected<std::string, PathError> {
  if (raw_path.empty()) {
    return "";
  }

  std::string normalized = raw_path;
  std::ranges::replace(normalized, '\\', '/');

  if (normalized.front() == '/') {
    return std::unexpected(PathError::AbsoluteNotAllowed);
  }

  if (normalized.size() >= 2 && std::isalpha(normalized[0]) != 0 && normalized[1] == ':') {
    return std::unexpected(PathError::AbsoluteNotAllowed);
  }

  std::string_view view = normalized;
  if (view == ".." || view.starts_with("../") || view.ends_with("/..") ||
      view.find("/../") != std::string_view::npos) {
    return std::unexpected(PathError::TraversalNotAllowed);
  }

  std::string cleaned;
  cleaned.reserve(normalized.size());

  std::size_t start = 0;
  while (start < normalized.size()) {
    std::size_t end = normalized.find('/', start);
    std::string_view segment = std::string_view(normalized).substr(start, end - start);

    if (!segment.empty() && segment != ".") {
      if (!cleaned.empty()) {
        cleaned += '/';
      }
      cleaned += segment;
    }

    if (end == std::string::npos) {
      break;
    }
    start = end + 1;
  }

  return cleaned;
}

} // namespace fmm::core
