include_directories(${CMAKE_SOURCE_DIR}/src/libsoxr ${CMAKE_SOURCE_DIR}/src/)

file(GLOB FILE_LIST *.c *.cpp *.h)

add_library(libsoxr STATIC ${FILE_LIST})
