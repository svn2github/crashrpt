project (CrashRpt)

# Preserve backwards compatibility
cmake_minimum_required (VERSION 2.8)

# CrashRpt version number
set (CRASHRPT_VER 1300)

# Set output directory for executable files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

# Set output directory for DLL files
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

# Set output directory for LIB files
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/lib)

# Precompiled headers stuff (CMake doesn't have a standard command for enabling precompiled headers,
# so we have to use a macro)
MACRO(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource SourcesVar)
  IF(MSVC)	
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
    SET(Sources ${${SourcesVar}})

    SET_SOURCE_FILES_PROPERTIES(${PrecompiledSource}
                                PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_OUTPUTS "${PrecompiledBinary}")
    SET_SOURCE_FILES_PROPERTIES(${Sources}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledBinary}\" /FI\"${PrecompiledBinary}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_DEPENDS "${PrecompiledBinary}")  
    # Add precompiled header to SourcesVar
    LIST(APPEND ${SourcesVar} ${PrecompiledSource})
  ENDIF(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)

# Other CMakeLists are located in project subdirectories 

add_subdirectory("thirdparty/jpeg")
add_subdirectory("thirdparty/libpng")
add_subdirectory("thirdparty/minizip")
add_subdirectory("thirdparty/zlib")
add_subdirectory("thirdparty/tinyxml")

add_subdirectory("reporting/crashcon")
add_subdirectory("reporting/crashrpt")
add_subdirectory("reporting/crashrpttest")
add_subdirectory("reporting/crashsender")

add_subdirectory("processing/crashrptprobe")
add_subdirectory("processing/crprober")

add_subdirectory("tests")
