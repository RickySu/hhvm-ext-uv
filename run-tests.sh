#!/bin/sh

if [ -z "$HPHP_HOME" ]; then
  echo 1>&2 "\$HPHP_HOME environment variable must be set!"
  exit 1
fi

DIRNAME=`dirname $0`
REALPATH=`which realpath`
if [ ! -z "${REALPATH}" ]; then
  DIRNAME=`realpath ${DIRNAME}`
fi

if [ ! -z "$1" ]; then
  # Run single PHP file

  # Setup trace "module:level" and trace output.
  # Note: we were getting segfault in tests/JIT-segfault.php -vEval.Jit=true,
  # it worked fine with -vEval.SimulateARM=true. We've fixed the code by implementing our own sweep() methods.
  # So watch out!
  # (hhvm/hphp/runtime/vm/jit/fixup.cpp, FixupMap::fixup(VMExecutionContext* ec) const {...})

  #TRACE=tx64:1,fixup:3 \
  TRACE=tx64:1,fixup:0 \
    HPHP_TRACE_FILE=/dev/stderr \
    ${HPHP_HOME}/hphp/hhvm/hhvm \
    -vDynamicExtensions.0=${DIRNAME}/uv.so -vEval.SimulateARM=true -vEval.Jit=false \
    "$1"
else
  # Run all tests
  ${HPHP_HOME}/hphp/test/run \
    -a "-vDynamicExtensions.0=${DIRNAME}/uv.so -vEval.Jit=true" \
    ${DIRNAME}/tests
fi

