# It worth requiring an older version here, if anyone has problems with
# 2.8.  But that means finding somebody to test against those versions...
cmake_minimum_required(VERSION 2.8)

# Declare our project name and version.
project(pstsdk)
set(pstsdk_VERSION_MAJOR 0)
set(pstsdk_VERSION_MINOR 2)
set(pstsdk_VERSION_PATCH 0)

# Turn on testing.
enable_testing()

# Make our pstsdk/ subdirectory available for headers.
include_directories("${PROJECT_SOURCE_DIR}")

# Set up our compiler flags.
if(MSVC)
  set(CMAKE_CXX_FLAGS "/DPSTSDK_VALIDATION_LEVEL_FULL /EHsc /nologo /W4 /WX")
elseif(CMAKE_COMPILER_IS_GNUCC)
  set(CMAKE_CXX_FLAGS "-Wall -Werror -g -DPSTSDK_VALIDATION_LEVEL_FULL -std=c++0x")
endif()

# Make sure we have Boost.
include(FindBoost)
find_package(Boost 1.42.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# Use iconv if we have it.  This is required on non-Win32 platforms,
# because we don't necessarily know the encoding of wchar_t.
find_library(ICONV_LIBRARY NAMES iconv)

# Compile our unit tests.
add_subdirectory(test)

# Install our headers.  There may be a more elegant way to do this.
file(GLOB pstsdk_util_headers pstsdk/util/*.h)
install(FILES ${pstsdk_util_headers} DESTINATION include/pstsdk/util)
file(GLOB pstsdk_disk_headers pstsdk/disk/*.h)
install(FILES ${pstsdk_disk_headers} DESTINATION include/pstsdk/disk)
file(GLOB pstsdk_ndb_headers pstsdk/ndb/*.h)
install(FILES ${pstsdk_ndb_headers} DESTINATION include/pstsdk/ndb)
file(GLOB pstsdk_ltp_headers pstsdk/ltp/*.h)
install(FILES ${pstsdk_ltp_headers} DESTINATION include/pstsdk/ltp)
file(GLOB pstsdk_pst_headers pstsdk/pst/*.h)
install(FILES ${pstsdk_pst_headers} DESTINATION include/pstsdk/pst)
file(GLOB pstsdk_toplevel_headers pstsdk/*.h)
install(FILES ${pstsdk_toplevel_headers} DESTINATION include/pstsdk)
