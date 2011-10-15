project(CrashRptProbe)

# Create the list of source files
aux_source_directory( . source_files )

list(APPEND source_files ${CMAKE_SOURCE_DIR}/reporting/crashrpt/Utility.cpp
			${CMAKE_SOURCE_DIR}/reporting/crashsender/md5.cpp)

# Define _UNICODE (use wide-char encoding)
add_definitions(-D_UNICODE)

# Add include dir
include_directories( ${CMAKE_SOURCE_DIR}/include 
			${CMAKE_SOURCE_DIR}/reporting/crashrpt
			${CMAKE_SOURCE_DIR}/reporting/crashsender
			${CMAKE_SOURCE_DIR}/thirdparty/zlib
			${CMAKE_SOURCE_DIR}/thirdparty/minizip
			${CMAKE_SOURCE_DIR}/thirdparty/tinyxml
			${CMAKE_SOURCE_DIR}/thirdparty/dbghelp/include)

# Add library build target
add_library(CrashRptProbe ${source_files})

target_link_libraries(CrashRptProbe zlib minizip tinyxml ${CMAKE_SOURCE_DIR}/thirdparty/dbghelp/lib/dbghelp.lib Rpcrt4.lib shell32.lib gdi32.lib version.lib psapi.lib)


