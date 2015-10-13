#!/bin/sh
if [ -z "$HHVM_BIN" ]; then
    if [ -z "$HPHP_HOME" ]; then
       echo 1>&2 "\$HPHP_HOME environment variable must be set!"
       exit 1
    fi
    export HHVM_BIN="${HPHP_HOME}/hphp/hhvm/hhvm"
fi

DIRNAME=`dirname $0`
REALPATH=`which realpath`
if [ ! -z "${REALPATH}" ]; then
  DIRNAME=`realpath ${DIRNAME}`
fi
$HHVM_BIN ./testtools/test/run \
   -a "-vDynamicExtensions.0=${DIRNAME}/uv.so -vEval.Jit=true" \
   ${DIRNAME}/tests/*/test_*.php

