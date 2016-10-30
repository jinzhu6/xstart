include_directories(${CMAKE_SOURCE_DIR}/src/curl ${CMAKE_SOURCE_DIR}/src/zlib)

file(GLOB FILE_LIST *.c *.cpp *.h)

add_library(curl STATIC ${FILE_LIST})
