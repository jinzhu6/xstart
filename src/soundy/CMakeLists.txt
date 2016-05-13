include_directories(${CMAKE_SOURCE_DIR}/src/)

file(GLOB FILE_LIST_CPP *.cpp)
file(GLOB FILE_LIST_C *.c)
file(GLOB FILE_LIST_H *.h)

add_library(soundy ${FILE_LIST_CPP} ${FILE_LIST_C} ${FILE_LIST_H})

#target_link_libraries(asound)

