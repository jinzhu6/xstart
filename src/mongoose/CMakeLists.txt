include_directories(${CMAKE_SOURCE_DIR}/src/mongoose ${CMAKE_SOURCE_DIR}/src/)

file(GLOB FILE_LIST *.c *.cpp *.h)

add_library(mongoose STATIC ${FILE_LIST})
