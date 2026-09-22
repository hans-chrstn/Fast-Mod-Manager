find_package(Catch2 3 CONFIG REQUIRED)

add_library(mod-manager-test-support OBJECT
  support/FakeInventoryService.cpp
  support/FixtureFilesystemScanner.cpp
  support/StubProcessLauncher.cpp
  support/StubScriptEngine.cpp
)

target_include_directories(mod-manager-test-support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mod-manager-test-support PUBLIC domain core application)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(mod-manager-test-support PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
  )
endif()

add_executable(mod-manager-tests
  smoke_test.cpp
  unit/profile_identity_test.cpp
  unit/typed_identity_test.cpp
  unit/gamefs_plan_test.cpp
  unit/compiled_namespace_test.cpp
  unit/namespace_explanation_test.cpp
  unit/namespace_source_mapping_test.cpp
  unit/conflict_resolution_test.cpp
  unit/file_ownership_graph_test.cpp
  unit/package_dependency_graph_test.cpp
  unit/package_plan_test.cpp
  unit/package_root_mapping_test.cpp
  unit/planner_input_test.cpp
  unit/game_catalog_test.cpp
  unit/json_profile_persistence_test.cpp
  unit/json_package_catalog_test.cpp
  unit/catalog_inventory_service_test.cpp
  unit/inventory_model_test.cpp
  unit/service_registry_test.cpp
  unit/script_engine_test.cpp
  unit/process_launcher_test.cpp
  unit/fixture_filesystem_scanner_test.cpp
  unit/async_task_test.cpp
  unit/libarchive_package_detector_test.cpp
  unit/libarchive_reader_test.cpp
  unit/path_normalizer_test.cpp
  unit/rust_boundary_test.cpp
  unit/staging_repository_test.cpp
  integration/archive_ingestion_test.cpp
)

target_link_libraries(mod-manager-tests
    PRIVATE
    domain core application infrastructure ui mod-manager-test-support
    Catch2::Catch2WithMain
    Qt6::Core Qt6::Gui Qt6::Widgets
)

find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(FIXTURES_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/fixtures/corpus")
add_custom_command(
  OUTPUT "${FIXTURES_OUTPUT_DIR}/valid_simple.zip"
         "${FIXTURES_OUTPUT_DIR}/corrupt_magic.zip"
         "${FIXTURES_OUTPUT_DIR}/traversal_malicious.zip"
         "${FIXTURES_OUTPUT_DIR}/oversized_mock.zip"
         "${FIXTURES_OUTPUT_DIR}/nested_archive.zip"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${FIXTURES_OUTPUT_DIR}"
  COMMAND Python3::Interpreter "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/generate_corpus.py" "${FIXTURES_OUTPUT_DIR}"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/fixtures/generate_corpus.py"
  COMMENT "Generating test fixture corpus..."
)
add_custom_target(generate_fixtures DEPENDS "${FIXTURES_OUTPUT_DIR}/valid_simple.zip")
add_dependencies(mod-manager-tests generate_fixtures)

target_compile_definitions(mod-manager-tests PRIVATE 
  FIXTURES_DIR="${FIXTURES_OUTPUT_DIR}"
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

add_test(
  NAME production-binary-excludes-test-doubles
  COMMAND ${CMAKE_COMMAND}
          "-DNM_TOOL=${CMAKE_NM}"
          "-DBINARY=$<TARGET_FILE:mod-manager>"
          -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/verify_production_binary.cmake"
)
