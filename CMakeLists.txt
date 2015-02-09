# This script configures the JT solution to build.
#
# The repository contains multiple project
#  TKJT toolkit
#  JT Assistant
#
# You may choose between them for generating a complete solution configured
# for all-sufficient build and installation of the project.

cmake_minimum_required (VERSION 2.8.10 FATAL_ERROR)

project (JT)

# =============================================================================
# Configure CMake
# =============================================================================

set(CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/CMake"
  ${CMAKE_MODULE_PATH}
)

include (DefineMacro)
include (DefineCxxFlags)

# =============================================================================
# Prompt user options
# =============================================================================

set (BUILD_PROJECT "JTAssistant" CACHE STRING "Select a project to build: [JTAssistant, TKJT].")

set_property (CACHE BUILD_PROJECT PROPERTY STRINGS "JTAssistant" "TKJT")

if (BUILD_PROJECT STREQUAL "JTAssistant")
  option (BUILD_TKJT "Set this option to include JT toolkit build into solution." ON)
else()
  unset (BUILD_TKJT CACHE)
  set (BUILD_TKJT ON)
endif()

# =============================================================================
# Configure (prompt) primary 3rdparties and dependencies
# =============================================================================

if (BUILD_PROJECT STREQUAL "JTAssistant")
  if(WIN32)
    find_package (WindowsSDK)
  endif()
  set (EIGEN_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/JTAssistant/Eigen3" CACHE PATH "Path to \"Eigen\" library.")
  set (QT_ROOT_DIR   "" CACHE PATH "Path to Qt libraries.")
else()
  unset (QT_ROOT_DIR CACHE)
  unset (EIGEN_ROOT_DIR CACHE)
  unset (WINDOWSSDK_DIRS CACHE)
endif()

set (OCCT_ROOT_DIR "" CACHE PATH "Path to Open CASCADE libraries.")
set (ZLIB_ROOT_DIR "" CACHE PATH "Path to \"zlib\" compression library.")
set (TBB_ROOT_DIR  "" CACHE PATH "Path to TBB multithreading libraries.")

set (CMAKE_PREFIX_PATH "${WINDOWSSDK_DIRS};${QT_ROOT_DIR};${EIGEN_ROOT_DIR};${ZLIB_ROOT_DIR};${TBB_ROOT_DIR}" ${CMAKE_PREFIX_PATH})

# =============================================================================
# Define internal variables
# =============================================================================

set (CMAKE_DEBUG_CONFIGURATIONS Debug)
set (CMAKE_RELEASE_CONFIGURATIONS Release RelWithDebInfo MinSizeRel)

# =============================================================================
# Include projects or dependencies into solution
# =============================================================================

if (BUILD_PROJECT STREQUAL "TKJT")
  add_subdirectory (TKJT)
endif()

if (BUILD_PROJECT STREQUAL "JTAssistant")
  add_subdirectory (TKJT)
  add_subdirectory (JTAssistant)
endif()

# =============================================================================
# Copy documentation
# =============================================================================

install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt" DESTINATION "${CMAKE_INSTALL_PREFIX}" OPTIONAL)