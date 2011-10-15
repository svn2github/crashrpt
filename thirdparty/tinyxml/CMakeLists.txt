project(tinyxml)

# Create the list of source files
aux_source_directory( . source_files )

# Define _UNICODE (use wide-char encoding)
add_definitions(-D_UNICODE)

# Add library build target
add_library(tinyxml ${source_files})

