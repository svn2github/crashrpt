project(crashcon)

# Create the list of source files
aux_source_directory( . source_files )

# Define _UNICODE (use wide-char encoding)
add_definitions(-D_UNICODE )

fix_default_compiler_settings_()

# Add include dir
include_directories(${CMAKE_SOURCE_DIR}/include 
					${CMAKE_SOURCE_DIR}/reporting/CrashRpt
					${CMAKE_SOURCE_DIR}/thirdparty/wtl 
					${CMAKE_SOURCE_DIR}/thirdparty/zlib
					${CMAKE_SOURCE_DIR}/thirdparty/minizip  
					${CMAKE_SOURCE_DIR}/thirdparty/jpeg 
					${CMAKE_SOURCE_DIR}/thirdparty/libpng 
					${CMAKE_SOURCE_DIR}/thirdparty/tinyxml )

# Add executable build target
add_executable(crashcon ${source_files})

# Add input link libraries
target_link_libraries(crashcon CrashRpt)

set_target_properties(crashcon PROPERTIES DEBUG_POSTFIX d )
