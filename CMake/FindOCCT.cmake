# This script finds OCCT libraries.
#
# The script looks for OCCT default installation package,
# including TKernel and TKMath libraries. Both release
# and debug builds are considered.
#
# The script requires
#
#  OCCT_ROOT_DIR - root directory of OCCT library package.
#
# Once done the script will define
#
#  OCCT_FOUND - package is succesfully found.
#  OCCT_INCLUDE_DIRS - directory containing public headers files.
#  OCCT_LIBRARIES_RELEASE - release version of libraries.
#  OCCT_LIBRARIES_DEBUG - debug version of libraries.
#  OCCT_DLLS_RELEASE - release version of dlls.
#  OCCT_DLLS_DEBUG - debug version of dlls.

include(FindPackageHandleStandardArgs)

# =============================================================================
# Check for required variables
# =============================================================================

if (NOT OCCT_ROOT_DIR)
  message(FATAL_ERROR "OCCT_ROOT_DIR not found. Please locate before proceeding.")
endif()

# =============================================================================
# Check for required includes
# =============================================================================

find_path (OCCT_INCLUDE
  NAMES MMgt_TShared.hxx
  PATHS "${OCCT_ROOT_DIR}/inc"
  NO_DEFAULT_PATH
)

if (NOT OCCT_INCLUDE)
  message (SEND_ERROR "OCCT headers not found. Please locate OCCT_INCLUDE.")
endif()

# =============================================================================
# Check for required libraries
# =============================================================================

macro(OCCT_find_library name)
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
  if(WIN32)
    find_program (OCCT_${name}_RELEASE_DLL     ${name}.dll PATHS "${OCCT_ROOT_DIR}/${os_with_bit}/${compiler}/bin"  NO_DEFAULT_PATH)
    find_program (OCCT_${name}_DEBUG_DLL       ${name}.dll PATHS "${OCCT_ROOT_DIR}/${os_with_bit}/${compiler}/bind" NO_DEFAULT_PATH)
  endif()
  find_library (OCCT_${name}_RELEASE_LIBRARY ${name} PATHS "${OCCT_ROOT_DIR}/${os_with_bit}/${compiler}/lib"  NO_DEFAULT_PATH)
  find_library (OCCT_${name}_DEBUG_LIBRARY   ${name} PATHS "${OCCT_ROOT_DIR}/${os_with_bit}/${compiler}/libd" NO_DEFAULT_PATH)
  find_library (OCCT_${name}_RELEASE_LIBRARY ${name} PATHS "${OCCT_ROOT_DIR}/lib"  NO_DEFAULT_PATH)
  find_library (OCCT_${name}_DEBUG_LIBRARY   ${name} PATHS "${OCCT_ROOT_DIR}/libd" NO_DEFAULT_PATH)
  if (NOT OCCT_${name}_RELEASE_LIBRARY AND NOT OCCT_${name}_DEBUG_LIBRARY)
    message (SEND_ERROR "OCCT ${name} library not found. Please locate OCCT_${name}_RELEASE_LIBRARY or (and) OCCT_${name}_DEBUG_LIBRARY.")
  elseif (WIN32 AND NOT OCCT_${name}_RELEASE_DLL AND NOT OCCT_${name}_DEBUG_DLL)
    message (SEND_ERROR "OCCT ${name} dll not found. Please locate OCCT_${name}_RELEASE_DLL or (and) OCCT_${name}_DEBUG_DLL.")
  else()
    set (OCCT_${name}_LIBRARY_FOUND ON)
  endif()
endmacro()

OCCT_find_library (TKernel)
OCCT_find_library (TKMath)

FIND_PACKAGE_HANDLE_STANDARD_ARGS (OCCT DEFAULT_MSG OCCT_INCLUDE OCCT_TKernel_LIBRARY_FOUND OCCT_TKMath_LIBRARY_FOUND)

# =============================================================================
# Set "public" variables
# =============================================================================

if (OCCT_FOUND)
  set (OCCT_INCLUDE_DIRS      ${OCCT_INCLUDE})
  set (OCCT_LIBRARIES_RELEASE ${OCCT_TKernel_RELEASE_LIBRARY} ${OCCT_TKMath_RELEASE_LIBRARY})
  set (OCCT_LIBRARIES_DEBUG   ${OCCT_TKernel_DEBUG_LIBRARY} ${OCCT_TKMath_DEBUG_LIBRARY})
  set (OCCT_DLLS_RELEASE      ${OCCT_TKernel_RELEASE_DLL} ${OCCT_TKMath_RELEASE_DLL})
  set (OCCT_DLLS_DEBUG        ${OCCT_TKernel_DEBUG_DLL} ${OCCT_TKMath_DEBUG_DLL})
endif()
