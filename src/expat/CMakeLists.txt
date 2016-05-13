include_directories(${CMAKE_SOURCE_DIR}/src/expat)

file(GLOB FILE_LIST *.c)

add_library(expat STATIC ${FILE_LIST})

