include_directories(${CMAKE_SOURCE_DIR}/src/zlib)

file(GLOB FILE_LIST *.c)

add_library(zlib STATIC ${FILE_LIST})
