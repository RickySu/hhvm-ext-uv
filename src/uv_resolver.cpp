#include "ext.h"
#include "hphp/runtime/base/thread-init-fini.h"

namespace HPHP {

    typedef struct uv_getaddrinfo_ext_s: public uv_getaddrinfo_t{
        ObjectData *resolver_object_data;        
    } uv_getaddrinfo_ext_t;
    
    typedef struct uv_getnameinfo_ext_s: public uv_getnameinfo_t {
        ObjectData *resolver_object_data;        
    } uv_getnameinfo_ext_t;

    typedef struct uv_resolver_s {
        uv_getnameinfo_ext_t *getnameinfo;
        uv_getaddrinfo_ext_t *getaddrinfo;
    } uv_resolver_t;
    
    static void HHVM_METHOD(UVSignal, __construct, const Object &o_loop) {
        InternalResourceData *loop_resource_data = FETCH_RESOURCE(o_loop, InternalResourceData, s_uvloop);
        Resource resource(NEWOBJ(CallbackResourceData(sizeof(uv_resolver_t))));
        SET_RESOURCE(this_, resource, s_uvresolver);
        CallbackResourceData *resolver_resource_data = FETCH_RESOURCE(this_, CallbackResourceData, s_uvresolver);
        uv_resolver_t *resolver = (uv_resolver_t *) resolver_resource_data->getInternalResourceData();
        resolver->getnameinfo = NULL;
        resolver->getaddrinfo = NULL;
    }
    
    static void HHVM_METHOD(UVSignal, __destruct) {
        CallbackResourceData *resource_data = FETCH_RESOURCE(this_, CallbackResourceData, s_uvsignal);
        uv_resolver_t *resolver = (uv_resolver_t *) resolver_resource_data->getInternalResourceData();
        
        if(resolver->getnameinfo != NULL){
            delete resolver->getnameinfo;
            resolver->getnameinfo = NULL;
        }
        
        if(resolver->getaddrinfo != NULL){
            delete resolver->getaddrinfo;
            resolver->getaddrinfo = NULL;
        }
    }
    
    void uvExtension::_initUVSignalClass() {
        HHVM_ME(UVSignal, __construct);
        HHVM_ME(UVSignal, __destruct);        
    }
}
