include(ExternalProject)
set(LIBUV_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libuv)
set(LIBUV_PREFIX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/build)
set(LIBUV_INCLUDE_DIR ${LIBUV_PREFIX_DIR}/include)
set(LIBUV_LIB ${LIBUV_PREFIX_DIR}/lib)

ExternalProject_Add(LIBUV
    SOURCE_DIR ${LIBUV_SOURCE}
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND cd ${LIBUV_SOURCE} && ./autogen.sh
    CONFIGURE_COMMAND cd ${LIBUV_SOURCE} && CFLAGS=-fPIC ./configure --prefix=${LIBUV_PREFIX_DIR}
    BUILD_COMMAND cd ${LIBUV_SOURCE} && make
    INSTALL_COMMAND cd ${LIBUV_SOURCE} && make install
)

mark_as_advanced(
    LIBUV_LIB
    LIBUV_INCLUDE_DIR
)
