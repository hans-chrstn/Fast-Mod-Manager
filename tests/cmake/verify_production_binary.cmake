if(NOT DEFINED NM_TOOL OR NM_TOOL STREQUAL "")
  message(FATAL_ERROR "CMake did not provide a symbol inspection tool")
endif()

if(NOT DEFINED BINARY OR NOT EXISTS "${BINARY}")
  message(FATAL_ERROR "Production binary does not exist: ${BINARY}")
endif()

execute_process(
  COMMAND "${NM_TOOL}" "${BINARY}"
  RESULT_VARIABLE inspection_result
  OUTPUT_VARIABLE production_symbols
  ERROR_VARIABLE inspection_error
)

if(NOT inspection_result EQUAL 0)
  message(FATAL_ERROR "Failed to inspect production binary: ${inspection_error}")
endif()

foreach(test_double
    FakeInventoryService
    FixtureFilesystemScanner
    StubProcessLauncher
    StubScriptEngine)
  string(FIND "${production_symbols}" "${test_double}" symbol_position)
  if(NOT symbol_position EQUAL -1)
    message(FATAL_ERROR "Production binary contains test double: ${test_double}")
  endif()
endforeach()
