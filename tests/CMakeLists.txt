find_package(Catch2 3 CONFIG REQUIRED)

add_executable(mod-manager-tests
  smoke_test.cpp
  unit/profile_identity_test.cpp
  unit/json_profile_persistence_test.cpp
  unit/inventory_model_test.cpp
  unit/service_registry_test.cpp
  unit/script_engine_test.cpp
  unit/process_launcher_test.cpp
)

target_link_libraries(mod-manager-tests
  PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    domain
    core
    application
    infrastructure
    ui
    Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(mod-manager-tests)
