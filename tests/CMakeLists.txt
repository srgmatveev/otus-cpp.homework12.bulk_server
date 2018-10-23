FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(GTest REQUIRED)
set(GMOCK_BOTH_LIBRARIES gmock gmock_main)
include_directories(${GTEST_INCLUDE_DIRS})

function(test name)
  add_executable (${name} run_test.cpp utils_test.cpp bulk_test.cpp)
  target_link_libraries(${name} ${GTEST_BOTH_LIBRARIES})
  target_link_libraries(${name} ${GMOCK_BOTH_LIBRARIES})
  target_link_libraries(${name} ${CMAKE_THREAD_LIBS_INIT})
  add_test(${name} ${CMAKE_BINARY_DIR}/tests/${name})
  set_target_properties(${name}  PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        COMPILE_OPTIONS -Wpedantic -Wall -Wextra
        )
  endfunction(test)

test(unit_test)