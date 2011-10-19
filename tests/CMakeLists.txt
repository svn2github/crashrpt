project(Tests)

# Create the list of source files
aux_source_directory( . source_files )

list(APPEND source_files ${CMAKE_SOURCE_DIR}/reporting/CrashRpt/Utility.cpp)

# Enable usage of precompiled header
set(srcs_using_precomp ${source_files})
list(REMOVE_ITEM srcs_using_precomp ./stdafx.cpp )
add_msvc_precompiled_header(stdafx.h ./stdafx.cpp srcs_using_precomp )

# Define _UNICODE (use wide-char encoding)
add_definitions(-D_UNICODE )

fix_default_compiler_settings_()

# Add include dir
include_directories( ${CMAKE_SOURCE_DIR}/include 
                     ${CMAKE_SOURCE_DIR}/reporting/CrashRpt
					 ${CMAKE_SOURCE_DIR}/thirdparty/wtl )

# Add executable build target
add_executable(Tests ${source_files})

# Add input link libraries
target_link_libraries(Tests CrashRpt CrashRptProbe)

set_target_properties(Tests PROPERTIES DEBUG_POSTFIX d )
