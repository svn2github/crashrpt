project (CrashRpt)

cmake_minimum_required (VERSION 2.8)

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
