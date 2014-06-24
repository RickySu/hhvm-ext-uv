set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
include(buildLibuv)

CONFIGURE_FILE(${CMAKE_CURRENT_BINARY_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

include_directories(${LIBUV_INCLUDE_DIR})
set(LIBUV_LIBRARIES "${LIBUV_LIB}/libuv.a")

HHVM_EXTENSION(uv
    src/resource/InternalResourceData.cpp
    src/resource/CallbackResourceData.cpp
    src/resource/TcpResourceData.cpp
    src/ext.cpp
    src/uv_util.cpp
    src/uv_loop.cpp
    src/uv_signal.cpp
    src/uv_tcp.cpp
)
HHVM_SYSTEMLIB(uv ext_uv.php)

target_link_libraries(uv ${LIBUV_LIBRARIES})
