#pragma once

#include "core/IArchiveReader.hpp"

namespace fmm::infrastructure {

class LibArchiveReader : public core::IArchiveReader {
public:
  [[nodiscard]] auto readTree(const std::filesystem::path& archive_path) const
      -> std::expected<domain::VirtualTree, core::ArchiveError> override;
};

} // namespace fmm::infrastructure
