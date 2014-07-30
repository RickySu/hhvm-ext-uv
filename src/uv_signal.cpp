#include "ext.h"
#include "hphp/runtime/base/thread-init-fini.h"

namespace HPHP {

    typedef struct uv_signal_ext_s:public uv_signal_t{
        bool start = false;
        ObjectData *signal_object_data;        
    } uv_signal_ext_t;
    
    static void signal_handle_callback(uv_signal_ext_t *signal_handle, int signo) {
        CallbackResourceData *signal_resource_data = FETCH_RESOURCE(((uv_signal_ext_t *) signal_handle)->signal_object_data, CallbackResourceData, s_uvsignal);
        vm_call_user_func(signal_resource_data->getCallback(), make_packed_array(((uv_signal_ext_t *) signal_handle)->signal_object_data, signo));
    }
    
    static void HHVM_METHOD(UVSignal, __construct) {
        Resource resource(NEWOBJ(CallbackResourceData(sizeof(uv_signal_ext_t))));
        SET_RESOURCE(this_, resource, s_uvsignal);
        CallbackResourceData *signal_resource_data = FETCH_RESOURCE(this_, CallbackResourceData, s_uvsignal);
        uv_signal_ext_t *signal_handle = (uv_signal_ext_t*) signal_resource_data->getInternalResourceData();
        signal_handle->signal_object_data = getThisOjectData(this_);
        uv_signal_init(uv_default_loop(), signal_handle);
    }
    
    static int64_t HHVM_METHOD(UVSignal, start, const Variant &signal_cb, int64_t signo) {
        CallbackResourceData *resource_data = FETCH_RESOURCE(this_, CallbackResourceData, s_uvsignal);
        uv_signal_ext_t *signal_handle = (uv_signal_ext_t *) resource_data->getInternalResourceData();
        resource_data->setCallback(signal_cb);
        signal_handle->start = true;
        int64_t ret = uv_signal_start(signal_handle, (uv_signal_cb) signal_handle_callback, signo);
        if(ret == 0){
            getThisOjectData(this_)->incRefCount();
        }
        return ret;
    }
    
    static int64_t HHVM_METHOD(UVSignal, stop) {
        CallbackResourceData *resource_data = FETCH_RESOURCE(this_, CallbackResourceData, s_uvsignal);
        uv_signal_ext_t *signal_handle = (uv_signal_ext_t *) resource_data->getInternalResourceData();
        int64_t ret = 0;
        if(signal_handle->start){
            ret = uv_signal_stop((uv_signal_t *) signal_handle);
            getThisOjectData(this_)->decRefAndRelease();
            signal_handle->start = false;            
        }
        return ret;
    }    
    
    static void HHVM_METHOD(UVSignal, __destruct) {
        CallbackResourceData *resource_data = FETCH_RESOURCE(this_, CallbackResourceData, s_uvsignal);
        uv_signal_ext_t *signal_handle = (uv_signal_ext_t *) resource_data->getInternalResourceData();
        uv_unref((uv_handle_t *) signal_handle);
    }
    
    void uvExtension::_initUVSignalClass() {
        HHVM_ME(UVSignal, __construct);
        HHVM_ME(UVSignal, __destruct);        
        HHVM_ME(UVSignal, start);
        HHVM_ME(UVSignal, stop);
    }
}
