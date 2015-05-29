#include "uv_natice_data.h"

namespace HPHP {
    UVNativeData::~UVNativeData(){
        callback.releaseForSweep();
    }
}
                                 