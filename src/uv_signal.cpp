#include "ext.h"
#include "hphp/runtime/base/thread-init-fini.h"

namespace HPHP {

    typedef struct uv_siginal_ext_s:public uv_signal_t{
        bool start = false;
        CallbackResourceData *callback_resource_data;
    } uv_signal_ext_t;
    
    void signal_handle_callback(uv_signal_ext_t *signal_handle, int signo) {    
        vm_call_user_func(signal_handle->callback_resource_data->getCallback(), make_packed_array(signo));
    }
    
    static void HHVM_METHOD(UVSignal, __construct, const Object *o_loop) {
        InternalResourceData *loop_resource_data = FETCH_RESOURCE(o_loop, InternalResourceData, s_uvloop);
        Resource resource(NEWOBJ(CallbackResourceData(sizeof(uv_signal_ext_t))));
        SET_RESOURCE(this_, resource, s_uvsignal);
        CallbackResourceData *signal_resource_data = FETCH_RESOURCE(this_, CallbackResourceData, s_uvsignal);
        uv_signal_ext_t *signal_handle = (uv_signal_ext_t*) signal_resource_data->getInternalResourceData();
        signal_handle->callback_resource_data = signal_resource_data;
        uv_signal_init((uv_loop_t*) loop_resource_data->getInternalResourceData(), signal_handle);
    }
    
    static int64_t HHVM_METHOD(UVSignal, start, const Object &signal_cb, int64_t signo) {
        CallbackResourceData *resource_data = FETCH_RESOURCE(this_, CallbackResourceData, s_uvsignal);
        uv_signal_ext_t *signal_handle = (uv_signal_ext_t *) resource_data->getInternalResourceData();
        resource_data->setCallback(signal_cb);
        signal_handle->start = true;
        return uv_signal_start(signal_handle, (uv_signal_cb) signal_handle_callback, signo);
    }
    
    static int64_t HHVM_METHOD(UVSignal, stop) {
        CallbackResourceData *resource_data = FETCH_RESOURCE(this_, CallbackResourceData, s_uvsignal);
        uv_signal_ext_t *signal_handle = (uv_signal_ext_t *) resource_data->getInternalResourceData();
        return uv_signal_stop(signal_handle);
    }    
    
    static void HHVM_METHOD(UVSignal, __destruct) {
        CallbackResourceData *resource_data = FETCH_RESOURCE(this_, CallbackResourceData, s_uvsignal);
        uv_signal_ext_t *signal_handle = (uv_signal_ext_t *) resource_data->getInternalResourceData();
        if(signal_handle->start){
            uv_signal_stop(signal_handle);
        }
    }
    
    void uvExtension::_initUVSignalClass() {
        HHVM_ME(UVSignal, __construct);
        HHVM_ME(UVSignal, __destruct);        
        HHVM_ME(UVSignal, start);
        HHVM_ME(UVSignal, stop);
    }
}