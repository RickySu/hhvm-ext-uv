include(ExternalProject)
set(LIBUV_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libuv)
set(LIBUV_PREFIX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/build)
set(LIBUV_INCLUDE_DIR ${LIBUV_PREFIX_DIR}/include)
set(LIBUV_LIB ${LIBUV_PREFIX_DIR}/lib)

set(CLEAN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/LIBUV-prefix\;${CLEAN_FILES}")

ExternalProject_Add(LIBUV
    SOURCE_DIR ${LIBUV_SOURCE}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

ExternalProject_Add_Step(LIBUV AUTOGEN
    COMMAND "./autogen.sh"
    WORKING_DIRECTORY ${LIBUV_SOURCE}
    COMMENT "libuv autogen..."
)

ExternalProject_Add_Step(LIBUV CONFIGURE
    COMMAND env CFLAGS=-fPIC\ -O2 ./configure --prefix=${LIBUV_PREFIX_DIR}
    WORKING_DIRECTORY ${LIBUV_SOURCE}
    DEPENDEES AUTOGEN
    COMMENT "libuv configure..."
)

ExternalProject_Add_Step(LIBUV MAKE_CLEAN
    COMMAND make clean
    WORKING_DIRECTORY ${LIBUV_SOURCE}
    DEPENDEES CONFIGURE
    COMMENT "libuv make clean..."
)

ExternalProject_Add_Step(LIBUV MAKE_ALL
    COMMAND make
    WORKING_DIRECTORY ${LIBUV_SOURCE}
    DEPENDEES MAKE_CLEAN
    COMMENT "libuv make all..."
)

ExternalProject_Add_Step(LIBUV MAKE_INSTALL
    COMMAND make install
    WORKING_DIRECTORY ${LIBUV_SOURCE}
    DEPENDEES MAKE_ALL
    ALWAYS 1
    COMMENT "libuv make install..."
)


mark_as_advanced(
    LIBUV_LIB
    LIBUV_INCLUDE_DIR
)

