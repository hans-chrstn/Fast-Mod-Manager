find_package(Catch2 3 CONFIG REQUIRED)

add_executable(mod-manager-tests
  smoke_test.cpp
)

target_link_libraries(mod-manager-tests PRIVATE Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(mod-manager-tests)
