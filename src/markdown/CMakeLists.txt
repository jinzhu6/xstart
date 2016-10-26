include_directories(${CMAKE_SOURCE_DIR}/src/markdown)

set(CMAKE_CXX_FLAGS "-fpermissive")

file(GLOB FILE_LIST *.c *.h)

add_library(markdown STATIC ${FILE_LIST})
