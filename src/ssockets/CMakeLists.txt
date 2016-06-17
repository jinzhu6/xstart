include_directories(${CMAKE_SOURCE_DIR}/src/ssockets)

file(GLOB FILE_LIST_CPP *.c*)
file(GLOB FILE_LIST_H *.h)

add_library(ssockets STATIC ${FILE_LIST_CPP} ${FILE_LIST_H})
