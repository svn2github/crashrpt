project(CrashRpt)

# Create the list of source files
aux_source_directory( . source_files )

# Define _UNICODE (use wide-char encoding)
add_definitions(-D_UNICODE -D_CRT_SECURE_NO_DEPRECATE)

# Add include dir
include_directories( ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/thirdparty/wtl)

# Add library build target
add_library(CrashRpt ${source_files})

target_link_libraries(CrashRpt rpcrt4.dll version.lib)


