include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/xstart ${CMAKE_SOURCE_DIR}/src/corela ${CMAKE_SOURCE_DIR}/src/freetype ${CMAKE_SOURCE_DIR}/src/cairo ${CMAKE_SOURCE_DIR}/src/opencv2 ${CMAKE_SOURCE_DIR}/src/libnova)

file(GLOB FILE_LIST_C *.c)
file(GLOB FILE_LIST_CPP *.cpp)
file(GLOB FILE_LIST_H *.h)

if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
    file(GLOB FILE_LIST_RC *.rc)
endif()

add_executable(glxtest ${FILE_LIST_C} ${FILE_LIST_CPP} ${FILE_LIST_H} ${FILE_LIST_RC})

find_package(OpenGL)
include_directories(${OPENGL_INCLUDE_DIRS})

#target_link_libraries(xstart gmscript expat freetype pixman cairo zlib ssockets opencv2 corela ogg vorbis astro soundy ${OPENGL_LIBRARIES})
target_link_libraries(glxtest gmscript expat freetype cairo zlib ssockets opencv2 pixman corela rt ${OPENGL_LIBRARIES})

if(WIN32)
    target_link_libraries(glxtest wsock32 ws2_32 winmm)
else()
    target_link_libraries(glxtest pthread Xrandr Xext Xxf86vm Xext X11)
endif(WIN32)
