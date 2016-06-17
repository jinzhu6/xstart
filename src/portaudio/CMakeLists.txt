cmake_minimum_required(VERSION 2.6)

SET(SOURCE_FILES common/pa_allocation.c common/pa_allocation.h common/pa_converters.c common/pa_converters.h common/pa_cpuload.c common/pa_cpuload.h common/pa_debugprint.c common/pa_debugprint.h common/pa_dither.c common/pa_dither.h common/pa_endianness.h common/pa_front.c common/pa_hostapi.h common/pa_memorybarrier.h common/pa_process.c common/pa_process.h common/pa_ringbuffer.c common/pa_ringbuffer.h common/pa_stream.c common/pa_stream.h common/pa_trace.c common/pa_trace.h common/pa_types.h common/pa_util.h hostapi/skeleton/pa_hostapi_skeleton.c)

if(UNIX)
	include_directories(${CMAKE_SOURCE_DIR}/src/portaudio ${CMAKE_SOURCE_DIR}/src/portaudio/common ${CMAKE_SOURCE_DIR}/src/portaudio/hostapi/alsa ${CMAKE_SOURCE_DIR}/src/portaudio/hostapi/skeleton ${CMAKE_SOURCE_DIR}/src/portaudio/os/unix)
	list(APPEND SOURCE_FILES hostapi/alsa/pa_linux_alsa.c os/unix/pa_unix_hostapis.c os/unix/pa_unix_util.c os/unix/pa_unix_util.h)
elseif(WIN32)
	include_directories(${CMAKE_SOURCE_DIR}/src/portaudio ${CMAKE_SOURCE_DIR}/src/portaudio/common ${CMAKE_SOURCE_DIR}/src/portaudio/hostapi/dsound ${CMAKE_SOURCE_DIR}/src/portaudio/hostapi/asio ${CMAKE_SOURCE_DIR}/src/asio ${CMAKE_SOURCE_DIR}/src/asio/host ${CMAKE_SOURCE_DIR}/src/asio/host/pc ${CMAKE_SOURCE_DIR}/src/asio/common ${CMAKE_SOURCE_DIR}/src/portaudio/hostapi/skeleton ${CMAKE_SOURCE_DIR}/src/portaudio/hostapi/wmme ${CMAKE_SOURCE_DIR}/src/portaudio/os/win)
	list(APPEND SOURCE_FILES hostapi/dsound/pa_win_ds.c hostapi/dsound/pa_win_ds_dynlink.c hostapi/dsound/pa_win_ds_dynlink.h hostapi/wmme/pa_win_wmme.c os/win/pa_win_coinitialize.c os/win/pa_win_coinitialize.h os/win/pa_win_hostapis.c os/win/pa_win_util.c os/win/pa_win_waveformat.c os/win/pa_x86_plain_converters.h hostapi/asio/pa_asio.cpp hostapi/asio/iasiothiscallresolver.cpp ../asio/common/asio.cpp ../asio/host/asiodrivers.cpp ../asio/host/pc/asiolist.cpp)
elseif(APPLE)
	include_directories(${CMAKE_SOURCE_DIR}/src/portaudio ${CMAKE_SOURCE_DIR}/src/portaudio/common ${CMAKE_SOURCE_DIR}/src/portaudio/hostapi/coreaudio ${CMAKE_SOURCE_DIR}/src/portaudio/hostapi/skeleton)
	list(APPEND SOURCE_FILES hostapi/coreaudio/pa_mac_core.c hostapi/coreaudio/pa_mac_core_blocking.c hostapi/coreaudio/pa_mac_core_blocking.h hostapi/coreaudio/pa_mac_core_internal.h hostapi/coreaudio/pa_mac_core_old.c hostapi/coreaudio/pa_mac_core_utilities.c hostapi/coreaudio/pa_mac_core_utilities.h)
endif()

add_library(portaudio STATIC ${SOURCE_FILES})

if(UNIX)
	target_link_libraries(portaudio asound)
	add_definitions(-DPA_USE_ALSA)
elseif(WIN32)
	target_link_libraries(portaudio user32 dsound)
	add_definitions(-DPA_USE_DS -DPA_USE_ASIO -D_CRT_SECURE_NO_WARNINGS)
elseif(APPLE)
	target_link_libraries(portaudio )
	add_definitions(-DPA_USE_COREAUDIO)
endif()

