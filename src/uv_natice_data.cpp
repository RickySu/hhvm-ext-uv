#include "uv_natice_data.h"

namespace HPHP {
    void UVNativeData::sweep(){
        callback.releaseForSweep();
    }
    UVNativeData::~UVNativeData(){
        sweep();
    }
}
                                 