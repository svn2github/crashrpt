project(CrashRpt)

# Create the list of source files
aux_source_directory( . source_files )

list(APPEND source_files ./CrashRpt.def)

# Enable usage of precompiled header
set(srcs_using_precomp ${source_files})
list(REMOVE_ITEM srcs_using_precomp ./StdAfx.cpp ./CrashRpt.def)
add_msvc_precompiled_header(stdafx.h ./StdAfx.cpp srcs_using_precomp)

# Define _UNICODE (use wide-char encoding)
add_definitions(-D_UNICODE -D_CRT_SECURE_NO_DEPRECATE)

fix_default_compiler_settings_()

# Add include dir
include_directories( ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/thirdparty/wtl)

# Add library build target
if(CRASHRPT_BUILD_SHARED_LIBS)
	add_library(CrashRpt SHARED ${source_files})
else(CRASHRPT_BUILD_SHARED_LIBS)
	add_library(CrashRpt STATIC ${source_files})
endif(CRASHRPT_BUILD_SHARED_LIBS)

target_link_libraries(CrashRpt rpcrt4.dll version.lib)

if(CRASHRPT_BUILD_SHARED_LIBS)
	set_target_properties(CrashRpt PROPERTIES DEBUG_POSTFIX ${CRASHRPT_VER}d )
	set_target_properties(CrashRpt PROPERTIES RELEASE_POSTFIX ${CRASHRPT_VER} )
else(CRASHRPT_BUILD_SHARED_LIBS)
	set_target_properties(CrashRpt PROPERTIES DEBUG_POSTFIX LIBd )
	set_target_properties(CrashRpt PROPERTIES RELEASE_POSTFIX LIB )
endif(CRASHRPT_BUILD_SHARED_LIBS)

