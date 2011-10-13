project(zlib)

# Create the list of source files
aux_source_directory( . source_files )

# Define _UNICODE (use wide-char encoding)
add_definitions(-D_UNICODE -D_CRT_SECURE_NO_DEPRECATE)

# Add library build target
add_library(zlib ${source_files})

