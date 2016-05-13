include_directories(${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/src/xstart ${CMAKE_SOURCE_DIR}/src/corela ${CMAKE_SOURCE_DIR}/src/freetype ${CMAKE_SOURCE_DIR}/src/cairo ${CMAKE_SOURCE_DIR}/src/opencv2 ${CMAKE_SOURCE_DIR}/src/libnova ${CMAKE_SOURCE_DIR}/src/midifile ${CMAKE_SOURCE_DIR}/src/mongoose)

file(GLOB FILE_LIST_CPP *.cpp)
file(GLOB FILE_LIST_C *.c)
file(GLOB FILE_LIST_H *.h)
file(GLOB FILE_LIST_RC *.rc)

#if(${CMAKE_GENERATOR} MATCHES "Visual Studio")
#    file(GLOB FILE_LIST_RC *.rc)
#endif()

add_executable(xstart icon.ico ${FILE_LIST_CPP} ${FILE_LIST_C} ${FILE_LIST_H} ${FILE_LIST_RC})

#find_package(OpenGL)
#include_directories(${OPENGL_INCLUDE_DIRS})
#target_link_libraries(xstart gmscript expat freetype pixman cairo zlib ssockets opencv2 corela ogg vorbis astro soundy ${OPENGL_LIBRARIES})

target_link_libraries(xstart gmscript expat freetype cairo zlib ssockets opencv2 pixman corela midifile portaudio libnodave soundtouch libsoxr mongoose)

if(WIN32)
    target_link_libraries(xstart wsock32 ws2_32 winmm dsound dsbase libnova ogg vorbis soundy)
else()
    target_link_libraries(xstart pthread Xrandr Xext Xxf86vm Xext X11 GL rt)
    target_link_libraries(xstart gstinterfaces-0.10 gstreamer-0.10 gobject-2.0 gmodule-2.0 gthread-2.0 xml2 glib-2.0)
endif(WIN32)
