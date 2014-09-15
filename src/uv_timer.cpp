#include "ext.h"
#include "hphp/runtime/base/thread-init-fini.h"

namespace HPHP {

    typedef struct uv_timer_ext_s:public uv_timer_t{
        bool start;
        ObjectData *timer_object_data;
    } uv_timer_ext_t;
    
    static void timer_handle_callback(uv_timer_ext_t *timer_handle) {
        auto callback = timer_handle->timer_object_data->o_get("callback", false, s_uvtimer);        
        vm_call_user_func(callback, make_packed_array(timer_handle->timer_object_data));
    }
    
    ALWAYS_INLINE void releaseHandle(uv_timer_ext_t *handle) {
          if(handle->start){
            handle->start = false;
            handle->timer_object_data->decRefAndRelease();
          }
    }
    
    static void HHVM_METHOD(UVTimer, __construct) {
        Resource resource(NEWOBJ(CallbackResourceData(sizeof(uv_timer_ext_t))));
        SET_RESOURCE(this_, resource, s_uvtimer);
        CallbackResourceData *timer_resource_data = FETCH_RESOURCE(this_, CallbackResourceData, s_uvtimer);
        uv_timer_ext_t *timer_handle = (uv_timer_ext_t*) timer_resource_data->getInternalResourceData();
        uv_timer_init(uv_default_loop(), timer_handle);
        timer_handle->start = false;
    }
    
    static int64_t HHVM_METHOD(UVTimer, start, const Variant &timer_cb, int64_t start, int64_t repeat) {
        CallbackResourceData *resource_data = FETCH_RESOURCE(this_, CallbackResourceData, s_uvtimer);
        uv_timer_ext_t *timer_handle = (uv_timer_ext_t *) resource_data->getInternalResourceData();
        this_->o_set("callback", timer_cb, s_uvtimer);
        int64_t ret = uv_timer_start(timer_handle, (uv_timer_cb) timer_handle_callback, start, repeat);
        if(ret == 0){
            timer_handle->start = true;
            timer_handle->timer_object_data = getThisOjectData(this_);            
            timer_handle->timer_object_data->incRefCount();
        }
        return ret;
    }
    
    static int64_t HHVM_METHOD(UVTimer, stop) {
        CallbackResourceData *resource_data = FETCH_RESOURCE(this_, CallbackResourceData, s_uvtimer);
        uv_timer_ext_t *timer_handle = (uv_timer_ext_t *) resource_data->getInternalResourceData();
        int64_t ret = 0;
        if(timer_handle->start){
            ret = uv_timer_stop((uv_timer_t *) timer_handle);
            releaseHandle(timer_handle);
        }
        return ret;
    }    
    
    static void HHVM_METHOD(UVTimer, __destruct) {
        CallbackResourceData *resource_data = FETCH_RESOURCE(this_, CallbackResourceData, s_uvtimer);
        uv_timer_ext_t *timer_handle = (uv_timer_ext_t *) resource_data->getInternalResourceData();
        uv_unref((uv_handle_t *) timer_handle);
        releaseHandle(timer_handle);
    }
    
    void uvExtension::_initUVTimerClass() {
        HHVM_ME(UVTimer, __construct);
        HHVM_ME(UVTimer, __destruct);        
        HHVM_ME(UVTimer, start);
        HHVM_ME(UVTimer, stop);
    }
}
