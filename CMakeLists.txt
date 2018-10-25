cmake_minimum_required(VERSION 3.9)
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 1)
if($ENV{TRAVIS_BUILD_NUMBER})
    set(VERSION_PATCH $ENV{TRAVIS_BUILD_NUMBER})
endif()
set(MY_PROJECT_NAME bulk_server)
project(${MY_PROJECT_NAME})
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS system filesystem thread REQUIRED)
IF(Boost_FOUND)
  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
  LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})
ENDIF(Boost_FOUND)

add_executable(${MY_PROJECT_NAME} main.cpp bulk.cpp bulk_storage.cpp 
bulk_observer.cpp metrics.cpp)

set_target_properties(${MY_PROJECT_NAME}  PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        COMPILE_OPTIONS -Wpedantic -Wall -Wextra
        )
target_link_libraries(${MY_PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

set(BUILD_TEST ON)
if(${BUILD_TEST})
enable_testing()
include_directories(./)
add_subdirectory(tests)
endif()

install(TARGETS ${MY_PROJECT_NAME} RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${MY_PROJECT_NAME}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Sergio Matveev aka Feiler")
set(CPACK_PACKAGE_CONTACT srgmatveev@gmail.com)

include (CPack)