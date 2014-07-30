#include "ext.h"

namespace HPHP {
    using JIT::VMRegAnchor;
    
    static void HHVM_METHOD(UVLoop, run, int64_t option) {
        uv_run_mode mode;
        VMRegAnchor _;
        switch(option){
            case 0:
                mode = UV_RUN_DEFAULT;
                break;
            case 1:
                mode = UV_RUN_ONCE;
                break;            
            case 2:
                mode = UV_RUN_NOWAIT;
                break;
            default:
                mode = UV_RUN_DEFAULT;
        }
        uv_run(uv_default_loop(), mode);
    }
    
    static int64_t HHVM_METHOD(UVLoop, alive) {
        return uv_loop_alive(uv_default_loop());
    }
    
    static void HHVM_METHOD(UVLoop, updateTime) {
        uv_update_time(uv_default_loop());
    }
    
    static int64_t HHVM_METHOD(UVLoop, now) {
        return uv_now(uv_default_loop());
    }
    
    static int64_t HHVM_METHOD(UVLoop, backendFd) {
        InternalResourceData *resource_data = FETCH_RESOURCE(this_, InternalResourceData, s_uvloop);
        return uv_backend_fd((const uv_loop_t *)resource_data->getInternalResourceData());
    }
    
    static int64_t HHVM_METHOD(UVLoop, backendTimeout) {
        return uv_backend_timeout(uv_default_loop());
    }    
    
    void uvExtension::_initUVLoopClass() {
        HHVM_ME(UVLoop, run);
        HHVM_ME(UVLoop, alive);
        HHVM_ME(UVLoop, updateTime);
        HHVM_ME(UVLoop, now);
        HHVM_ME(UVLoop, backendFd);
        HHVM_ME(UVLoop, backendTimeout);
    }
}
