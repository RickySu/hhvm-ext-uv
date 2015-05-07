#ifndef HHVM_INCLUDE_H
#define HHVM_INCLUDE_H

#include "../config.h"
#if HHVM_API_VERSION < 20150212
    #include "hphp/runtime/base/base-includes.h"
#else
    #include "hphp/runtime/ext/extension.h"
#endif

#include "hphp/runtime/base/socket.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#endif