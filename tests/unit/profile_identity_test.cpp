#include "domain/ProfileIdentity.hpp"

#include <catch2/catch_test_macros.hpp>

using namespace fmm::domain;

TEST_CASE("ProfileIdentity validation", "[domain][profile]") {
  SECTION("accepts valid names") {
    auto profile = fmm::domain::ProfileIdentity::create("Skyrim Modded");
    REQUIRE(profile.has_value());
    REQUIRE(profile->name() == "Skyrim Modded");
    REQUIRE_FALSE(profile->parent_id().has_value());
  }

  SECTION("accepts valid sub-profile names with parent id") {
    auto profile = fmm::domain::ProfileIdentity::create("Mage Build", "Skyrim Modded");
    REQUIRE(profile.has_value());
    REQUIRE(profile->name() == "Mage Build");
    REQUIRE(profile->parent_id().value_or("") == "Skyrim Modded");
  }

  SECTION("Empty name is rejected") {
    auto result = ProfileIdentity::create("");
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == ProfileIdentityError::EmptyName);
  }

  SECTION("Leading or trailing whitespace is rejected") {
    REQUIRE(ProfileIdentity::create(" Profile").error() ==
            ProfileIdentityError::TrailingOrLeadingWhitespace);
    REQUIRE(ProfileIdentity::create("Profile ").error() ==
            ProfileIdentityError::TrailingOrLeadingWhitespace);
  }

  SECTION("Invalid characters are rejected") {
    REQUIRE(ProfileIdentity::create("Profile/1").error() ==
            ProfileIdentityError::InvalidCharacters);
    REQUIRE(ProfileIdentity::create("Profile\\1").error() ==
            ProfileIdentityError::InvalidCharacters);
    REQUIRE(ProfileIdentity::create("Profile:1").error() ==
            ProfileIdentityError::InvalidCharacters);
    REQUIRE(ProfileIdentity::create("Profile*1").error() ==
            ProfileIdentityError::InvalidCharacters);
    REQUIRE(ProfileIdentity::create("Profile?1").error() ==
            ProfileIdentityError::InvalidCharacters);
    REQUIRE(ProfileIdentity::create("Profile\"1").error() ==
            ProfileIdentityError::InvalidCharacters);
    REQUIRE(ProfileIdentity::create("Profile<1").error() ==
            ProfileIdentityError::InvalidCharacters);
    REQUIRE(ProfileIdentity::create("Profile>1").error() ==
            ProfileIdentityError::InvalidCharacters);
    REQUIRE(ProfileIdentity::create("Profile|1").error() ==
            ProfileIdentityError::InvalidCharacters);
    REQUIRE(ProfileIdentity::create(std::string("Profile\0", 8)).error() ==
            ProfileIdentityError::InvalidCharacters);
  }

  SECTION("Reserved names are rejected") {
    REQUIRE(ProfileIdentity::create(".").error() == ProfileIdentityError::ReservedName);
    REQUIRE(ProfileIdentity::create("..").error() == ProfileIdentityError::ReservedName);
  }

  SECTION("Fails when name is too long") {
    constexpr int too_long_length = 256;
    std::string long_name(too_long_length, 'a');
    auto result = ProfileIdentity::create(long_name);
    REQUIRE(!result.has_value());
    REQUIRE(result.error() == ProfileIdentityError::NameTooLong);
  }

  SECTION("Equality compares inner names correctly") {
    auto profile_a1 = ProfileIdentity::create("Profile A").value();
    auto profile_a2 = ProfileIdentity::create("Profile A").value();
    auto profile_b = ProfileIdentity::create("Profile B").value();

    REQUIRE(profile_a1 == profile_a2);
    REQUIRE_FALSE(profile_a1 == profile_b);
  }
}
