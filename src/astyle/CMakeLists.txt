include_directories(${CMAKE_SOURCE_DIR}/src/astyle)

set(CMAKE_CXX_FLAGS "-fpermissive")

file(GLOB FILE_LIST *.cpp *.h)

add_library(astyle STATIC ${FILE_LIST})

