#include "ext.h"

namespace HPHP {

    static void HHVM_METHOD(UVLoop, __construct) {
        Resource resource(NEWOBJ(InternalResourceData(sizeof(uv_loop_t))));
        SET_RESOURCE(this_, resource, s_uvloop);
        InternalResourceData *resource_data = FETCH_RESOURCE(this_, InternalResourceData, s_uvloop);
        uv_loop_init((uv_loop_t *) resource_data->getInternalResourceData());
    }

    static void HHVM_METHOD(UVLoop, run, int64_t option) {
        uv_run_mode mode;
        InternalResourceData *resource_data = FETCH_RESOURCE(this_, InternalResourceData, s_uvloop);
        JIT::VMRegAnchor _;
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
        uv_run((uv_loop_t *) resource_data->getInternalResourceData(), mode);
    }
    
    static int64_t HHVM_METHOD(UVLoop, alive) {
        InternalResourceData *resource_data = FETCH_RESOURCE(this_, InternalResourceData, s_uvloop);
        return uv_loop_alive((const uv_loop_t *)resource_data->getInternalResourceData());
    }
    
    static void HHVM_METHOD(UVLoop, updateTime) {
        InternalResourceData *resource_data = FETCH_RESOURCE(this_, InternalResourceData, s_uvloop);
        uv_update_time((uv_loop_t *)resource_data->getInternalResourceData());
    }
    
    static int64_t HHVM_METHOD(UVLoop, now) {
        InternalResourceData *resource_data = FETCH_RESOURCE(this_, InternalResourceData, s_uvloop);
        return uv_now((const uv_loop_t *)resource_data->getInternalResourceData());
    }
    
    static int64_t HHVM_METHOD(UVLoop, backendFd) {
        InternalResourceData *resource_data = FETCH_RESOURCE(this_, InternalResourceData, s_uvloop);
        return uv_backend_fd((const uv_loop_t *)resource_data->getInternalResourceData());
    }
    
    static int64_t HHVM_METHOD(UVLoop, backendTimeout) {
        InternalResourceData *resource_data = FETCH_RESOURCE(this_, InternalResourceData, s_uvloop);
        return uv_backend_timeout((const uv_loop_t *)resource_data->getInternalResourceData());
    }    
    
    void uvExtension::_initUVLoopClass() {
        HHVM_ME(UVLoop, __construct);
        HHVM_ME(UVLoop, run);
        HHVM_ME(UVLoop, alive);
        HHVM_ME(UVLoop, updateTime);
        HHVM_ME(UVLoop, now);
        HHVM_ME(UVLoop, backendFd);
        HHVM_ME(UVLoop, backendTimeout);
    }
}