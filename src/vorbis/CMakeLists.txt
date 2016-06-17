include_directories(${CMAKE_SOURCE_DIR}/src/ogg ${CMAKE_SOURCE_DIR}/src/vorbis)

file(GLOB FILE_LIST_C *.c)
file(GLOB FILE_LIST_H *.h)

add_library(vorbis STATIC ${FILE_LIST_C} ${FILE_LIST_H})
