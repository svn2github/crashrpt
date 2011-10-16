project(CrashRpt)

# Create the list of source files
aux_source_directory( . source_files )

# Enable usage of precompiled header
set(srcs_using_precomp ${source_files})
list(REMOVE_ITEM srcs_using_precomp ./StdAfx.cpp)
add_msvc_precompiled_header(stdafx.h ./StdAfx.cpp srcs_using_precomp)


# Define _UNICODE (use wide-char encoding)
add_definitions(-D_UNICODE -D_CRT_SECURE_NO_DEPRECATE)

# Add include dir
include_directories( ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/thirdparty/wtl)

# Add library build target
add_library(CrashRpt ${source_files})

target_link_libraries(CrashRpt rpcrt4.dll version.lib)

set_target_properties(CrashRpt PROPERTIES DEBUG_POSTFIX d )

