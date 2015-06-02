include(ExternalProject)

set(R3_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/r3)
set(R3_PREFIX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/build)
set(R3_INCLUDE_DIR ${R3_PREFIX_DIR}/include)
set(R3_LIB ${R3_PREFIX_DIR}/lib)

set(CLEAN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/R3-prefix\;${CLEAN_FILES}")

ExternalProject_Add(R3
    SOURCE_DIR ${R3_SOURCE}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
)

ExternalProject_Add_Step(R3 AUTOGEN
    COMMAND "./autogen.sh"
    WORKING_DIRECTORY ${R3_SOURCE}
    COMMENT "r3 autogen..."
)

ExternalProject_Add_Step(R3 CONFIGURE
    COMMAND env CFLAGS=-fPIC\ -O2 ./configure --prefix=${R3_PREFIX_DIR}
    WORKING_DIRECTORY ${R3_SOURCE}
    DEPENDEES AUTOGEN
    COMMENT "r3 configure..."
)

ExternalProject_Add_Step(R3 MAKE_CLEAN
    COMMAND make clean
    WORKING_DIRECTORY ${R3_SOURCE}
    DEPENDEES CONFIGURE
    COMMENT "r3 make clean..."
)

ExternalProject_Add_Step(R3 MAKE_ALL
    COMMAND make
    WORKING_DIRECTORY ${R3_SOURCE}
    DEPENDEES MAKE_CLEAN
    COMMENT "r3 make all..."
)

ExternalProject_Add_Step(R3 MAKE_INSTALL
    COMMAND make install
    WORKING_DIRECTORY ${R3_SOURCE}
    DEPENDEES MAKE_ALL
#    ALWAYS 1
    COMMENT "r3 make install..."
)

mark_as_advanced(
    R3_LIB
    R3_INCLUDE_DIR
)
