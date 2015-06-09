#include "ext.h"
#define RUN_DEFAULT  0
#define RUN_ONCE  1
#define RUN_NOWAIT 2


#define REGISTER_UV_LOOP_CONSTANT(name) \
    Native::registerClassConstant<KindOfInt64>(s_uvloop.get(), \
            makeStaticString(#name), \
            name)

namespace HPHP {

#if HHVM_API_VERSION < 20140702L
    using JIT::VMRegAnchor;
#endif

    static void HHVM_METHOD(UVLoop, run, int64_t option) {
        uv_run_mode mode;
        VMRegAnchor _;
        switch(option){
            case RUN_DEFAULT:
                mode = UV_RUN_DEFAULT;
                break;
            case RUN_ONCE:
                mode = UV_RUN_ONCE;
                break;            
            case RUN_NOWAIT:
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
        return uv_backend_fd(uv_default_loop());
    }
    
    static int64_t HHVM_METHOD(UVLoop, backendTimeout) {
        return uv_backend_timeout(uv_default_loop());
    }    
    
    void uvExtension::_initUVLoopClass() {
        REGISTER_UV_LOOP_CONSTANT(RUN_DEFAULT);
        REGISTER_UV_LOOP_CONSTANT(RUN_ONCE);
        REGISTER_UV_LOOP_CONSTANT(RUN_NOWAIT);
        
        HHVM_ME(UVLoop, run);
        HHVM_ME(UVLoop, alive);
        HHVM_ME(UVLoop, updateTime);
        HHVM_ME(UVLoop, now);
        HHVM_ME(UVLoop, backendFd);
        HHVM_ME(UVLoop, backendTimeout);
    }
}
