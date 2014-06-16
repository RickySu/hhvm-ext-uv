#include "ext.h"
#include "hphp/runtime/base/thread-init-fini.h"

namespace HPHP {

    struct EventHandlerData final : RequestEventHandler {
        EventHandlerData() {}
        void destroy() {}
        void requestShutdown() override {}
        void requestInit() override {
            destroy();
        }
        ObjectData *m_event_handler;
    };
    
    IMPLEMENT_STATIC_REQUEST_LOCAL(EventHandlerData, s_event_handler);
    
    void signal_handler_cb(uv_signal_t *_handle, int signo){
        vm_call_user_func(s_event_handler->m_event_handler, make_packed_array(123));
        echo("okoko!!!");
    }
    
    static void HHVM_METHOD(UVSignal, __construct, const Object *o_loop) {
        UVLoopResource *loop_resource = FETCH_RESOURCE(o_loop, UVLoopResource, s_uvloop);
        uv_signal_t *handler = new uv_signal_t();
        uv_signal_init((uv_loop_t*) loop_resource->getInternalResource(), handler);
        Resource resource(NEWOBJ(InternalResource(handler)));
        SET_RESOURCE(this_, resource, s_uvsignal);
    }
    
    static void HHVM_METHOD(UVSignal, start, const Object &signal_cb, int64_t signo) {
        JIT::VMRegAnchor _;
        InternalResource *resource = FETCH_RESOURCE(this_, InternalResource, s_uvsignal);
        uv_signal_t *handler = (uv_signal_t *)resource->getInternalResource();
        vm_call_user_func("var_dump", make_packed_array(123));
        signal_cb.get()->incRefCount();
        s_event_handler->m_event_handler = signal_cb.get();
//        handler->callback = signal_cb.get();
        uv_signal_start(handler, signal_handler_cb, signo);
    }
    
/*    
    static void HHVM_METHOD(UVLoop, __destruct) {
        UVLoopResource *resource = FETCH_RESOURCE(this_, UVLoopResource, s_uvloop);
        uv_loop_t* loop = (uv_loop_t *)resource->getInternalResource();
        uv_loop_close(loop);
        delete loop;
    }
*/
    
    void uvExtension::_initUVSignalClass() {
        HHVM_ME(UVSignal, __construct);
        HHVM_ME(UVSignal, start);
//        HHVM_ME(UVLoop, alive);
    }
}