include(ExternalProject)

set(LIBUV_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libuv)

ExternalProject_Add(libuv
    GIT_REPOSITORY "https://github.com/joyent/libuv.git"
    GIT_TAG "v0.11.25"
    SOURCE_DIR ${LIBUV_SOURCE}
    PATCH_COMMAND ${LIBUV_SOURCE}/autogen.sh
    CONFIGURE_COMMAND CFLAGS=-fPIC ${LIBUV_SOURCE}/configure --prefix=${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/build
BUILD_COMMAND ${MAKE})

set(LIBUV_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/build/include)
set(LIBUV_LIB ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/build/lib)

mark_as_advanced(
    LIBUV_LIB
    LIBUV_INCLUDE_DIR
)
