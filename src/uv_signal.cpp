#include "ext.h"
#include "uv_natice_data.h"

namespace HPHP {

    typedef struct uv_signal_ext_s:public uv_signal_t{
        bool start;
        ObjectData *signal_object_data;        
    } uv_signal_ext_t;

    ALWAYS_INLINE uv_signal_ext_t *fetchResource(UVNativeData *data){
        return (uv_signal_ext_t *) data->resource_handle;
    }
    
    ALWAYS_INLINE void releaseHandle(uv_signal_ext_t *handle) {
        if(handle->start){
            uv_signal_stop((uv_signal_t *) handle);
        }
        uv_unref((uv_handle_t *) handle);
        delete handle;
    }
    
    static void signal_handle_callback(uv_signal_ext_t *signal_handle, int signo) {
        auto* data = Native::data<UVNativeData>(signal_handle->signal_object_data);
        vm_call_user_func(data->callback, make_packed_array(signal_handle->signal_object_data, signo));
    }
    
    static void HHVM_METHOD(UVSignal, __construct, const Object &loop) {
        auto* loop_data = Native::data<UVLoopData>(loop.get());
        auto* data = Native::data<UVNativeData>(this_);
        SET_LOOP(this_, loop, s_uvsignal);
        data->resource_handle = (void *) new uv_signal_ext_t();
        uv_signal_ext_t *signal_handle = fetchResource(data);
        uv_signal_init(loop_data->loop, signal_handle);
        signal_handle->start = false;
        signal_handle->signal_object_data = NULL;
    }
    
    static int64_t HHVM_METHOD(UVSignal, start, const Variant &signal_cb, int64_t signo) {
        auto* data = Native::data<UVNativeData>(this_);
        uv_signal_ext_t *signal_handle = fetchResource(data);
        data->callback = signal_cb;
        int64_t ret = uv_signal_start(signal_handle, (uv_signal_cb) signal_handle_callback, signo);
        if(ret == 0){
            signal_handle->start = true;
            signal_handle->signal_object_data = this_;
            this_->incRefCount();
        }
        return ret;
    }
    
    static int64_t HHVM_METHOD(UVSignal, stop) {
        auto* data = Native::data<UVNativeData>(this_);
        uv_signal_ext_t *signal_handle = fetchResource(data);
        int64_t ret = 0;
        if(signal_handle->start){
            ret = uv_signal_stop((uv_signal_t *) signal_handle);
            this_->decRefAndRelease();
            signal_handle->start = false;            
        }
        return ret;
    }    
    
    static void HHVM_METHOD(UVSignal, __destruct) {
        auto* data = Native::data<UVNativeData>(this_);
        uv_signal_ext_t *signal_handle = fetchResource(data);
        releaseHandle(signal_handle);
    }
    
    void uvExtension::_initUVSignalClass() {
        HHVM_ME(UVSignal, __construct);
        HHVM_ME(UVSignal, __destruct);        
        HHVM_ME(UVSignal, start);
        HHVM_ME(UVSignal, stop);
        Native::registerNativeDataInfo<UVNativeData>(s_uvsignal.get());
    }
}
