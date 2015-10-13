#include "ext.h"
#define RUN_DEFAULT  0
#define RUN_ONCE  1
#define RUN_NOWAIT 2


#define REGISTER_UV_LOOP_CONSTANT(name) \
    Native::registerClassConstant<KindOfInt64>(s_uvloop.get(), \
            makeStaticString(#name), \
            name)

namespace HPHP {

UVLoopData::UVLoopData(){
    loop = new uv_loop_t();
    uv_loop_init(loop);
}
void UVLoopData::sweep(){
    if(loop != uv_default_loop()){
        uv_loop_close(loop);
        delete loop;
    }
}
UVLoopData::~UVLoopData() {
    sweep();
}

#if HHVM_API_VERSION < 20140702L
    using JIT::VMRegAnchor;
#endif

    static void HHVM_METHOD(UVLoop, run, int64_t option) {
        uv_run_mode mode;
        VMRegAnchor _;
        auto* data = Native::data<UVLoopData>(this_);
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
        uv_run(data->loop, mode);
    }
    
    static int64_t HHVM_METHOD(UVLoop, alive) {
        auto* data = Native::data<UVLoopData>(this_);
        return uv_loop_alive(data->loop);
    }
    
    static void HHVM_METHOD(UVLoop, stop) {
        auto* data = Native::data<UVLoopData>(this_);
        uv_stop(data->loop);
    }
    
    static void HHVM_METHOD(UVLoop, updateTime) {
        auto* data = Native::data<UVLoopData>(this_);
        uv_update_time(data->loop);
    }
    
    static int64_t HHVM_METHOD(UVLoop, now) {
        auto* data = Native::data<UVLoopData>(this_);
        return uv_now(data->loop);
    }
    
    static int64_t HHVM_METHOD(UVLoop, backendFd) {
        auto* data = Native::data<UVLoopData>(this_);
        return uv_backend_fd(data->loop);
    }
    
    static int64_t HHVM_METHOD(UVLoop, backendTimeout) {
        auto* data = Native::data<UVLoopData>(this_);
        return uv_backend_timeout(data->loop);
    }    
    
    static Object HHVM_STATIC_METHOD(UVLoop, makeDefaultLoop) {
        Object loop(makeObject(s_uvloop, false));
        auto* loop_data = Native::data<UVLoopData>(loop.get());
        loop_data->loop = uv_default_loop();
        return loop;
    }
    
    void uvExtension::_initUVLoopClass() {
        REGISTER_UV_LOOP_CONSTANT(RUN_DEFAULT);
        REGISTER_UV_LOOP_CONSTANT(RUN_ONCE);
        REGISTER_UV_LOOP_CONSTANT(RUN_NOWAIT);
        
        HHVM_STATIC_ME(UVLoop, makeDefaultLoop);
        HHVM_ME(UVLoop, run);
        HHVM_ME(UVLoop, stop);
        HHVM_ME(UVLoop, alive);
        HHVM_ME(UVLoop, updateTime);
        HHVM_ME(UVLoop, now);
        HHVM_ME(UVLoop, backendFd);
        HHVM_ME(UVLoop, backendTimeout);
        Native::registerNativeDataInfo<UVLoopData>(s_uvloop.get());
    }
}
