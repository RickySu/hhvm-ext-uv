#! /bin/sh

if [ -z "$HPHP_HOME" ]; then
    echo HPHP_HOME environment variable must be set!
    exit 1
fi


printf "<?hh\n" > ext_uv.php
tail -q -n +2 src/*php src/types/*php src/exceptions/*php >> ext_uv.php



CMAKE_MODULE_PATH="./CMake" $HPHP_HOME/hphp/tools/hphpize/hphpize

cmake .
make

