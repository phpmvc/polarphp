# This source file is part of the polarphp.org open source project
#
# Copyright (c) 2017 - 2018 polarphp software foundation
# Copyright (c) 2017 - 2018 zzu_softboy <zzu_softboy@163.com>
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See http://polarphp.org/LICENSE.txt for license information
# See http://polarphp.org/CONTRIBUTORS.txt for the list of polarphp project authors
#
# Created by polarboy on 2018/08/17.

cmake_minimum_required(VERSION 3.10.2)

if(POLICY CMP0068)
   cmake_policy(SET CMP0068 NEW)
   set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR ON)
endif()

project(polarphp C CXX)

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
   message(STATUS "No build type selected, default to Debug")
   set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type (default Debug)" FORCE)
endif()

# setup some important path

set(POLAR_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}") # --src-root
set(POLAR_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}") # --prefix
set(POLAR_UNITTEST_BINARY_DIR "${POLAR_BINARY_DIR}/${CMAKE_CFG_INTDIR}/unittestbins")
set(POLAR_THIRDPARTY_DIR "${POLAR_SOURCE_DIR}/thirdparty")
set(POLAR_DEPS_INSTALL_DIR "${POLAR_BINARY_DIR}/depslibs")
set(POLAR_CMAKE_MODULES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
set(POLAR_MAIN_INCLUDE_DIR "${POLAR_SOURCE_DIR}/include")
set(POLAR_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include") # --includedir
set(POLAR_MAIN_INCLUDE_DIR "${POLAR_SOURCE_DIR}/include")
set(POLAR_TEST_DIR "${POLAR_SOURCE_DIR}/tests")
set(POLAR_CMAKE_TEST_CODE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake/codes)
set(POLAR_SYSTEM_NAME ${CMAKE_HOST_SYSTEM_NAME})
string(TOLOWER ${POLAR_SYSTEM_NAME} POLAR_SYSTEM_NORMAL_NAME)
set(POLAR_CONFIGURE_TEMP_DIR ${POLAR_BINARY_DIR}/conf_temp)
set(POLAR_HEADER_INSTALL_DIR ${CMAKE_PREFIX_PATH}/include/php)

# Add path for custom CMake modules.
list(APPEND CMAKE_MODULE_PATH
   "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
   "${POLAR_CMAKE_MODULES_DIR}")

# Make a job pool for things that can't yet be distributed
cmake_host_system_information(
   RESULT localhost_logical_cores QUERY NUMBER_OF_LOGICAL_CORES)
set_property(GLOBAL PROPERTY JOB_POOLS local_jobs=${localhost_logical_cores})
# Put linking in that category
set_property(GLOBAL PROPERTY JOB_POOL_LINK local_jobs)

# This should only apply if you are both on an Apple host, and targeting Apple.
if(CMAKE_HOST_APPLE AND APPLE)
   # if CMAKE_LIBTOOL is not set, try and find it with xcrun or find_program
   if(NOT CMAKE_LIBTOOL)
      if(NOT CMAKE_XCRUN)
         find_program(CMAKE_XCRUN NAMES xcrun)
      endif()
      if(CMAKE_XCRUN)
         execute_process(COMMAND ${CMAKE_XCRUN} -find libtool
            OUTPUT_VARIABLE CMAKE_LIBTOOL
            OUTPUT_STRIP_TRAILING_WHITESPACE)
      endif()

      if(NOT CMAKE_LIBTOOL OR NOT EXISTS CMAKE_LIBTOOL)
         find_program(CMAKE_LIBTOOL NAMES libtool)
      endif()
   endif()

   get_property(languages GLOBAL PROPERTY ENABLED_LANGUAGES)
   if(CMAKE_LIBTOOL)
      set(CMAKE_LIBTOOL ${CMAKE_LIBTOOL} CACHE PATH "libtool executable")
      message(STATUS "Found libtool - ${CMAKE_LIBTOOL}")

      execute_process(COMMAND ${CMAKE_LIBTOOL} -V
         OUTPUT_VARIABLE LIBTOOL_V_OUTPUT
         OUTPUT_STRIP_TRAILING_WHITESPACE)
      if("${LIBTOOL_V_OUTPUT}" MATCHES ".*cctools-([0-9.]+).*")
         string(REGEX REPLACE ".*cctools-([0-9.]+).*" "\\1" LIBTOOL_VERSION
            ${LIBTOOL_V_OUTPUT})
         if(NOT LIBTOOL_VERSION VERSION_LESS "862")
            set(LIBTOOL_NO_WARNING_FLAG "-no_warning_for_no_symbols")
         endif()
      endif()

      foreach(lang ${languages})
         set(CMAKE_${lang}_CREATE_STATIC_LIBRARY
            "\"${CMAKE_LIBTOOL}\" -static ${LIBTOOL_NO_WARNING_FLAG} -o <TARGET> \
            <LINK_FLAGS> <OBJECTS> ")
      endforeach()
   endif()

   # If DYLD_LIBRARY_PATH is set we need to set it on archiver commands
   if(DYLD_LIBRARY_PATH)
      set(dyld_envar "DYLD_LIBRARY_PATH=${DYLD_LIBRARY_PATH}")
      foreach(lang ${languages})
         foreach(cmd ${CMAKE_${lang}_CREATE_STATIC_LIBRARY})
            list(APPEND CMAKE_${lang}_CREATE_STATIC_LIBRARY_NEW
               "${dyld_envar} ${cmd}")
         endforeach()
         set(CMAKE_${lang}_CREATE_STATIC_LIBRARY
            ${CMAKE_${lang}_CREATE_STATIC_LIBRARY_NEW})
      endforeach()
   endif()
endif()

if (NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE "Debug")
endif()

string(TOLOWER ${CMAKE_BUILD_TYPE} POLAR_BUILD_TYPE)
string(TOUPPER ${CMAKE_BUILD_TYPE} POLAR_BUILD_CONFIG)

if (CMAKE_BUILD_TYPE AND
      NOT POLAR_BUILD_TYPE MATCHES "^(debug|release|relwithdebinfo|minsizerel)$")
   message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
endif()

# First include general CMake utilities.
include(PolarUtils)
include(SummaryOutput)
include(ProcessPhpVersion)
include(ThreadUtils)
include(CheckUtils)
include(AddDefinitions)
include(AddUnittests)
include(GetHostTriple)
include(TargetUtils)
include(ProcessSources)
include(GetHostTriple)

set(CMAKE_C_STANDARD_REQUIRED 11)
set(CMAKE_CXX_STANDARD_REQUIRED 17)

option(BUILD_SHARED_LIBS
   "Build all libraries as shared libraries instead of static" ON)

# setup options
option(POLAR_ENABLE_RE2C_CGOTO
   "re2c-cgoto, whether to enable computed goto gcc extension with re2c"
   OFF)

option(POLAR_ENABLE_LIBGCC
   "Enable explicitly linking against libgcc"
   OFF)

option(POLAR_WITH_VALGRIND
   "whether to enable valgrind support"
   OFF)
set(POLAR_VALGRIND_DIR "" CACHE string "the valgrind directory")

option(POLAR_ENABLE_GCOV
   "Enable GCOV code coverage (requires LTP) - FOR DEVELOPERS ONLY!!"
   OFF)

option(POLAR_DISABLE_INLINE_OPTIMIZATION
   "If building zend_execute.lo fails, try this switch"
   OFF)

set(POLAR_CONFIG_FILE_PATH "Default" CACHE string "Set the path in which to look for php.ini")
set(POLAR_CONFIG_FILE_SCAN_DIR "Default" CACHE string "Set the path where to scan for configuration files")

option(POLAR_ENABLE_SIGCHILD
   "whether to enable PHP's own SIGCHLD handler"
   OFF)

option(POLAR_ENABLE_DMALLOC
   "whether to enable dmalloc"
   OFF)

option(POLAR_ENABLE_IPV6
   "whether to enable IPv6 support"
   OFF)

##
## DTRACE CHECKS
##
option(POLAR_ENABLE_DTRACE
   "whether to enable DTrace support"
   OFF)

option(POLAR_ENABLE_FD_SETSIZE
   "how big to make fd sets"
   OFF)

option(POLAR_DISABLE_ZEND_SIGNALS
   "whether to enable zend signal handling"
   OFF)

set(POLAR_FD_SETSIZE 7168 CACHE STRING "how big to make fd sets")

option(POLAR_TSRM_USE_PTH
   "Use GNU Pth"
   OFF)

option(POLAR_TSRM_USE_PTHREADS
   "Use pthread implementation"
   ON)

option(POLAR_DISABLE_GCC_GLOABL_REGS
   "whether to enable GCC global register variables"
   OFF)

option(POLAR_USE_FOLDERS "Enable solution folders in Visual Studio. Disable for Express versions." ON)
if (POLAR_USE_FOLDERS)
   set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

option(POLAR_ENABLE_BACKTRACES "Enable embedding backtraces on crash." ON)
if(POLAR_ENABLE_BACKTRACES)
   set(ENABLE_BACKTRACES 1)
endif()

set(POLAR_WITH_TSRM_PTH_CONFIG "pth-config" CACHE FILEPATH "Use GNU Pth")

set(POLAR_LIBDIR_SUFFIX "" CACHE STRING "Define suffix of library directory name (32/64)" )

set(POLAR_TOOLS_INSTALL_DIR "bin" CACHE STRING "Path for binary subdirectory (defaults to 'bin')")
mark_as_advanced(POLAR_TOOLS_INSTALL_DIR)

option(POLAR_ENABLE_FFI "Use libffi to call external functions from the interpreter" OFF)
set(FFI_LIBRARY_DIR "" CACHE PATH "Additional directory, where CMake should search for libffi.so")
set(FFI_INCLUDE_DIR "" CACHE PATH "Additional directory, where CMake should search for ffi.h or ffi/ffi.h")

option(POLAR_ENABLE_TERMINFO "Use terminfo database if available." ON)

option(POLAR_ENABLE_PIC "Build Position-Independent Code" ON)
option(POLAR_ENABLE_WARNINGS "Enable compiler warnings." ON)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   option(POLAR_ENABLE_MODULE_DEBUGGING "Compile with -gmodules." ON)
   option(POLAR_ENABLE_LOCAL_SUBMODULE_VISIBILITY "Compile with -fmodules-local-submodule-visibility." OFF)
else()
   option(POLAR_ENABLE_MODULE_DEBUGGING "Compile with -gmodules." OFF)
   option(POLAR_ENABLE_LOCAL_SUBMODULE_VISIBILITY "Compile with -fmodules-local-submodule-visibility." ON)
endif()

option(POLAR_ENABLE_EXPENSIVE_CHECKS "Enable expensive checks" OFF)
option(POLAR_ENABLE_LLD "Use lld as C and C++ linker." OFF)
option(POLAR_ENABLE_PEDANTIC "Compile with pedantic enabled." ON)
option(POLAR_ENABLE_WERROR "Fail and stop if a warning is triggered." OFF)

if(NOT POLAR_BUILD_TYPE STREQUAL "debug" )
   option(POLAR_ENABLE_ASSERTIONS "Enable assertions" OFF)
else()
   option(POLAR_ENABLE_ASSERTIONS "Enable assertions" ON)
endif()

set(POLAR_ABI_BREAKING_CHECKS "WITH_ASSERTS" CACHE STRING
   "Enable abi-breaking checks.  Can be WITH_ASSERTS, FORCE_ON or FORCE_OFF.")

option(POLAR_ENABLE_CRASH_OVERRIDES "Enable crash overrides." ON)
if(POLAR_ENABLE_CRASH_OVERRIDES)
   set(ENABLE_CRASH_OVERRIDES 1)
endif()

option(POLAR_EXTERNALIZE_DEBUGINFO
   "Generate dSYM files and strip executables and libraries (Darwin Only)" OFF)

option(POLAR_BUILD_TESTS
   "Build polarphp unit tests. If OFF, just generate build targets." OFF)
option(POLAR_INCLUDE_TESTS "Generate build targets for the polarphp unit tests." ON)

option(POLAR_ENABLE_TERMINFO "Use terminfo database if available." ON)
option(POLAR_ENABLE_ZLIB "Use zlib for compression/decompression if available." ON)
# Override the default target with an environment variable named by POLAR_TARGET_TRIPLE_ENV.
set(POLAR_TARGET_TRIPLE_ENV CACHE STRING "The name of environment variable to override default target. Disabled by blank.")
mark_as_advanced(POLAR_TARGET_TRIPLE_ENV)

# They are used as destination of target generators.
set(POLAR_RUNTIME_OUTPUT_INTDIR ${POLAR_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin)
set(POLAR_LIBRARY_OUTPUT_INTDIR ${POLAR_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib${POLAR_LIBDIR_SUFFIX})
get_filename_component(POLAR_RUNTIME_OUTPUT_INTDIR ${POLAR_RUNTIME_OUTPUT_INTDIR} ABSOLUTE)
get_filename_component(POLAR_LIBRARY_OUTPUT_INTDIR ${POLAR_LIBRARY_OUTPUT_INTDIR} ABSOLUTE)
set(POLAR_TOOLS_BINARY_DIR ${POLAR_RUNTIME_OUTPUT_INTDIR}) # --bindir
set(POLAR_LIBRARY_DIR      ${POLAR_LIBRARY_OUTPUT_INTDIR}) # --libdir

if (POLAR_BUILD_STATIC)
   set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
endif()

# add third party library install dir into find paths
set(CMAKE_FIND_ROOT_PATH "${POLAR_DEPS_INSTALL_DIR} ${CMAKE_FIND_ROOT_PATH}")

if(WIN32 OR CYGWIN)
   # DLL platform -- put DLLs into bin.
   set(POLAR_SHLIB_OUTPUT_INTDIR ${POLAR_RUNTIME_OUTPUT_INTDIR})
else()
   set(POLAR_SHLIB_OUTPUT_INTDIR ${POLAR_LIBRARY_OUTPUT_INTDIR})
endif()

file(TO_NATIVE_PATH "/" DIR_SEPARATOR)

# Sanity check our source directory to make sure that we are not trying to
# generate an in-source build (unless on MSVC_IDE, where it is ok), and to make
# sure that we don't have any stray generated files lying around in the tree
# (which would end up getting picked up by header search, instead of the correct
# versions).
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR AND NOT MSVC_IDE)
   message(FATAL_ERROR "In-source builds are not allowed.
      Please create a directory and run cmake from there, passing the path
      to this source directory as the last argument.
      This process created the file `CMakeCache.txt' and the directory `CMakeFiles'.
      Please delete them.")
endif()

set(POLAR_COMPILE_DEFINITIONS "")
set(POLAR_COMPILE_FLAGS "")
set(POLAR_COMMON_DEPENDS "")

# setup php version info
polar_setup_php_version()

message("build polarphp version: ${POLAR_PACKAGE_VERSION}")
message("build polarphp version id: ${POLAR_VERSION_ID}")

set(POLAR_PACKAGE_NAME polarphp)
set(POLAR_PACKAGE_STRING "${POLAR_PACKAGE_NAME} ${POLAR_PACKAGE_VERSION}")
set(POLAR_BUG_REPORT_URL "http://polarphp.org/bugs/")
set(PACKAGE_NAME ${POLAR_PACKAGE_NAME})
set(PACKAGE_STRING ${POLAR_PACKAGE_STRING})
set(PACKAGE_BUGREPORT ${POLAR_BUG_REPORT_URL})
set(PACKAGE_VERSION ${POLAR_PACKAGE_VERSION})

# add /usr/local/bin to PATH Env var
set(ENV{PATH} "/usr/local/bin:$ENV{PATH}")

if (NOT EXISTS POLAR_CONFIGURE_TEMP_DIR)
   file(MAKE_DIRECTORY ${POLAR_CONFIGURE_TEMP_DIR})
endif()

include(CheckCompilerVersion)
include(DetermineGCCCompatible)
include(DetectHostEnvironment)

# By default, we target the host, but this can be overridden at CMake
# invocation time.
set(POLAR_DEFAULT_TARGET_TRIPLE "${POLAR_HOST_TRIPLE}" CACHE STRING
  "Default target for which LLVM will generate code." )
set(TARGET_TRIPLE "${POLAR_DEFAULT_TARGET_TRIPLE}")
message(STATUS "polarphp host triple: ${POLAR_HOST_TRIPLE}")
message(STATUS "polarphp default target triple: ${POLAR_DEFAULT_TARGET_TRIPLE}")
include(HandlePolarOptions)
include(ProcesseRequirePackages)

# Configuring Zend and TSRM.
include(CheckTsrm)
include(CheckZend)

string(TIMESTAMP _buildDate "%Y-%m-%d %H:%M")

if (POLAR_BUILD_TYPE STREQUAL "debug")
   #   if (NOT PHP_BUILD_DATE)
   #      set(PHP_BUILD_DATE "${_buildDate}" CACHE STRING "polarphp build date")
   #      mark_as_advanced(PHP_BUILD_DATE)
   #   endif()
   # the birthday of PHP language
   set(PHP_BUILD_DATE "1995/06/08")
else()
   set(PHP_BUILD_DATE ${_buildDate})
endif()

set(PHP_OS ${POLAR_SYSTEM_NAME})
set(POLAR_OS ${POLAR_SYSTEM_NAME})

# setup default build rpath
set(CMAKE_BUILD_RPATH "${POLAR_DEPS_INSTALL_DIR}/lib")

include_directories(${POLAR_SOURCE_DIR}
   ${POLAR_MAIN_INCLUDE_DIR}
   ${POLAR_MAIN_INCLUDE_DIR}/polarphp
   ${POLAR_MAIN_INCLUDE_DIR}/polarphp/global
   ${POLAR_MAIN_INCLUDE_DIR}/polarphp/zend
   ${POLAR_MAIN_INCLUDE_DIR}/polarphp/tsrm
   ${POLAR_SOURCE_DIR}/src/Zend
   ${POLAR_SOURCE_DIR}/src/TSRM
   ${POLAR_INCLUDE_DIR}
   ${POLAR_INCLUDE_DIR}/polarphp
   ${POLAR_INCLUDE_DIR}/polarphp/global
   ${POLAR_INCLUDE_DIR}/polarphp/vm/zend
   ${POLAR_INCLUDE_DIR}/polarphp/vm/tsrm)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(ImportThirdparty)

add_subdirectory(include)
add_subdirectory(artifacts)
add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(thirdparty)

if (POLAR_INCLUDE_TESTS)
   polar_check_headers(glob)
   include(LitUtils)
   #setup some important lit path
   set(POLAR_LIT_RUNTIME_DIR ${POLAR_BINARY_DIR}/litruntime)
   set(POLAR_LIT_TEST_BIN_DIR ${POLAR_LIT_RUNTIME_DIR}/bin)
   set(POLAR_LIT_TEST_LIB_DIR ${POLAR_LIT_RUNTIME_DIR}/lib)
   add_subdirectory(devtools)
   add_subdirectory(unittests)
   add_subdirectory(tests)
endif()

polar_compile_env_summary_output()

