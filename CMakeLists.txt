cmake_minimum_required(VERSION 3.1)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
  "${CMAKE_SOURCE_DIR}/Modules/")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

set (PROJECT ICFPC2019)
set (APPNAME "icfpc19")

project (${PROJECT})
set (${PROJECT}_VERSION_MAJOR 0)
set (${PROJECT}_VERSION_MINOR 3)
set (${PROJECT}_VERSION_PATCH 0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

set(SOURCE_FILES
    main.cpp
    cMap.cpp
    cStrategy.cpp
    cWorker.cpp
)

set(HEADER_FILES
    main.h
    cMap.h
    cStrategy.h
    cWorker.h
)

find_path(LEMON_SOURCE_ROOT_DIR CMakeLists.txt
  PATHS "${CMAKE_SOURCE_DIR}/libs/lemon-1.3.1"
  NO_DEFAULT_PATH
  DOC "Location of LEMON source as a CMAKE subproject")

if(EXISTS ${LEMON_SOURCE_ROOT_DIR})
  add_subdirectory(${LEMON_SOURCE_ROOT_DIR} deps/lemon)
  set(LEMON_INCLUDE_DIRS
    ${LEMON_SOURCE_ROOT_DIR}
    ${CMAKE_BINARY_DIR}/deps/lemon
  )
  set(LEMON_LIBRARIES lemon)
  unset(LEMON_ROOT_DIR CACHE)
  unset(LEMON_DIR CACHE)
  unset(LEMON_INCLUDE_DIR CACHE)
  unset(LEMON_LIBRARY CACHE)
else()
  find_package(LEMON QUIET NO_MODULE)
  find_package(LEMON REQUIRED)
endif()

include_directories(
  ${PROJECT_SOURCE_DIR}
  ${LEMON_INCLUDE_DIRS}
)

add_executable(${APPNAME} ${SOURCE_FILES} ${HEADER_FILES})
