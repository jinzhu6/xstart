include_directories(${CMAKE_SOURCE_DIR}/src/libnodave ${CMAKE_SOURCE_DIR}/src/)


if(WIN32)
    add_library(libnodave STATIC log2.h nodave.c nodave.h openS7online.c openS7online.h openSocketw.c openSocket.h setportw.c setport.h)
    add_definitions(-DBCCWIN -DDAVE_LITTLE_ENDIAN)
else()
    add_library(libnodave STATIC log2.h nodave.c nodave.h openS7online.c openS7online.h openSocket.c openSocket.h setport.c setport.h)
    add_definitions(-DLINUX -DDAVE_LITTLE_ENDIAN)
endif(WIN32)
