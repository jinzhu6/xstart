include_directories(${CMAKE_SOURCE_DIR}/src/dsbase)

file(GLOB FILE_LIST *.c *.cpp *.h)

add_library(dsbase STATIC ${FILE_LIST})
