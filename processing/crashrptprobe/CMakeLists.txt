project(CrashRptProbe)

# Create the list of source files
aux_source_directory( . source_files )

list(APPEND source_files ./CrashRptProbe.def ${CMAKE_SOURCE_DIR}/reporting/crashrpt/Utility.cpp
			${CMAKE_SOURCE_DIR}/reporting/crashsender/md5.cpp)

# Enable usage of precompiled header
set(srcs_using_precomp ${source_files})
list(REMOVE_ITEM srcs_using_precomp  ./CrashRptProbe.def ./stdafx.cpp ${CMAKE_SOURCE_DIR}/reporting/crashsender/md5.cpp)
add_msvc_precompiled_header(stdafx.h ./stdafx.cpp srcs_using_precomp)

# Define _UNICODE (use wide-char encoding)
add_definitions(-D_UNICODE)

fix_default_compiler_settings_()

# Add include dir
include_directories( ${CMAKE_SOURCE_DIR}/include 
			${CMAKE_SOURCE_DIR}/reporting/crashrpt
			${CMAKE_SOURCE_DIR}/reporting/crashsender
			${CMAKE_SOURCE_DIR}/thirdparty/zlib
			${CMAKE_SOURCE_DIR}/thirdparty/minizip
			${CMAKE_SOURCE_DIR}/thirdparty/tinyxml
			${CMAKE_SOURCE_DIR}/thirdparty/dbghelp/include)

# Add library build target
if(CRASHRPT_USE_STATIC_LINKAGE)
	add_library(CrashRptProbe STATIC ${source_files})
else(CRASHRPT_USE_STATIC_LINKAGE)
	add_library(CrashRptProbe SHARED ${source_files})
endif(CRASHRPT_USE_STATIC_LINKAGE)

target_link_libraries(CrashRptProbe zlib minizip tinyxml ${CMAKE_SOURCE_DIR}/thirdparty/dbghelp/lib/dbghelp.lib Rpcrt4.lib shell32.lib gdi32.lib version.lib psapi.lib)

if(CRASHRPT_USE_STATIC_LINKAGE)

	set_target_properties(CrashRptProbe PROPERTIES DEBUG_POSTFIX LIBd )
	set_target_properties(CrashRptProbe PROPERTIES RELEASE_POSTFIX LIB )

else(CRASHRPT_USE_STATIC_LINKAGE)

	set_target_properties(CrashRptProbe PROPERTIES DEBUG_POSTFIX d )
	set_target_properties(CrashRptProbe PROPERTIES IMPORT_POSTFIX_DEBUG d )

endif(CRASHRPT_USE_STATIC_LINKAGE)


