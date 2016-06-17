include_directories(${CMAKE_SOURCE_DIR}/src/opencv2 ${CMAKE_SOURCE_DIR}/src/zlib)

file(GLOB_RECURSE FILE_LIST_CPP *.cpp)
file(GLOB_RECURSE FILE_LIST_C *.c)
file(GLOB_RECURSE FILE_LIST_H *.h)

add_library(opencv2 ${FILE_LIST_CPP} ${FILE_LIST_C} ${FILE_LIST_H})
