#include "infrastructure/LibArchiveReader.hpp"

#include "core/PathNormalizer.hpp"

#include <algorithm>
#include <archive.h>
#include <archive_entry.h>
#include <sstream>
#include <vector>

namespace fmm::infrastructure {

namespace {
void insertIntoTree(domain::VirtualTree& tree, const std::string& path_str, bool is_dir,
                    std::uint64_t size) {
  std::stringstream path_stream(path_str);
  std::string segment;
  domain::VirtualNode* current = &tree.root();

  std::vector<std::string> segments;
  while (std::getline(path_stream, segment, '/')) {
    if (!segment.empty() && segment != "." && segment != "..") {
      segments.push_back(segment);
    }
  }

  for (std::size_t index = 0; index < segments.size(); ++index) {
    const auto& seg = segments[index];
    bool is_last = (index == segments.size() - 1);

    auto node_iter =
        std::ranges::find_if(current->children, [&seg](const domain::VirtualNode& node) -> bool {
          return node.name == seg;
        });

    if (node_iter != current->children.end()) {
      current = &(*node_iter);
      if (is_last && is_dir) {
        current->is_directory = true;
      }
    } else {
      bool node_is_dir = is_last ? is_dir : true;
      std::uint64_t node_size = (is_last && !is_dir) ? size : 0;
      current->children.push_back({seg, node_is_dir, node_size, {}});
      current = &current->children.back();
    }
  }
}
} // namespace

auto LibArchiveReader::readTree(const std::filesystem::path& archive_path) const
    -> std::expected<domain::VirtualTree, core::ArchiveError> {
  struct archive* arc = archive_read_new();
  archive_read_support_filter_all(arc);
  archive_read_support_format_all(arc);

  constexpr int block_size = 10240;
  int result = archive_read_open_filename(arc, archive_path.c_str(), block_size);
  if (result != ARCHIVE_OK) {
    archive_read_free(arc);
    return std::unexpected(core::ArchiveError::IoError);
  }

  domain::VirtualTree tree;
  struct archive_entry* entry = nullptr;

  while (true) {
    int header_result = archive_read_next_header(arc, &entry);
    if (header_result == ARCHIVE_EOF) {
      break;
    }
    if (header_result != ARCHIVE_OK && header_result != ARCHIVE_WARN) {
      archive_read_free(arc);
      return std::unexpected(core::ArchiveError::CorruptHeader);
    }

    const char* pathname = archive_entry_pathname(entry);
    if (pathname != nullptr) {
      auto normalized = core::PathNormalizer::normalizeArchiveEntry(pathname);
      if (!normalized.has_value()) {
        archive_read_free(arc);
        return std::unexpected(core::ArchiveError::InvalidFormat);
      }
      bool is_dir = (archive_entry_filetype(entry) == AE_IFDIR);
      auto size = static_cast<std::uint64_t>(archive_entry_size(entry));
      insertIntoTree(tree, normalized.value(), is_dir, size);
    }

    archive_read_data_skip(arc);
  }

  archive_read_close(arc);
  archive_read_free(arc);

  return tree;
}

} // namespace fmm::infrastructure
