# Print a warning about ctest if using v2.6
if ( CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 8 )
  message ( WARNING " Running tests via CTest will not work with this version of CMake. If you need this functionality, upgrade to CMake 2.8." )
endif ()

# Include useful utils
include ( MantidUtils )

# Make the default build type Release
if ( NOT CMAKE_CONFIGURATION_TYPES )
  if ( NOT CMAKE_BUILD_TYPE )
    message ( STATUS "No build type selected, default to Release." )
    set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE )
    # Set the possible values of build type for cmake-gui
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
      "MinSizeRel" "RelWithDebInfo")
  else ()
    message ( STATUS "Build type is " ${CMAKE_BUILD_TYPE} )
  endif ()
endif()

# We want shared libraries everywhere
set ( BUILD_SHARED_LIBS On )

# Send libraries to common place
set ( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin )
set ( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin )

# This allows us to group targets logically in Visual Studio
set_property ( GLOBAL PROPERTY USE_FOLDERS ON )

# Look for stdint and add define if found
include ( CheckIncludeFiles )
check_include_files ( stdint.h stdint )
if ( stdint )
  add_definitions ( -DHAVE_STDINT_H )
endif ( stdint )

###########################################################################
# Look for dependencies - bail out if any not found
###########################################################################

set ( Boost_NO_BOOST_CMAKE TRUE )
find_package ( Boost REQUIRED date_time regex ) 
include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )
add_definitions ( -DBOOST_ALL_DYN_LINK )
# Need this defined globally for our log time values
add_definitions ( -DBOOST_DATE_TIME_POSIX_TIME_STD_CONFIG )

find_package ( Poco REQUIRED )
include_directories( SYSTEM ${POCO_INCLUDE_DIRS} )

find_package ( Nexus 4.3.0 REQUIRED )
include_directories ( SYSTEM ${NEXUS_INCLUDE_DIR} )

find_package ( MuParser REQUIRED )

find_package ( Doxygen ) # optional

# Need to change search path to find zlib include on Windows.
# Couldn't figure out how to extend CMAKE_INCLUDE_PATH variable for extra path
# so I'm caching old value, changing it temporarily and then setting it back
set ( MAIN_CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} )
set ( CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH}/zlib123 )
find_package ( ZLIB REQUIRED )
set ( CMAKE_INCLUDE_PATH ${MAIN_CMAKE_INCLUDE_PATH} )

find_package ( PythonInterp )
if ( MSVC )
  # Wrapper script to call either python or python_d depending on directory contents
  set ( PYTHON_EXE_WRAPPER_SRC "${CMAKE_CURRENT_SOURCE_DIR}/Build/win_python.bat" )
  set ( PYTHON_EXE_WRAPPER "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR}/win_python.bat" )
else()
  set ( PYTHON_EXE_WRAPPER ${PYTHON_EXECUTABLE} )
endif()

###########################################################################
# Look for Git. Used for version headers - faked if not found.
# Also makes sure our commit hooks are linked in the right place.
###########################################################################

set ( MtdVersion_WC_LAST_CHANGED_REV 0 )
set ( MtdVersion_WC_LAST_CHANGED_DATE Unknown )
set ( NOT_GIT_REPO "Not" )

find_package ( Git )
if ( GIT_FOUND )
  # Get the last revision
  execute_process ( COMMAND ${GIT_EXECUTABLE} describe --long
                    OUTPUT_VARIABLE GIT_DESCRIBE
                    ERROR_VARIABLE NOT_GIT_REPO
                    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  )
  if ( NOT NOT_GIT_REPO ) # i.e This is a git repository!

    # Remove the tag name part
    string ( REGEX MATCH "[-](.*)" MtdVersion_WC_LAST_CHANGED_REV ${GIT_DESCRIBE} )
    # Extract the SHA1 part (with a 'g' prefix which stands for 'git')
    # N.B. The variable comes back from 'git describe' with a line feed on the end, so we need to lose that
    string ( REGEX MATCH "(g.*)[^\n]" MtdVersion_WC_LAST_CHANGED_SHA ${MtdVersion_WC_LAST_CHANGED_REV} )
    # Get the number part (number of commits since tag)
    string ( REGEX MATCH "[0-9]+" MtdVersion_WC_LAST_CHANGED_REV ${MtdVersion_WC_LAST_CHANGED_REV} )
    # Get the date of the last commit
    execute_process ( COMMAND ${GIT_EXECUTABLE} log -1 --format=format:%cD OUTPUT_VARIABLE MtdVersion_WC_LAST_CHANGED_DATE 
                      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
    string ( SUBSTRING ${MtdVersion_WC_LAST_CHANGED_DATE} 0 16 MtdVersion_WC_LAST_CHANGED_DATE )

    ###########################################################################
    # This part puts our hooks (in .githooks) into .git/hooks
    ###########################################################################
    # First need to find the top-level directory of the git repository
    execute_process ( COMMAND ${GIT_EXECUTABLE} rev-parse --show-toplevel
                      OUTPUT_VARIABLE GIT_TOP_LEVEL
                      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    )
    # N.B. The variable comes back from 'git describe' with a line feed on the end, so we need to lose that
    string ( REGEX MATCH "(.*)[^\n]" GIT_TOP_LEVEL ${GIT_TOP_LEVEL} )
    # Prefer symlinks on platforms that support it so we don't rely on cmake running to be up-to-date
    # On Windows, we have to copy the file
    if ( WIN32 )
      execute_process ( COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GIT_TOP_LEVEL}/.githooks/pre-commit
                                                                      ${GIT_TOP_LEVEL}/.git/hooks )
      execute_process ( COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GIT_TOP_LEVEL}/.githooks/commit-msg
                                                                      ${GIT_TOP_LEVEL}/.git/hooks )
    else ()
      execute_process ( COMMAND ${CMAKE_COMMAND} -E create_symlink ${GIT_TOP_LEVEL}/.githooks/pre-commit
                                                                   ${GIT_TOP_LEVEL}/.git/hooks/pre-commit )
      execute_process ( COMMAND ${CMAKE_COMMAND} -E create_symlink ${GIT_TOP_LEVEL}/.githooks/commit-msg
                                                                   ${GIT_TOP_LEVEL}/.git/hooks/commit-msg )
    endif ()

  endif()

else()
  # Just use a dummy version number and print a warning
  message ( STATUS "Git not found - using dummy revision number and date" )
endif()

mark_as_advanced( MtdVersion_WC_LAST_CHANGED_REV MtdVersion_WC_LAST_CHANGED_DATE )

###########################################################################
# Include the file that contains the version number
# This must come after the git describe business above because it can be
# used to override the patch version number (MtdVersion_WC_LAST_CHANGED_REV)
###########################################################################

include ( VersionNumber )

if ( NOT NOT_GIT_REPO ) # i.e This is a git repository!
  ###########################################################################
  # Create the file containing the patch version number for use by cpack
  # The patch number make have been overridden by VerisonNumber so create
  # the file used by cpack here
  ###########################################################################
  configure_file ( ${GIT_TOP_LEVEL}/Code/Mantid/Build/CMake/PatchVersionNumber.cmake.in
                   ${GIT_TOP_LEVEL}/Code/Mantid/Build/CMake/PatchVersionNumber.cmake )
endif()

###########################################################################
# Look for OpenMP and set compiler flags if found
###########################################################################

find_package ( OpenMP )

###########################################################################
# Add linux-specific things
###########################################################################
if ( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
  include ( LinuxSetup )
endif ()

###########################################################################
# Add compiler options if using gcc
###########################################################################
if ( CMAKE_COMPILER_IS_GNUCXX )
  include ( GNUSetup )
endif ()

###########################################################################
# Setup cppcheck
###########################################################################
find_package ( Cppcheck )
if ( CPPCHECK_EXECUTABLE )
  set ( CPPCHECK_SOURCE_DIRS
        Framework
        MantidPlot
        MantidQt
        Vates
      )

  set ( CPPCHECK_USE_INCLUDE_DIRS OFF CACHE BOOL "Use specified include directories. WARNING: cppcheck will run significantly slower." )

  set ( CPPCHECK_INCLUDE_DIRS
        Framework/Algorithms/inc
        Framework/GPUAlgorithms/inc
        Framework/PythonInterface/inc
        Framework/Nexus/inc
        Framework/MPIAlgorithms/inc
        Framework/MDAlgorithms/inc
        Framework/DataHandling/inc
        Framework/WorkflowAlgorithms/inc
        Framework/MDEvents/inc
        Framework/DataObjects/inc
        Framework/Geometry/inc
        Framework/ICat/inc
        Framework/CurveFitting/inc
        Framework/API/inc
        Framework/TestHelpers/inc
        Framework/Crystal/inc
        Framework/PythonAPI/inc
        Framework/Kernel/inc
        Vates/VatesAPI/inc
        Vates/VatesSimpleGui/ViewWidgets/inc
        Vates/VatesSimpleGui/StandAloneExec/inc
        Vates/VatesSimpleGui/QtWidgets/inc
        MantidQt/MantidWidgets/inc
        MantidQt/CustomDialogs/inc
        MantidQt/DesignerPlugins/inc
        MantidQt/CustomInterfaces/inc
        MantidQt/API/inc
        MantidQt/Factory/inc
        MantidQt/SliceViewer/inc
      )

  set ( CPPCHECK_EXCLUDES
        Framework/DataHandling/src/LoadDAE/
        Framework/DataHandling/src/LoadRaw/
        Framework/ICat/inc/MantidICat/GSoapGenerated/
        Framework/ICat/src/GSoapGenerated/
        Framework/ICat/src/GSoapGenerated.cpp
        Framework/ICat/src/GSoap/
        Framework/ICat/src/GSoap.cpp
        Framework/Kernel/src/ANN/
        Framework/Kernel/src/ANN_complete.cpp
        Framework/PythonAPI/src/
        MantidPlot/src/nrutil.cpp
        MantidPlot/src/origin/OPJFile.cpp
      )

  # Header files to be ignored require different handling
  set ( CPPCHECK_HEADER_EXCLUDES
        MantidPlot/src/origin/OPJFile.h
		MantidPlot/src/origin/tree.hh
        Framework/PythonAPI/inc/boost/python/detail/referent_storage.hpp
        Framework/PythonAPI/inc/boost/python/detail/type_list_impl_no_pts.hpp
      )

  # setup the standard arguments
  set (_cppcheck_args "${CPPCHECK_ARGS}")
    list ( APPEND _cppcheck_args ${CPPCHECK_TEMPLATE_ARG} )
    if ( CPPCHECK_NUM_THREADS GREATER 0)
        list ( APPEND _cppcheck_args -j ${CPPCHECK_NUM_THREADS} )
  endif ( CPPCHECK_NUM_THREADS GREATER 0)

  # process list of include/exclude directories
  set (_cppcheck_source_dirs)
  foreach (_dir ${CPPCHECK_SOURCE_DIRS} )
    set ( _tmpdir "${CMAKE_SOURCE_DIR}/${_dir}" )
    if ( EXISTS ${_tmpdir} )
      list ( APPEND _cppcheck_source_dirs ${_tmpdir} )
    endif ()
  endforeach()

  set (_cppcheck_includes)
  foreach( _dir ${CPPCHECK_INCLUDE_DIRS} )
    set ( _tmpdir "${CMAKE_SOURCE_DIR}/${_dir}" )
    if ( EXISTS ${_tmpdir} )
      list ( APPEND _cppcheck_includes -I ${_tmpdir} )
    endif ()
  endforeach()
  if (CPPCHECK_USE_INCLUDE_DIRS)
    list ( APPEND _cppcheck_args ${_cppcheck_includes} )
  endif (CPPCHECK_USE_INCLUDE_DIRS)

  set (_cppcheck_excludes)
  foreach( _file ${CPPCHECK_EXCLUDES} )
    set ( _tmp "${CMAKE_SOURCE_DIR}/${_file}" )
    if ( EXISTS ${_tmp} )
      list ( APPEND _cppcheck_excludes -i ${_tmp} )
    endif ()
  endforeach()
  list ( APPEND _cppcheck_args ${_cppcheck_excludes} )

  # Handle header files in the required manner
  set (_cppcheck_header_excludes)
  foreach( _file ${CPPCHECK_HEADER_EXCLUDES} )
    set ( _tmp "${CMAKE_SOURCE_DIR}/${_file}" )
    if ( EXISTS ${_tmp} )
      list ( APPEND _cppcheck_header_excludes --suppress=*:${_tmp} )
    endif()
  endforeach()
  list ( APPEND _cppcheck_args ${_cppcheck_header_excludes} )

  # put the finishing bits on the final command call
  set (_cppcheck_xml_args)
  if (CPPCHECK_GENERATE_XML)
    list( APPEND _cppcheck_xml_args --xml --xml-version=2 ${_cppcheck_source_dirs} 2> ${CMAKE_BINARY_DIR}/cppcheck.xml )
  else (CPPCHECK_GENERATE_XML)
    list( APPEND _cppcheck_xml_args  ${_cppcheck_source_dirs} )
  endif (CPPCHECK_GENERATE_XML)

  # generate the target
  if (NOT TARGET cppcheck)
    add_custom_target ( cppcheck
                        COMMAND ${CPPCHECK_EXECUTABLE} ${_cppcheck_args} ${_cppcheck_header_excludes} ${_cppcheck_xml_args}
                        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                        COMMENT "Running cppcheck"
                      )
    set_target_properties(cppcheck PROPERTIES EXCLUDE_FROM_ALL TRUE)
  endif()
endif ( CPPCHECK_EXECUTABLE )


###########################################################################
# Set up the unit tests target
###########################################################################

find_package ( CxxTest )
if ( CXXTEST_FOUND )
  enable_testing ()
  add_custom_target( check COMMAND ${CMAKE_CTEST_COMMAND} )
  make_directory( ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Testing )
  message ( STATUS "Added target ('check') for unit tests" )
else ()
  message ( STATUS "Could NOT find CxxTest - unit testing not available" )
endif ()

# Some unit tests need GMock/GTest
find_package ( GMock )

if ( GMOCK_FOUND AND GTEST_FOUND )
  message ( STATUS "GMock/GTest is available for unit tests." )
else ()
  message ( STATUS "GMock/GTest is not available. Some unit tests will not run." ) 
endif()

find_package ( PyUnitTest )
if ( PYUNITTEST_FOUND )
  enable_testing ()
  message (STATUS "Found pyunittest generator")
else()
  message (STATUS "Could NOT find PyUnitTest - unit testing of python not available" )
endif()

# GUI testing via Squish
find_package ( Squish )
if ( SQUISH_FOUND )
  # CMAKE_MODULE_PATH gets polluted when ParaView is present
  set( MANTID_CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} )
  include( SquishAddTestSuite )
  enable_testing()
  message ( STATUS "Found Squish for GUI testing" )
else()
  message ( STATUS "Could not find Squish - GUI testing not available. Try specifying your SQUISH_INSTALL_DIR cmake variable." )
endif()

###########################################################################
# Set a flag to indicate that this script has been called
###########################################################################

set ( COMMONSETUP_DONE TRUE )
