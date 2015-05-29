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
             ~UVResolverData(){
                 addrinfoCallback.releaseForSweep();
                 nameinfoCallback.releaseForSweep();
                 if(getaddrinfo_resource){
                     delete getaddrinfo_resource;
                 }
                 if(getnameinfo_resource){
                     delete getnameinfo_resource;
                 }
             }
    };

    static void on_addrinfo_resolved(uv_getaddrinfo_ext_t *info, int status, struct addrinfo *res) {
        auto* data = Native::data<UVResolverData>(info->resolver_object_data);
        auto callback = data->addrinfoCallback;
        StringData *addrString = NULL;
        char addr[17] = {'\0'};
        if(status == 0){        
            uv_ip4_name((struct sockaddr_in*) res->ai_addr, addr, 16);
            addrString = StringData::Make(addr);
            uv_freeaddrinfo(res);
        }
        vm_call_user_func(callback, make_packed_array(status, addrString));
        RELEASE_INFO(info);
    }

    static void on_nameinfo_resolved(uv_getnameinfo_ext_t *info, int status, const char *hostname, const char *service) {
        auto* data = Native::data<UVResolverData>(info->resolver_object_data);
        auto callback = data->nameinfoCallback;
        StringData *hostnameString = NULL, *serviceString = NULL;
        if(status == 0){        
            hostnameString = StringData::Make(hostname, CopyString);
            serviceString = StringData::Make(service, CopyString);            
        }
        vm_call_user_func(callback, make_packed_array(status, hostnameString, serviceString));
        RELEASE_INFO(info);
    }
    
    static int64_t HHVM_METHOD(UVResolver, getaddrinfo, const String &node, const Variant &service, const Variant &callback) {
        auto* data = Native::data<UVResolverData>(this_);
        uv_getaddrinfo_ext_t *getaddrinfo = new uv_getaddrinfo_ext_t();
        getaddrinfo->resolver_object_data = this_;
        getaddrinfo->resolver_object_data->incRefCount();
        data->addrinfoCallback = callback;
        int ret = uv_getaddrinfo(uv_default_loop(), getaddrinfo, (uv_getaddrinfo_cb) on_addrinfo_resolved, node.c_str(), service.toString().c_str(), NULL);
        if(ret != 0){
            RELEASE_INFO(getaddrinfo);
        }
        return ret;
    }
    
    static int64_t HHVM_METHOD(UVResolver, getnameinfo, const String &addr, const Variant &callback) {
        auto* data = Native::data<UVResolverData>(this_);
        int64_t ret;
        static struct sockaddr_in addr4;
        
        if((ret = uv_ip4_addr(addr.c_str(), 0, &addr4)) !=0){
            return ret;
        }
        
        uv_getnameinfo_ext_t *getnameinfo = new uv_getnameinfo_ext_t();
        getnameinfo->resolver_object_data = this_;
        getnameinfo->resolver_object_data->incRefCount();
        data->nameinfoCallback = callback;
        
        if((ret = uv_getnameinfo(uv_default_loop(), getnameinfo, (uv_getnameinfo_cb) on_nameinfo_resolved, (const struct sockaddr*) &addr4, 0)) != 0) {
            RELEASE_INFO(getnameinfo);
        }
        
        return ret;
    }
    
    void uvExtension::_initUVResolverClass() {
        HHVM_ME(UVResolver, getaddrinfo);
        HHVM_ME(UVResolver, getnameinfo);
        Native::registerNativeDataInfo<UVResolverData>(s_uvresolver.get());
    }
}
