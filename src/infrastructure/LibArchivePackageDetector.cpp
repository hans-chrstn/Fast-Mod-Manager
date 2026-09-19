#include "infrastructure/LibArchivePackageDetector.hpp"

#include <archive.h>

namespace fmm::infrastructure {

auto LibArchivePackageDetector::detectFormat(const std::filesystem::path& file_path,
                                             std::error_code& error_code) const
    -> domain::PackageIdentity {
  struct archive* arc = archive_read_new();
  archive_read_support_filter_all(arc);
  archive_read_support_format_all(arc);

  domain::PackageFormat format = domain::PackageFormat::Unknown;

  constexpr int block_size = 10240;
  int result = archive_read_open_filename(arc, file_path.c_str(), block_size);
  if (result == ARCHIVE_OK) {
    struct archive_entry* entry = nullptr;
    if (archive_read_next_header(arc, &entry) == ARCHIVE_OK) {
      int format_code = archive_format(arc) & ARCHIVE_FORMAT_BASE_MASK;
      if (format_code == ARCHIVE_FORMAT_ZIP) {
        format = domain::PackageFormat::Zip;
      } else if (format_code == ARCHIVE_FORMAT_7ZIP) {
        format = domain::PackageFormat::SevenZip;
      } else if (format_code == ARCHIVE_FORMAT_RAR || format_code == ARCHIVE_FORMAT_RAR_V5) {
        format = domain::PackageFormat::Rar;
      }
    }
  } else {
    error_code = std::make_error_code(std::errc::io_error);
  }

  archive_read_close(arc);
  archive_read_free(arc);

  return {file_path.stem().string(), format};
}

} // namespace fmm::infrastructure
