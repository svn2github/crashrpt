project(CrashSender)

# Create the list of source files
aux_source_directory( . source_files )

# Enable usage of precompiled header
set(srcs_using_precomp ${source_files})
list(REMOVE_ITEM srcs_using_precomp ./stdafx.cpp ./md5.cpp ./base64.cpp)
add_msvc_precompiled_header(stdafx.h ./stdafx.cpp srcs_using_precomp)

list(APPEND source_files 
	./CrashSender.rc 
	${CMAKE_SOURCE_DIR}/reporting/CrashRpt/Utility.cpp 
	${CMAKE_SOURCE_DIR}/reporting/CrashRpt/SharedMem.cpp)

# Define _UNICODE (use wide-char encoding)
add_definitions(-D_UNICODE )

# Add include dir
include_directories( ${CMAKE_SOURCE_DIR}/include 
                            ${CMAKE_SOURCE_DIR}/reporting/CrashRpt
                            ${CMAKE_SOURCE_DIR}/thirdparty/wtl 
                            ${CMAKE_SOURCE_DIR}/thirdparty/zlib
                            ${CMAKE_SOURCE_DIR}/thirdparty/minizip  
                            ${CMAKE_SOURCE_DIR}/thirdparty/jpeg 
                            ${CMAKE_SOURCE_DIR}/thirdparty/libpng 
                            ${CMAKE_SOURCE_DIR}/thirdparty/tinyxml
                            ${CMAKE_SOURCE_DIR}/thirdparty/dbghelp/include )

# Add executable build target
add_executable(CrashSender WIN32 ${source_files})

# Add input link libraries
target_link_libraries(CrashSender zlib minizip libjpeg libpng tinyxml WS2_32.lib Dnsapi.lib wininet.lib Rpcrt4.lib Gdi32.lib shell32.lib Comdlg32.lib version.lib psapi.lib)


set_target_properties(CrashSender PROPERTIES 
				DEBUG_POSTFIX ${CRASHRPT_VER}d
				RELEASE_POSTFIX ${CRASHRPT_VER} )