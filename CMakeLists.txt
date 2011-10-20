project (CrashRpt)

# Preserve backwards compatibility
cmake_minimum_required (VERSION 2.8)

# CrashRpt version number
set (CRASHRPT_VER 1300)

# Build configurations
if(CMAKE_CONFIGURATION_TYPES)
   set(CMAKE_CONFIGURATION_TYPES Debug Release)
   set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
     "Reset the configurations to what we need"
     FORCE)
endif()

# Build options
option(CRASHRPT_BUILD_SHARED_LIBS "If set (default), CrashRpt modules are built as dynamic-link libraries, otherwise as static libs." ON)
option(CRASHRPT_LINK_CRT_AS_DLL "If set (default), CrashRpt modules link C run-time (CRT) as multi-threaded dynamic libraries, otherwise as multi-threaded static libs." ON)

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

# Tweaks CMake's default compiler/linker settings.
# The macro is taken from Google Test code.
macro(fix_default_compiler_settings_)
  if (MSVC)
    # For MSVC, CMake sets certain flags to defaults we want to override.
    # This replacement code is taken from sample in the CMake Wiki at
    # http://www.cmake.org/Wiki/CMake_FAQ#Dynamic_Replace.
    foreach (flag_var
             CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
             CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
      if (NOT CRASHRPT_LINK_CRT_AS_DLL)        
        string(REPLACE "/MD" "-MT" ${flag_var} "${${flag_var}}")
      endif(NOT CRASHRPT_LINK_CRT_AS_DLL)        

      # We prefer more strict warning checking.
      # Replaces /W3 with /W4 in defaults.
      string(REPLACE "/W3" "-W4" ${flag_var} "${${flag_var}}")
    endforeach()
  endif()
endmacro()

# Other CMakeLists are located in project subdirectories 

add_subdirectory("reporting/crashcon")
add_subdirectory("reporting/crashrpt")
add_subdirectory("reporting/crashrpttest")
add_subdirectory("reporting/crashsender")

add_subdirectory("processing/crashrptprobe")
add_subdirectory("processing/crprober")

add_subdirectory("tests")

# Set output directory for LIB files
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/thirdparty/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}thirdparty/lib)

add_subdirectory("thirdparty/jpeg")
add_subdirectory("thirdparty/libpng")
add_subdirectory("thirdparty/minizip")
add_subdirectory("thirdparty/zlib")
add_subdirectory("thirdparty/tinyxml")
