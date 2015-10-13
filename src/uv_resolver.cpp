#include "ext.h"

#define RELEASE_INFO(info) \
    info->resolver_object_data->decRefAndRelease();\
    delete info

namespace HPHP {

    typedef struct uv_getaddrinfo_ext_s: public uv_getaddrinfo_t{
        ObjectData *resolver_object_data;
    } uv_getaddrinfo_ext_t;
    
    typedef struct uv_getnameinfo_ext_s: public uv_getnameinfo_t {
        ObjectData *resolver_object_data;
    } uv_getnameinfo_ext_t;

    class UVResolverData{
        public:
             uv_getaddrinfo_ext_t *getaddrinfo_resource = NULL;
             uv_getnameinfo_ext_t *getnameinfo_resource = NULL;
             Variant addrinfoCallback;
             Variant nameinfoCallback;
             void sweep(){
                 addrinfoCallback.releaseForSweep();
                 nameinfoCallback.releaseForSweep();
                 if(getaddrinfo_resource){
                     delete getaddrinfo_resource;
                 }
                 if(getnameinfo_resource){
                     delete getnameinfo_resource;
                 }
             }
             ~UVResolverData(){
                 sweep();
             }
    };

    static void on_addrinfo_resolved(uv_getaddrinfo_ext_t *info, int status, struct addrinfo *res) {
        auto* data = Native::data<UVResolverData>(info->resolver_object_data);
        auto callback = data->addrinfoCallback;
        char addr[17] = {'\0'};
        if(status == 0){        
            uv_ip4_name((struct sockaddr_in*) res->ai_addr, addr, 16);
            uv_freeaddrinfo(res);
        }
        vm_call_user_func(callback, make_packed_array(status, String(addr)));
        RELEASE_INFO(info);
    }

    static void on_nameinfo_resolved(uv_getnameinfo_ext_t *info, int status, const char *hostname, const char *service) {
        auto* data = Native::data<UVResolverData>(info->resolver_object_data);
        auto callback = data->nameinfoCallback;
        if(status == 0){        
            vm_call_user_func(callback, make_packed_array(status, String(hostname, CopyString), String(service, CopyString)));
        }
        else{
            vm_call_user_func(callback, make_packed_array(status, NULL, NULL));
        }
        RELEASE_INFO(info);
    }
    
    static int64_t HHVM_METHOD(UVResolver, getaddrinfo, const String &node, const Variant &service, const Variant &callback) {
        auto* data = Native::data<UVResolverData>(this_);
        auto* loop_data = getLoopData(this_, s_uvresolver);
        uv_getaddrinfo_ext_t *getaddrinfo = new uv_getaddrinfo_ext_t();
        getaddrinfo->resolver_object_data = this_;
        getaddrinfo->resolver_object_data->incRefCount();
        data->addrinfoCallback = callback;
        int ret = uv_getaddrinfo(loop_data==NULL?uv_default_loop():loop_data->loop, getaddrinfo, (uv_getaddrinfo_cb) on_addrinfo_resolved, node.c_str(), service.toString().c_str(), NULL);
        if(ret != 0){
            RELEASE_INFO(getaddrinfo);
        }
        return ret;
    }
    
    static int64_t HHVM_METHOD(UVResolver, getnameinfo, const String &addr, const Variant &callback) {
        auto* data = Native::data<UVResolverData>(this_);
        auto* loop_data = getLoopData(this_, s_uvresolver);
        int64_t ret;
        static struct sockaddr_in addr4;
        
        if((ret = uv_ip4_addr(addr.c_str(), 0, &addr4)) !=0){
            return ret;
        }
        
        uv_getnameinfo_ext_t *getnameinfo = new uv_getnameinfo_ext_t();
        getnameinfo->resolver_object_data = this_;
        getnameinfo->resolver_object_data->incRefCount();
        data->nameinfoCallback = callback;
        
        if((ret = uv_getnameinfo(loop_data==NULL?uv_default_loop():loop_data->loop, getnameinfo, (uv_getnameinfo_cb) on_nameinfo_resolved, (const struct sockaddr*) &addr4, 0)) != 0) {
            RELEASE_INFO(getnameinfo);
        }
        
        return ret;
    }
    
    static void HHVM_METHOD(UVResolver, __construct, const Variant &v_loop) {
        if(v_loop.isNull()){
            return;
        }
        
        Object loop = v_loop.toObject();
        checkUVLoopInstance(loop, 1, s_uvresolver, StaticString("__construct"));
        SET_LOOP(this_, loop, s_uvresolver);
    }
    
    void uvExtension::_initUVResolverClass() {
        HHVM_ME(UVResolver, __construct);
        HHVM_ME(UVResolver, getaddrinfo);
        HHVM_ME(UVResolver, getnameinfo);
        Native::registerNativeDataInfo<UVResolverData>(s_uvresolver.get());
    }
}
