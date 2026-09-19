find_package(Catch2 3 CONFIG REQUIRED)

add_executable(mod-manager-tests
  smoke_test.cpp
  unit/profile_identity_test.cpp
  unit/json_profile_persistence_test.cpp
  unit/inventory_model_test.cpp
  unit/service_registry_test.cpp
  unit/script_engine_test.cpp
  unit/process_launcher_test.cpp
  unit/fixture_filesystem_scanner_test.cpp
  unit/async_task_test.cpp
  unit/game_manager_test.cpp
  unit/libarchive_package_detector_test.cpp
  unit/libarchive_reader_test.cpp
  unit/path_normalizer_test.cpp
)

target_link_libraries(mod-manager-tests
    domain core application infrastructure ui
    Catch2::Catch2WithMain
    Qt6::Core Qt6::Gui Qt6::Widgets
)

add_executable(async-test-app
    async_test_app.cpp
)

target_link_libraries(async-test-app
    domain core application infrastructure ui
    Qt6::Core Qt6::Gui Qt6::Widgets
)

add_executable(detector-test-app
    detector_test_app.cpp
)

target_link_libraries(detector-test-app
    domain core infrastructure
)

add_executable(reader-test-app
    reader_test_app.cpp
)

target_link_libraries(reader-test-app
    domain core infrastructure
)

include(Catch)
catch_discover_tests(mod-manager-tests)
