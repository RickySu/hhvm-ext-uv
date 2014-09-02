#! /bin/sh
if [ -z "$HPHPIZE_PATH" ]; then
    if [ -z "$HPHP_HOME" ]; then
        echo HPHP_HOME environment variable must be set!
        exit 1
    fi
    HPHPIZE_PATH=$HPHP_HOME/hphp/tools/hphpize/hphpize    
fi

if [ -z "$HHVM_BIN" ]; then
    if [ -z "$HPHP_HOME" ]; then
        HHVM_BIN=env hhvm
    else
        HHVM_BIN=$HPHP_HOME/hphp/hhvm/hhvm
    fi
fi

tail -q -n +2 src/*.php src/types/*.php src/exceptions/*.php | $HHVM_BIN tools/minify.php > ext_uv.php

CMAKE_MODULE_PATH="./CMake" $HPHPIZE_PATH

cmake .
make

