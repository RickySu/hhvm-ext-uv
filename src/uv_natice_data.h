#ifndef UV_NATIVE_DATA_H
#define UV_NATIVE_DATA_H

#include "ext.h"

namespace HPHP {
    class UVNativeData {
        public:
            void *resource_handle = NULL;
            Variant callback;
            ~UVNativeData();
            void sweep();
    };
}

#endif
