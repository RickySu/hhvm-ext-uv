#include "ext.h"

namespace HPHP {

    static void HHVM_METHOD(UVLoop, __construct) {
        uv_loop_t* loop;
        loop = new uv_loop_t();
        uv_loop_init(loop);
        Resource resource(NEWOBJ(UVLoopResource(loop)));        
        SET_RESOURCE(this_, resource, s_uvloop);
    }

    static void HHVM_METHOD(UVLoop, __destruct) {
        UVLoopResource *resource = FETCH_RESOURCE(this_, UVLoopResource, s_uvloop);    
        uv_loop_t* loop = (uv_loop_t *)resource->getInternalResource();
        uv_loop_close(loop);
        delete loop;
    }

    static void HHVM_METHOD(UVLoop, run, int64_t option) {
        uv_run_mode mode;
        UVLoopResource *resource = FETCH_RESOURCE(this_, UVLoopResource, s_uvloop);
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
        uv_run((uv_loop_t *)resource->getInternalResource(), mode);
    }
    
    static int64_t HHVM_METHOD(UVLoop, alive) {
        UVLoopResource *resource = FETCH_RESOURCE(this_, UVLoopResource, s_uvloop);        
        return uv_loop_alive((const uv_loop_t *)resource->getInternalResource());
    }
    
    void uvExtension::_initUVLoopClass() {
        HHVM_ME(UVLoop, __construct);
        HHVM_ME(UVLoop, __destruct);
        HHVM_ME(UVLoop, run);
        HHVM_ME(UVLoop, alive);
    }
}