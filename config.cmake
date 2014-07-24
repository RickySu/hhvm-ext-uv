set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
include(buildLibuv)
include(buildr3)

CONFIGURE_FILE(${CMAKE_CURRENT_BINARY_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include_directories(${LIBUV_INCLUDE_DIR})
set(LIBUV_LIBRARIES ${LIBUV_LIB}/libuv.a ${R3_LIB}/libr3.a)

HHVM_EXTENSION(uv
    src/resource/InternalResourceData.cpp
    src/resource/CallbackResourceData.cpp
    src/resource/TcpResourceData.cpp
    src/resource/UdpResourceData.cpp
    src/resource/ResolverResourceData.cpp
    src/ext.cpp
    src/uv_util.cpp
    src/uv_loop.cpp
    src/uv_signal.cpp
    src/uv_tcp.cpp
    src/uv_udp.cpp
    src/uv_resolver.cpp
    src/uv_timer.cpp
)
HHVM_SYSTEMLIB(uv ext_uv.php)

target_link_libraries(uv ${LIBUV_LIBRARIES})
