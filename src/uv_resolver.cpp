#include "ext.h"

#define RELEASE_INFO(info) \
    info->resolver_object_data->decRefAndRelease();\
    delete info

namespace HPHP {

    class UVResolver {
        
    };
    
    typedef struct uv_getaddrinfo_ext_s: public uv_getaddrinfo_t{
        ObjectData *resolver_object_data;
    } uv_getaddrinfo_ext_t;
    
    typedef struct uv_getnameinfo_ext_s: public uv_getnameinfo_t {
        ObjectData *resolver_object_data;
    } uv_getnameinfo_ext_t;

    static void on_addrinfo_resolved(uv_getaddrinfo_ext_t *info, int status, struct addrinfo *res) {
        auto callback = info->resolver_object_data->o_get("addrinfoCallback", false, s_uvresolver);
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
        StringData *hostnameString = NULL, *serviceString = NULL;
        auto callback = info->resolver_object_data->o_get("nameinfoCallback", false, s_uvresolver);
        if(status == 0){        
            hostnameString = StringData::Make(hostname, CopyString);
            serviceString = StringData::Make(service, CopyString);            
        }
        vm_call_user_func(callback, make_packed_array(status, hostnameString, serviceString));
        RELEASE_INFO(info);
    }
    
    static int64_t HHVM_METHOD(UVResolver, getaddrinfo, const String &node, const Variant &service, const Variant &callback) {
        uv_getaddrinfo_ext_t *getaddrinfo = new uv_getaddrinfo_ext_t();
        getaddrinfo->resolver_object_data = getThisOjectData(this_);
        getaddrinfo->resolver_object_data->incRefCount();
        this_->o_set("addrinfoCallback", callback, s_uvresolver);
        int ret = uv_getaddrinfo(uv_default_loop(), getaddrinfo, (uv_getaddrinfo_cb) on_addrinfo_resolved, node.c_str(), service.toString().c_str(), NULL);
        if(ret != 0){
            RELEASE_INFO(getaddrinfo);
        }
        return ret;
    }
    
    static int64_t HHVM_METHOD(UVResolver, getnameinfo, const String &addr, const Variant &callback) {
        int64_t ret;
        static struct sockaddr_in addr4;
        
        if((ret = uv_ip4_addr(addr.c_str(), 0, &addr4)) !=0){
            return ret;
        }
        
        uv_getnameinfo_ext_t *getnameinfo = new uv_getnameinfo_ext_t();
        getnameinfo->resolver_object_data = getThisOjectData(this_);
        getnameinfo->resolver_object_data->incRefCount();
        this_->o_set("nameinfoCallback", callback, s_uvresolver);        
        
        if((ret = uv_getnameinfo(uv_default_loop(), getnameinfo, (uv_getnameinfo_cb) on_nameinfo_resolved, (const struct sockaddr*) &addr4, 0)) != 0) {
            RELEASE_INFO(getnameinfo);
        }
        
        return ret;
    }
    
    void uvExtension::_initUVResolverClass() {
        HHVM_ME(UVResolver, getaddrinfo);
        HHVM_ME(UVResolver, getnameinfo);
        Native::registerNativeDataInfo<UVResolver>(s_uvresolver.get());
    }
}
