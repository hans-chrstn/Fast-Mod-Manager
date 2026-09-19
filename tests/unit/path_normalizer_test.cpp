#include "core/PathNormalizer.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("PathNormalizer rejects absolute paths", "[core][path]") {
  REQUIRE(!fmm::core::PathNormalizer::normalizeArchiveEntry("/etc/passwd").has_value());
  REQUIRE(!fmm::core::PathNormalizer::normalizeArchiveEntry("C:/Windows/System32").has_value());
  REQUIRE(!fmm::core::PathNormalizer::normalizeArchiveEntry("c:\\Windows\\System32").has_value());
}

TEST_CASE("PathNormalizer rejects directory traversal", "[core][path]") {
  REQUIRE(!fmm::core::PathNormalizer::normalizeArchiveEntry("../foo").has_value());
  REQUIRE(!fmm::core::PathNormalizer::normalizeArchiveEntry("foo/../bar").has_value());
  REQUIRE(!fmm::core::PathNormalizer::normalizeArchiveEntry("foo/..").has_value());
  REQUIRE(!fmm::core::PathNormalizer::normalizeArchiveEntry("..").has_value());
  REQUIRE(!fmm::core::PathNormalizer::normalizeArchiveEntry("..\\foo").has_value());
}

TEST_CASE("PathNormalizer normalizes valid paths", "[core][path]") {
  auto result1 = fmm::core::PathNormalizer::normalizeArchiveEntry("foo\\bar\\baz");
  REQUIRE(result1.has_value());
  REQUIRE(result1.value() == "foo/bar/baz");

  auto result2 = fmm::core::PathNormalizer::normalizeArchiveEntry("foo/./bar//baz/");
  REQUIRE(result2.has_value());
  REQUIRE(result2.value() == "foo/bar/baz");

  auto result3 = fmm::core::PathNormalizer::normalizeArchiveEntry("");
  REQUIRE(result3.has_value());
  REQUIRE(result3.value().empty());
}
