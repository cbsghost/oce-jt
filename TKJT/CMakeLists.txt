# This script configures project for building or importing TKJT library.
#
# In case of importing the TKJT library:
#
#  TKJT_ROOT_DIR should be defined to refer the TKJT library path.
#
# In case of building the TKJT library:
#
#  The script requires paths to 3rdparties and modules
#
#   FindOCE.cmake / OCE_ROOT_DIR
#   FindTBB.cmake  / TBB_ROOT_DIR
#   FindZLIB.cmake / ZLIB_ROOT_DIR

project (TKJT)

if (NOT BUILD_TKJT)
  set (TKJT_ROOT_DIR "" CACHE PATH "Path to prebuilt TKJT libraries.")
  macro(TKJT_find_library)
    math (EXPR compiler_bitness "32 + 32*(${CMAKE_SIZEOF_VOID_P}/8)")
    if (WIN32)
      set (os_with_bit "win${compiler_bitness}")
    elseif (APPLE)
      set (os_with_bit "mac${compiler_bitness}")
    else()
      set (os_with_bit "lin${compiler_bitness}")
    endif()
    if (MSVC)
      if (MSVC70)
        set (compiler vc7)
      elseif (MSVC80)
        set (compiler vc8)
      elseif (MSVC90)
        set (compiler vc9)
      elseif (MSVC10)
        set (compiler vc10)
      elseif (MSVC11)
        set (compiler vc11)
      elseif (MSVC12)
        set (compiler vc12)
      endif()
    elseif (DEFINED CMAKE_COMPILER_IS_GNUCC)
      set (compiler gcc)
    elseif (DEFINED CMAKE_COMPILER_IS_GNUCXX)
      set (compiler gxx)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
      set (compiler clang)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
      set (compiler icc)
    else()
      set (compiler ${CMAKE_GENERATOR})
      string (REGEX REPLACE " " "" compiler ${compiler})
    endif()
    if (WIN32)
      find_program (TKJT_RELEASE_DLL     TKJT.dll PATHS "${TKJT_ROOT_DIR}/${os_with_bit}/${compiler}/bin"  NO_DEFAULT_PATH)
      find_program (TKJT_DEBUG_DLL       TKJT.dll PATHS "${TKJT_ROOT_DIR}/${os_with_bit}/${compiler}/bind" NO_DEFAULT_PATH)
    endif()
    find_library (TKJT_RELEASE_LIBRARY TKJT PATHS "${TKJT_ROOT_DIR}/${os_with_bit}/${compiler}/lib"  NO_DEFAULT_PATH)
    find_library (TKJT_DEBUG_LIBRARY   TKJT PATHS "${TKJT_ROOT_DIR}/${os_with_bit}/${compiler}/libd" NO_DEFAULT_PATH)
    if (NOT TKJT_RELEASE_LIBRARY AND NOT TKJT_DEBUG_LIBRARY)
      message (SEND_ERROR "TKJT library not found. Please locate TKJT_RELEASE_LIBRARY or (and) TKJT_DEBUG_LIBRARY.")
    elseif (WIN32 AND NOT TKJT_RELEASE_DLL AND NOT TKJT_DEBUG_DLL)
      message (SEND_ERROR "TKJT dll not found. Please locate TKJT_RELEASE_DLL or (and) TKJT_DEBUG_DLL.")
    else()
      set (TKJT_LIBRARY_FOUND ON)
    endif()
  endmacro()

  find_path (TKJT_INCLUDE
    NAMES JtAttribute_Base.hxx
    PATHS "${TKJT_ROOT_DIR}/inc"
    NO_DEFAULT_PATH)

  if (NOT TKJT_INCLUDE)
    message (SEND_ERROR "TKJT library headers not found. Please locate TKJT_INCLUDE.")
  endif()

  TKJT_find_library()

  if (NOT TKJT_LIBRARY_FOUND)
    message (FATAL_ERROR "Please locate prebuilt TKJT library.")
  endif()

  if(TKJT_LIBRARY_FOUND)
    set (TKJT_INCLUDE_DIRS    ${TKJT_INCLUDE})
    set (TKJT_LIBRARY_RELEASE ${TKJT_RELEASE_LIBRARY})
    set (TKJT_LIBRARY_DEBUG   ${TKJT_DEBUG_LIBRARY})
    set (TKJT_DLL_RELEASE     ${TKJT_RELEASE_DLL})
    set (TKJT_DLL_DEBUG       ${TKJT_DEBUG_DLL})
  endif()

  add_library (TKJT SHARED IMPORTED GLOBAL)

  set_target_properties (TKJT PROPERTIES IMPORTED_CONFIGURATIONS DEBUG)
  set_target_properties (TKJT PROPERTIES IMPORTED_CONFIGURATIONS RELEASE)
  set_target_properties (TKJT PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${TKJT_INCLUDE_DIRS})
  set_target_properties (TKJT PROPERTIES IMPORTED_LOCATION_DEBUG ${TKJT_LIBRARY_DEBUG})
  set_target_properties (TKJT PROPERTIES IMPORTED_LOCATION_RELEASE ${TKJT_LIBRARY_RELEASE})
  if (WIN32)
    set_target_properties (TKJT PROPERTIES IMPORTED_IMPLIB_DEBUG ${TKJT_DLL_DEBUG})
    set_target_properties (TKJT PROPERTIES IMPORTED_IMPLIB_RELEASE ${TKJT_DLL_RELEASE})
  endif()
  
  if (WIN32)
    install_shared_libraries (TKJT_DLL DESTINATION "${CMAKE_INSTALL_PREFIX}")
  else()
    install_shared_libraries (TKJT_LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}")
  endif()

else()

  unset (TKJT_INCLUDE         CACHE)
  unset (TKJT_ROOT_DIR        CACHE)
  unset (TKJT_INCLUDE_DIRS    CACHE)
  unset (TKJT_RELEASE_LIBRARY CACHE)
  unset (TKJT_DEBUG_LIBRARY   CACHE)
  unset (TKJT_RELEASE_DLL     CACHE)
  unset (TKJT_DEBUG_DLL       CACHE)

  # =============================================================================
  # Look for required dependencies
  # =============================================================================

  find_package (OCE REQUIRED)
  set(OCE_LIBRARIES "TKernel;TKMath")
  find_package (TBB REQUIRED)
  find_package (ZLIB REQUIRED)

  if(WIN32)
    if(ZLIB_FOUND)
      get_filename_component(ZLIB_LIBRARY_DIR "${ZLIB_LIBRARY}" PATH)
      get_filename_component(ZLIB_LIBRARY_NAME "${ZLIB_LIBRARY}" NAME_WE)
      get_filename_component(ZLIB_ROOT_DIR "${ZLIB_LIBRARY_DIR}" PATH)
      find_program(ZLIB_DLL ${ZLIB_LIBRARY_NAME}.dll NAMES zlib1.dll PATHS "${ZLIB_ROOT_DIR}" "${ZLIB_ROOT_DIR}/bin" NO_DEFAULT_PATH)
    endif(ZLIB_FOUND)
  endif(WIN32)

  # =============================================================================
  # Define production steps
  # =============================================================================

  set (TKJT_HEADER_DIRS)
  set (TKJT_HEADERS)
  set (TKJT_SOURCES)
  set (TKJT_PACKAGES)

  include_directories(
    ${OCE_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIRS}
  )

  # =============================================================================
  # Define production steps : search sources
  # =============================================================================

  set (CMAKE_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)

  file (GLOB SRC_DIRS RELATIVE ${CMAKE_SOURCE_PATH} ${CMAKE_SOURCE_PATH}/*)
  foreach (SRC_DIR ${SRC_DIRS})
    if (IS_DIRECTORY ${CMAKE_SOURCE_PATH}/${SRC_DIR})
      list (APPEND TKJT_PACKAGES ${SRC_DIR})
    endif()
  endforeach()

  foreach (TKJT_PACKAGE ${TKJT_PACKAGES})
    file (GLOB_RECURSE PACKAGE_HEADERS ${CMAKE_SOURCE_PATH}/${TKJT_PACKAGE}/*.hxx)
    file (GLOB_RECURSE PACKAGE_SOURCES ${CMAKE_SOURCE_PATH}/${TKJT_PACKAGE}/*.cxx)
    source_group ("Header Files\\${TKJT_PACKAGE}" FILES ${PACKAGE_HEADERS})
    source_group ("Source Files\\${TKJT_PACKAGE}" FILES ${PACKAGE_SOURCES})
    list (APPEND TKJT_SOURCES ${PACKAGE_SOURCES})
    list (APPEND TKJT_HEADERS ${PACKAGE_HEADERS})
    list (APPEND TKJT_HEADER_DIRS ${CMAKE_SOURCE_PATH}/${TKJT_PACKAGE})
  endforeach()

  include_directories (${TKJT_HEADER_DIRS})

  add_library (TKJT SHARED ${TKJT_HEADERS} ${TKJT_SOURCES})

  target_link_libraries_config_aware (TKJT OCE)
  target_link_libraries_config_aware (TKJT TBB)
  target_link_libraries_config_aware (TKJT ZLIB)

  if (BUILD_PROJECT STREQUAL "TKJT")
    set_target_properties (TKJT PROPERTIES PUBLIC_HEADER "${TKJT_HEADERS}")
  endif()

  set_target_properties (TKJT PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${TKJT_HEADER_DIRS}")

  # =============================================================================
  # Define install steps
  # =============================================================================

  math (EXPR compiler_bitness "32 + 32*(${CMAKE_SIZEOF_VOID_P}/8)")
  if (WIN32)
    set (os_with_bit "win${compiler_bitness}")
  elseif (APPLE)
    set (os_with_bit "mac${compiler_bitness}")
  else()
    set (os_with_bit "lin${compiler_bitness}")
  endif()
  if (MSVC)
    if (MSVC70)
      set (compiler vc7)
    elseif (MSVC80)
      set (compiler vc8)
    elseif (MSVC90)
      set (compiler vc9)
    elseif (MSVC10)
      set (compiler vc10)
    elseif (MSVC11)
      set (compiler vc11)
    elseif (MSVC12)
      set (compiler vc12)
    endif()
  elseif (DEFINED CMAKE_COMPILER_IS_GNUCC)
    set (compiler gcc)
  elseif (DEFINED CMAKE_COMPILER_IS_GNUCXX)
    set (compiler gxx)
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set (compiler clang)
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    set (compiler icc)
  else()
    set (compiler ${CMAKE_GENERATOR})
    string (REGEX REPLACE " " "" compiler ${compiler})
  endif()
  if (BUILD_PROJECT STREQUAL "TKJT")
    set (RUNTIME_DIR ${CMAKE_INSTALL_PREFIX}/${os_with_bit}/${compiler}/bin)
    set (LIBRARY_DIR ${CMAKE_INSTALL_PREFIX}/${os_with_bit}/${compiler}/lib)
    set (ARCHIVE_DIR ${CMAKE_INSTALL_PREFIX}/${os_with_bit}/${compiler}/lib)
    set (PUBLIC_HEADER_DIR ${CMAKE_INSTALL_PREFIX}/inc)
    install (TARGETS TKJT RUNTIME       DESTINATION "${RUNTIME_DIR}"       CONFIGURATIONS ${CMAKE_RELEASE_CONFIGURATIONS}
                          LIBRARY       DESTINATION "${LIBRARY_DIR}"       CONFIGURATIONS ${CMAKE_RELEASE_CONFIGURATIONS}
                          ARCHIVE       DESTINATION "${ARCHIVE_DIR}"       CONFIGURATIONS ${CMAKE_RELEASE_CONFIGURATIONS}
                          PUBLIC_HEADER DESTINATION "${PUBLIC_HEADER_DIR}" CONFIGURATIONS ${CMAKE_RELEASE_CONFIGURATIONS})
    install (TARGETS TKJT RUNTIME       DESTINATION "${RUNTIME_DIR}d"      CONFIGURATIONS ${CMAKE_DEBUG_CONFIGURATIONS}
                          LIBRARY       DESTINATION "${LIBRARY_DIR}d"      CONFIGURATIONS ${CMAKE_DEBUG_CONFIGURATIONS}
                          ARCHIVE       DESTINATION "${ARCHIVE_DIR}d"      CONFIGURATIONS ${CMAKE_DEBUG_CONFIGURATIONS}
                          PUBLIC_HEADER DESTINATION "${PUBLIC_HEADER_DIR}" CONFIGURATIONS ${CMAKE_DEBUG_CONFIGURATIONS})
  else()
    if (WIN32)
      install (TARGETS TKJT RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}")
    else()
      install (TARGETS TKJT LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}")
    endif()
  endif()
endif()
