# This script configures project for JT Assistant application.
#
# The script requires paths to 3rdparties and modules
#
#  FindOCCT.cmake / OCCT_ROOT_DIR
#  FindTBB.cmake  / TBB_ROOT_DIR
#  FindZLIB.cmake / ZLIB_ROOT_DIR
#  FindQt5Widgets.cmake / QT_ROOT_DIR
#  FindEigen3.cmake / EIGEN_ROOT_DIR
#  FindOpenGL / WINDOWSSDK_DIRS in CMAKE_PREFIX_PATH
#
# The script requires TKJT library target.
#
#  ../TKJT/CMakeLists.txt

function (QT5_CUSTOM_WRAP_UI outfiles )
  set (options)
  set (oneValueArgs)
  set (multiValueArgs OPTIONS)

  cmake_parse_arguments (_WRAP_UI "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set (ui_files ${_WRAP_UI_UNPARSED_ARGUMENTS})
  set (ui_options ${_WRAP_UI_OPTIONS})
  file (MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/gen)
  foreach (it ${ui_files})
    get_filename_component (outfile ${it} NAME_WE)
    get_filename_component (infile ${it} ABSOLUTE)
    set (outfile ${CMAKE_CURRENT_BINARY_DIR}/gen/ui_${outfile}.h)
    add_custom_command (OUTPUT ${outfile}
      COMMAND ${Qt5Widgets_UIC_EXECUTABLE}
      ARGS ${ui_options} -o ${outfile} ${infile}
      MAIN_DEPENDENCY ${infile} VERBATIM)
      list (APPEND ${outfiles} ${outfile})
    endforeach()
  set (${outfiles} ${${outfiles}} PARENT_SCOPE)
endfunction()

# Tell CMake to run moc when necessary:
set (CMAKE_AUTOMOC ON)
set (CMAKE_INCLUDE_CURRENT_DIR ON)

if (POLICY CMP0020)
  cmake_policy (SET CMP0020 NEW)
endif()

if (POLICY CMP0043)
  cmake_policy (SET CMP0043 NEW)
endif()

# =============================================================================
# Look for required dependencies
# =============================================================================

find_package(OCCT REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Eigen3 3.2.0 REQUIRED)
find_package(TBB REQUIRED)
find_package(OpenGL REQUIRED)
find_package(ZLIB REQUIRED)

mark_as_advanced(CLEAR ZLIB_LIBRARY)

if(WIN32)
  if(ZLIB_FOUND)
    get_filename_component(ZLIB_LIBRARY_DIR "${ZLIB_LIBRARY}" PATH)
    get_filename_component(ZLIB_LIBRARY_NAME "${ZLIB_LIBRARY}" NAME_WE)
    get_filename_component(ZLIB_ROOT_DIR "${ZLIB_LIBRARY_DIR}" PATH)
    find_program(ZLIB_DLL ${ZLIB_LIBRARY_NAME}.dll NAMES zlib1.dll PATHS "${ZLIB_ROOT_DIR}" "${ZLIB_ROOT_DIR}/bin" NO_DEFAULT_PATH)
  endif(ZLIB_FOUND)
endif(WIN32)

set (OPENGL_LIBRARIES ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})

get_target_property (TKJT_INCLUDE_DIRS TKJT INTERFACE_INCLUDE_DIRECTORIES)

# =============================================================================
# Define production steps
# =============================================================================

include_directories (
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/src/BVH
  ${CMAKE_CURRENT_SOURCE_DIR}/src/JTData
  ${CMAKE_CURRENT_SOURCE_DIR}/src/JTGui
  ${CMAKE_CURRENT_SOURCE_DIR}/src/JTVis
  ${OCCT_INCLUDE_DIRS}
  ${ZLIB_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIR}
  ${TKJT_INCLUDE_DIRS}
)

if(WIN32)
  add_definitions (-DWNT -DWINVER=0x0500)
else()
  add_definitions (-fexceptions -fPIC -DOCC_CONVERT_SIGNALS)
endif()

# Suppression of 3rdparty warnings in MSVC with #pragma warnings (disable)
if(MSVC)
  add_definitions (/wd4718)
else()
  add_definitions (-Wno-unknown-pragmas)
endif()

file (GLOB_RECURSE ProjectSources src/*.cxx)
file (GLOB_RECURSE ProjectHeaders src/*.hxx)
set (ProjectUI src/JTGui/MainWindow.ui)
set (ProjectResources JTAssistant.qrc src/JTVis/Shaders.qrc)

qt5_custom_wrap_ui (UISrcs ${ProjectUI})
qt5_add_resources (ResourceSrcs ${ProjectResources})

if (WIN32)
  enable_language (RC)
  set (EXE_ICON res/icons/desktop/icon.rc)
endif()

source_group ("Header Files" FILES
  ${ProjectHeaders}
)

source_group ("Resources" FILES
  ${ProjectUI}
  ${ProjectResources}
  ${EXE_ICON}
)

source_group ("Generated" FILES
  ${UISrcs}
  ${ResourceSrcs}
)

add_executable (JTAssistant ${ProjectSources} ${ProjectHeaders} ${UISrcs} ${ResourceSrcs} ${EXE_ICON})

add_dependencies (JTAssistant TKJT)

target_link_libraries_config_aware (JTAssistant OCCT)
target_link_libraries_config_aware (JTAssistant TBB)
target_link_libraries_config_aware (JTAssistant OPENGL)
target_link_libraries_config_aware (JTAssistant ZLIB)

target_link_libraries (JTAssistant TKJT)
target_link_libraries (JTAssistant Qt5::Widgets)

qt5_use_modules (JTAssistant Widgets)

if (WIN32)
  if (MSVC)
    set_target_properties (JTAssistant PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
  endif (MSVC)
endif (WIN32)

# =============================================================================
# Define install steps
# =============================================================================

install (TARGETS JTAssistant RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}")

if(WIN32)
  install_shared_libraries (OCCT_DLLS DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install_shared_libraries (ZLIB_DLL  DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install_shared_libraries (TBB_DLLS  DESTINATION "${CMAKE_INSTALL_PREFIX}")
else()
  install_shared_libraries (OCCT_LIBRARIES DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install_shared_libraries (ZLIB_LIBRARIES DESTINATION "${CMAKE_INSTALL_PREFIX}")
  install_shared_libraries (TBB_LIBRARIES  DESTINATION "${CMAKE_INSTALL_PREFIX}")
endif()

foreach (qtmodule Core Gui Widgets)
  get_target_property (QTMODULE_LIBRARY_DEBUG Qt5::${qtmodule} LOCATION_DEBUG)
  get_target_property (QTMODULE_LIBRARY_RELEASE Qt5::${qtmodule} LOCATION_RELEASE)
  install_shared_libraries (QTMODULE_LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}")
endforeach()

set (QT5_PLATFORM_INTEGRATION_PLUGINS QMinimalIntegrationPlugin QOffscreenIntegrationPlugin)
if (WIN32)
  list (APPEND QT5_PLATFORM_INTEGRATION_PLUGINS QWindowsIntegrationPlugin)
else()
  list (APPEND QT5_PLATFORM_INTEGRATION_PLUGINS QXcbIntegrationPlugin)
endif (WIN32)

foreach(plugin ${QT5_PLATFORM_INTEGRATION_PLUGINS})
  get_target_property(PLUGIN_LIBRARY_DEBUG Qt5::${plugin} LOCATION_DEBUG)
  get_target_property(PLUGIN_LIBRARY_RELEASE Qt5::${plugin} LOCATION_RELEASE)
  install_shared_libraries(PLUGIN_LIBRARY DESTINATION "${CMAKE_INSTALL_PREFIX}/platforms")
endforeach()

if (WIN32)
  file (GLOB_RECURSE QT_icudt_path "${QT_ROOT_DIR}/bin/icudt*.dll")
  file (GLOB_RECURSE QT_icuin_path "${QT_ROOT_DIR}/bin/icuin*.dll")
  file (GLOB_RECURSE QT_icuuc_path "${QT_ROOT_DIR}/bin/icuuc*.dll")
  find_program(QT_icudt_DLL "${QT_icudt_path}")
  find_program(QT_icuin_DLL "${QT_icuin_path}")
  find_program(QT_icuuc_DLL "${QT_icuuc_path}")
  if (QT_icudt_DLL AND QT_icudt_DLL AND QT_icuuc_DLL)
    install_shared_libraries (QT_icudt_DLL DESTINATION "${CMAKE_INSTALL_PREFIX}")
    install_shared_libraries (QT_icuin_DLL DESTINATION "${CMAKE_INSTALL_PREFIX}")
    install_shared_libraries (QT_icuuc_DLL DESTINATION "${CMAKE_INSTALL_PREFIX}")
  else()
    unset (QT_icudt_DLL CACHE)
    unset (QT_icuin_DLL CACHE)
    unset (QT_icuuc_DLL CACHE)
  endif()
endif()

if (WIN32)
  set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP ON)
  set (CMAKE_INSTALL_DEBUG_LIBRARIES ON)
  unset (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
  include (InstallRequiredSystemLibraries)
  set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_DEBUG ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})

  set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP ON)
  unset (CMAKE_INSTALL_DEBUG_LIBRARIES)
  unset (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
  include (InstallRequiredSystemLibraries)
  set (CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_RELEASE ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})

  install (PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_DEBUG} DESTINATION "${CMAKE_INSTALL_PREFIX}" CONFIGURATIONS ${CMAKE_DEBUG_CONFIGURATIONS})
  install (PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_RELEASE} DESTINATION "${CMAKE_INSTALL_PREFIX}" CONFIGURATIONS ${CMAKE_RELEASE_CONFIGURATIONS})
endif()

if(UNIX)
  string (CONCAT run_script ${run_script} "#!/bin/sh" "\n")
  string (CONCAT run_script ${run_script} "appname=JTAssistant" "\n")
  string (CONCAT run_script ${run_script} "dirname=`dirname \\\$0`"  "\n")
  string (CONCAT run_script ${run_script} "tmp=\\\"\\\${dirname#?}\\\"" "\n")
  string (CONCAT run_script ${run_script} "if [ \\\"\\\${dirname%\\\$tmp}\\\" != \\\"/\\\" ]" "\n")
  string (CONCAT run_script ${run_script} "then" "\n")
  string (CONCAT run_script ${run_script} "dirname=\\\$PWD/\\\$dirname" "\n")
  string (CONCAT run_script ${run_script} "fi" "\n")
  string (CONCAT run_script ${run_script} "LD_LIBRARY_PATH=\\\$dirname" "\n")
  string (CONCAT run_script ${run_script} "export LD_LIBRARY_PATH" "\n")
  string (CONCAT run_script ${run_script} "\\\"\\\$dirname/\\\$appname\\\" -platform xcb" "\n")
  install (CODE "file(WRITE \"${CMAKE_BINARY_DIR}/JTAssistant.sh\" \"${run_script}\")")
  install (FILES "${CMAKE_BINARY_DIR}/JTAssistant.sh" DESTINATION "${CMAKE_INSTALL_PREFIX}" 
           PERMISSIONS 
           OWNER_EXECUTE OWNER_READ OWNER_WRITE
           GROUP_EXECUTE GROUP_READ
           WORLD_EXECUTE WORLD_READ)
endif()

# =============================================================================
# Copy documentation
# =============================================================================

install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/README.txt" DESTINATION "${CMAKE_INSTALL_PREFIX}" OPTIONAL)