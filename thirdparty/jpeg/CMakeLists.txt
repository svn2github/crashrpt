project(libjpeg)

# Create the list of source files
aux_source_directory( . source_files )

# Define _UNICODE (use wide-char encoding)
add_definitions(-D_UNICODE -D_CRT_SECURE_NO_DEPRECATE /wd4244)

# Add library build target
add_library(libjpeg ${source_files})

set_target_properties(libjpeg PROPERTIES DEBUG_POSTFIX d )