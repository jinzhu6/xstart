include_directories(${CMAKE_SOURCE_DIR}/src/pixman)

file(GLOB FILE_LIST *.c)

add_definitions(-D_USE_MATH_DEFINES -DPIXMAN_NO_TLS -DPACKAGE="x86-cairo" -D__SSE__ -D__MMX__)
add_library(pixman STATIC ${FILE_LIST})
