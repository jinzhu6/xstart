include_directories(${CMAKE_SOURCE_DIR}/src/libnova ${CMAKE_SOURCE_DIR}/src/)

file(GLOB FILE_LIST *.c *.cpp *.h)

add_library(libnova STATIC ${FILE_LIST})
