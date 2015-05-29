#ifndef HHVM_INCLUDE_H
#define HHVM_INCLUDE_H

#include "../config.h"
#if HHVM_API_VERSION < 20150212
    #error hhvm version must >= 3.6.0
#endif

#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/base/socket.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/native-data.h"

#endif