#include "infrastructure/LibArchivePackageDetector.hpp"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <system_error>

TEST_CASE("LibArchivePackageDetector handles nonexistent files", "[infrastructure][archive]") {
  fmm::infrastructure::LibArchivePackageDetector detector;
  std::error_code error;

  auto identity = detector.detectFormat("does_not_exist.zip", error);

  REQUIRE(error == std::errc::io_error);
  REQUIRE(identity.format() == fmm::domain::PackageFormat::Unknown);
}

TEST_CASE("LibArchivePackageDetector handles empty files as unknown", "[infrastructure][archive]") {
  fmm::infrastructure::LibArchivePackageDetector detector;
  std::error_code error;

  std::filesystem::path temp_file = std::filesystem::temp_directory_path() / "empty_test.zip";
  std::ofstream(temp_file).put('\0');

  auto identity = detector.detectFormat(temp_file, error);

  REQUIRE(identity.format() == fmm::domain::PackageFormat::Unknown);

  std::filesystem::remove(temp_file);
}
