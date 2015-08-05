#include "ext.h"
#include "uv_natice_data.h"

namespace HPHP {

    typedef struct uv_timer_ext_s:public uv_timer_t{
        bool start;
        ObjectData *timer_object_data;
    } uv_timer_ext_t;

    ALWAYS_INLINE uv_timer_ext_t *fetchResource(UVNativeData *data){
        return (uv_timer_ext_t *) data->resource_handle;
    }
    
    static void timer_handle_callback(uv_timer_ext_t *timer_handle) {
        auto* data = Native::data<UVNativeData>(timer_handle->timer_object_data);
        vm_call_user_func(data->callback, make_packed_array(timer_handle->timer_object_data));
    }
    
    ALWAYS_INLINE void releaseHandle(uv_timer_ext_t *handle) {
         if(handle->start){
            uv_timer_stop((uv_timer_t *) handle);
         }
         uv_unref((uv_handle_t *) handle);
         delete handle;
    }
    
    static void HHVM_METHOD(UVTimer, __construct, const Object &loop) {
        auto* loop_data = Native::data<UVLoopData>(loop.get());
        auto* data = Native::data<UVNativeData>(this_);
        data->resource_handle = (void *) new uv_timer_ext_t();
        SET_LOOP(this_, loop, s_uvtimer);
        uv_timer_ext_t *timer_handle = fetchResource(data);
        uv_timer_init(loop_data->loop, timer_handle);
        timer_handle->start = false;
        timer_handle->timer_object_data = NULL;
    }
    
    static int64_t HHVM_METHOD(UVTimer, start, const Variant &timer_cb, int64_t start, int64_t repeat) {
        auto* data = Native::data<UVNativeData>(this_);
        uv_timer_ext_t *timer_handle = fetchResource(data);
        data->callback = timer_cb;
        int64_t ret = uv_timer_start(timer_handle, (uv_timer_cb) timer_handle_callback, start, repeat);
        if(ret == 0){
            timer_handle->start = true;
            timer_handle->timer_object_data = this_;
            this_->incRefCount();
        }
        return ret;
    }
    
    static int64_t HHVM_METHOD(UVTimer, stop) {
        auto* data = Native::data<UVNativeData>(this_);
        uv_timer_ext_t *timer_handle = fetchResource(data);
        int64_t ret = 0;
        if(timer_handle->start){
            ret = uv_timer_stop((uv_timer_t *) timer_handle);
            timer_handle->start = false;
            this_->decRefAndRelease();
        }
        return ret;
    }    
    
    static void HHVM_METHOD(UVTimer, __destruct) {
        auto* data = Native::data<UVNativeData>(this_);
        uv_timer_ext_t *timer_handle = fetchResource(data);
        releaseHandle(timer_handle);
    }
    
    void uvExtension::_initUVTimerClass() {
        HHVM_ME(UVTimer, __construct);
        HHVM_ME(UVTimer, __destruct);        
        HHVM_ME(UVTimer, start);
        HHVM_ME(UVTimer, stop);
        Native::registerNativeDataInfo<UVNativeData>(s_uvtimer.get());
    }
}
