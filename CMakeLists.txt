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

qt_standard_project_setup()

add_executable(mod-manager
  src/main.cpp
)

target_link_libraries(mod-manager PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)

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
