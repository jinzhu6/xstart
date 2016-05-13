include_directories(${CMAKE_SOURCE_DIR}/src/)

file(GLOB FILE_LIST_C *.c)
file(GLOB FILE_LIST_H *.h)

add_library(ogg STATIC ${FILE_LIST_C} ${FILE_LIST_H})
