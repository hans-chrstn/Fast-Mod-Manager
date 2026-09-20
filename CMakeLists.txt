cmake_minimum_required(VERSION 3.28)

project(mod-manager VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES})
set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

option(MOD_MANAGER_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Gui Widgets)
find_package(LibArchive REQUIRED)

qt_standard_project_setup()

find_package(Corrosion REQUIRED)

corrosion_import_crate(MANIFEST_PATH src/rust/fmm-core/Cargo.toml)

add_library(domain OBJECT
  src/domain/ProfileIdentity.cpp
  src/domain/GameDefinition.hpp
)
target_include_directories(domain PUBLIC src)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(domain PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
  )
endif()

add_library(core OBJECT
  src/core/IPlugin.hpp
  src/core/IGameCatalog.hpp
  src/core/IGamePluginLoader.hpp
  src/core/IGamePluginScanner.hpp
  src/core/IActiveGameSelection.hpp
  src/core/IModScanner.hpp
  src/core/PathNormalizer.cpp
)
target_include_directories(core PUBLIC src)
target_link_libraries(core PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(core PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
  )
endif()

add_library(application OBJECT
  src/application/FakeInventoryService.hpp
  src/application/GameCatalogImpl.cpp
  src/application/ActiveGameSelectionImpl.cpp
  src/application/InventoryService.hpp
)
target_include_directories(application PUBLIC src)
target_link_libraries(application PUBLIC domain core)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(application PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
  )
endif()

add_library(infrastructure OBJECT
  src/infrastructure/JsonProfilePersistence.cpp
  src/infrastructure/StubScriptEngine.cpp
  src/infrastructure/StubProcessLauncher.cpp
  src/infrastructure/FixtureFilesystemScanner.cpp
  src/infrastructure/FilesystemGamePluginScanner.cpp
  src/infrastructure/LuaGamePluginLoader.cpp
  src/infrastructure/LibArchivePackageDetector.cpp
  src/infrastructure/DependencyValidatorImpl.cpp
  src/infrastructure/GameFsClient.cpp
  src/infrastructure/LibArchiveReader.cpp
  src/infrastructure/StagingRepositoryImpl.cpp
)
target_include_directories(infrastructure PUBLIC src)
target_link_libraries(infrastructure PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets domain core LibArchive::LibArchive fmm_core)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(infrastructure PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
  )
endif()

add_library(ui OBJECT
  src/ui/InventoryModel.cpp
  src/ui/InventoryWidget.cpp
  src/ui/ProgressOverlayWidget.cpp
  src/ui/MainWindow.cpp
)
target_include_directories(ui PUBLIC src)
target_link_libraries(ui PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets domain application)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(ui PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
  )
endif()

add_library(app OBJECT
  src/app/Bootstrapper.cpp
)
target_include_directories(app PUBLIC src)
target_link_libraries(app PUBLIC Qt6::Core Qt6::Gui Qt6::Widgets domain core application infrastructure ui)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(app PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
  )
endif()

add_executable(mod-manager
  src/main.cpp
)

target_link_libraries(mod-manager PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets domain core application infrastructure ui app)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(mod-manager PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wsign-conversion
  )
endif()

if(MOD_MANAGER_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(mod-manager PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(mod-manager PRIVATE -fsanitize=address,undefined)
endif()

include(CTest)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

install(TARGETS mod-manager RUNTIME DESTINATION bin)

