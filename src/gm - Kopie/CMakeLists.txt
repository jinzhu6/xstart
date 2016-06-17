include_directories(${CMAKE_SOURCE_DIR}/src/gm)

set(CMAKE_CXX_FLAGS "-fpermissive")

file(GLOB FILE_LIST *.cpp *.h)

add_library(gmscript STATIC ${FILE_LIST})
