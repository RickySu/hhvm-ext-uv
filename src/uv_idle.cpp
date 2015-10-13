#include "ext.h"
#include "uv_natice_data.h"

namespace HPHP {

    typedef struct uv_idle_ext_s:public uv_idle_t{
        bool start;
        ObjectData *idle_object_data;        
    } uv_idle_ext_t;

    ALWAYS_INLINE uv_idle_ext_t *fetchResource(UVNativeData *data){
        return (uv_idle_ext_t *) data->resource_handle;
    }
    
    ALWAYS_INLINE void releaseHandle(uv_idle_ext_t *handle) {
        if(handle->start){
            uv_idle_stop((uv_idle_t *) handle);
        }
        uv_unref((uv_handle_t *) handle);
        delete handle;
    }
    
    static void idle_handle_callback(uv_idle_ext_t *idle_handle) {
        auto* data = Native::data<UVNativeData>(idle_handle->idle_object_data);
        vm_call_user_func(data->callback, make_packed_array(idle_handle->idle_object_data));
    }
    
    static void HHVM_METHOD(UVIdle, __construct, const Variant &v_loop) {
        auto* data = Native::data<UVNativeData>(this_);
        data->resource_handle = (void *) new uv_idle_ext_t();
        uv_idle_ext_t *idle_handle = fetchResource(data);
        idle_handle->start = false;
        idle_handle->idle_object_data = NULL;
        
        if(v_loop.isNull()){
            uv_idle_init(uv_default_loop(), idle_handle);
            return;
        }
        
        Object loop = v_loop.toObject();
        checkUVLoopInstance(loop, 1, s_uvidle, StaticString("__construct"));
        auto* loop_data = Native::data<UVLoopData>(loop.get());        
        SET_LOOP(this_, loop, s_uvidle);
        uv_idle_init(loop_data->loop, idle_handle);

    }
    
    static int64_t HHVM_METHOD(UVIdle, start, const Variant &idle_cb) {
        auto* data = Native::data<UVNativeData>(this_);
        uv_idle_ext_t *idle_handle = fetchResource(data);
        data->callback = idle_cb;
        int64_t ret = uv_idle_start(idle_handle, (uv_idle_cb) idle_handle_callback);
        if(ret == 0){
            idle_handle->start = true;
            idle_handle->idle_object_data = this_;
            this_->incRefCount();
        }
        return ret;
    }
    
    static int64_t HHVM_METHOD(UVIdle, stop) {
        auto* data = Native::data<UVNativeData>(this_);
        uv_idle_ext_t *idle_handle = fetchResource(data);
        int64_t ret = 0;
        if(idle_handle->start){
            ret = uv_idle_stop((uv_idle_t *) idle_handle);
            this_->decRefAndRelease();
            idle_handle->start = false;            
        }
        return ret;
    }    
    
    static void HHVM_METHOD(UVIdle, __destruct) {
        auto* data = Native::data<UVNativeData>(this_);
        uv_idle_ext_t *idle_handle = fetchResource(data);
        releaseHandle(idle_handle);
    }
    
    void uvExtension::_initUVIdleClass() {
        HHVM_ME(UVIdle, __construct);
        HHVM_ME(UVIdle, __destruct);        
        HHVM_ME(UVIdle, start);
        HHVM_ME(UVIdle, stop);
        Native::registerNativeDataInfo<UVNativeData>(s_uvidle.get());
    }
}
