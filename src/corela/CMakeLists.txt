include_directories(${CMAKE_SOURCE_DIR}/src/freetype ${CMAKE_SOURCE_DIR}/src/dxsdk ${CMAKE_SOURCE_DIR}/src/dsbase)

file(GLOB FILE_LIST_CPP *.cpp)
file(GLOB FILE_LIST_C *.c)
file(GLOB FILE_LIST_H *.h)

add_library(corela ${FILE_LIST_CPP} ${FILE_LIST_C} ${FILE_LIST_H})

find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules (GLIB2   glib-2.0)

include_directories(${OPENGL_INCLUDE_DIRS} ${GLIB2_INCLUDE_DIRS})

target_link_libraries(corela gmscript expat freetype zlib ${OPENGL_LIBRARIES})

set(CMAKE_C_FLAGS "-I /usr/lib/x86_64-linux-gnu/glib-2.0/include/ -I/usr/include/gstreamer-0.10 -I/usr/include/glib-2.0 -I/usr/lib/arm-linux-gnueabihf/glib-2.0/include -I/usr/include/libxml2")
