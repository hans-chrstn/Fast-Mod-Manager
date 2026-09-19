#include "infrastructure/LibArchiveReader.hpp"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <system_error>

TEST_CASE("LibArchiveReader handles nonexistent files", "[infrastructure][archive]") {
  fmm::infrastructure::LibArchiveReader reader;

  auto result = reader.readTree("does_not_exist.zip");

  REQUIRE(!result.has_value());
  REQUIRE(result.error() == fmm::core::ArchiveError::IoError);
}

TEST_CASE("LibArchiveReader handles empty files as corrupt", "[infrastructure][archive]") {
  fmm::infrastructure::LibArchiveReader reader;

  std::filesystem::path temp_file =
      std::filesystem::temp_directory_path() / "empty_reader_test.zip";
  std::ofstream(temp_file).put('\0');

  auto result = reader.readTree(temp_file);

  REQUIRE(!result.has_value());
  REQUIRE(result.error() == fmm::core::ArchiveError::IoError);

  std::filesystem::remove(temp_file);
}
