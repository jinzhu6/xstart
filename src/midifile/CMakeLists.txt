include_directories(${CMAKE_SOURCE_DIR}/src/midifile ${CMAKE_SOURCE_DIR}/src/)

file(GLOB FILE_LIST *.c *.cpp *.h)

add_library(midifile STATIC ${FILE_LIST})
