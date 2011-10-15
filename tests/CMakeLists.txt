project(Tests)

# Create the list of source files
aux_source_directory( . source_files )

list(APPEND source_files ${CMAKE_SOURCE_DIR}/reporting/CrashRpt/Utility.cpp)

# Define _UNICODE (use wide-char encoding)
add_definitions(-D_UNICODE )

# Add include dir
include_directories( ${CMAKE_SOURCE_DIR}/include 
                            ${CMAKE_SOURCE_DIR}/reporting/CrashRpt)

# Add executable build target
add_executable(Tests ${source_files})

# Add input link libraries
target_link_libraries(Tests CrashRpt CrashRptProbe)


