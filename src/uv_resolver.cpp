#include "ext.h"

#define RELEASE_INFO(info) \
    delete info->callback; \
    delete info

namespace HPHP {

    typedef struct uv_getaddrinfo_ext_s: public uv_getaddrinfo_t{
        Variant *callback;
    } uv_getaddrinfo_ext_t;
    
    typedef struct uv_getnameinfo_ext_s: public uv_getnameinfo_t {
        Variant *callback;
    } uv_getnameinfo_ext_t;

    typedef struct uv_resolver_s {
        uv_loop_t *loop;
    } uv_resolver_t;
    
    static void on_addrinfo_resolved(uv_getaddrinfo_t *_info, int status, struct addrinfo *res) {
        uv_getaddrinfo_ext_t * info = (uv_getaddrinfo_ext_t *) _info;
        StringData *addrString = NULL;
        char addr[17] = {'\0'};
        if(status == 0){        
            uv_ip4_name((struct sockaddr_in*) res->ai_addr, addr, 16);
            addrString = StringData::Make(addr);
            uv_freeaddrinfo(res);
        }
        vm_call_user_func(*info->callback, make_packed_array(status, addrString));
        RELEASE_INFO(info);
    }

    static void on_nameinfo_resolved(uv_getnameinfo_t *_info, int status, const char *hostname, const char *service) {
        uv_getnameinfo_ext_t * info = (uv_getnameinfo_ext_t *) _info;
        StringData *hostnameString = NULL, *serviceString = NULL;
        if(status == 0){        
            hostnameString = StringData::Make(hostname, CopyString);
            serviceString = StringData::Make(service, CopyString);            
        }
        vm_call_user_func(*info->callback, make_packed_array(status, hostnameString, serviceString));
        RELEASE_INFO(info);
    }
    
    static void HHVM_METHOD(UVResolver, __construct, const Object &o_loop) {
        InternalResourceData *loop_resource_data = FETCH_RESOURCE(o_loop, InternalResourceData, s_uvloop);
        Resource resource(NEWOBJ(ResolverResourceData(sizeof(uv_resolver_t))));
        SET_RESOURCE(this_, resource, s_uvresolver);
        ResolverResourceData *resolver_resource_data = FETCH_RESOURCE(this_, ResolverResourceData, s_uvresolver);
        uv_resolver_t *resolver = (uv_resolver_t *) resolver_resource_data->getInternalResourceData();
        resolver->loop = (uv_loop_t*) loop_resource_data->getInternalResourceData();
    }
    
    static int64_t HHVM_METHOD(UVResolver, getaddrinfo, const String &node, const Variant &service, const Variant &callback) {
        ResolverResourceData *resource_data = FETCH_RESOURCE(this_, ResolverResourceData, s_uvresolver);
        uv_resolver_t *resolver = (uv_resolver_t *) resource_data->getInternalResourceData();
        uv_getaddrinfo_ext_t *getaddrinfo = new uv_getaddrinfo_ext_t();
        getaddrinfo->callback = new Variant(callback);
        int ret = uv_getaddrinfo(resolver->loop, getaddrinfo, on_addrinfo_resolved, node.c_str(), service.toString().c_str(), NULL);
        if(ret != 0){
            RELEASE_INFO(getaddrinfo);
        }
        return ret;
    }
    
    static int64_t HHVM_METHOD(UVResolver, getnameinfo, const String &addr, const Variant &callback) {
        int64_t ret;
        ResolverResourceData *resource_data = FETCH_RESOURCE(this_, ResolverResourceData, s_uvresolver);
        uv_resolver_t *resolver = (uv_resolver_t *) resource_data->getInternalResourceData();        
        static struct sockaddr_in addr4;
        uv_getnameinfo_ext_t *getnameinfo = new uv_getnameinfo_ext_t();
        getnameinfo->callback = new Variant(callback);        
        if((ret = uv_ip4_addr(addr.c_str(), 0, &addr4)) !=0){
            return ret;
        }
        if((ret = uv_getnameinfo(resolver->loop, getnameinfo, on_nameinfo_resolved, (const struct sockaddr*) &addr4, 0)) != 0) {
            RELEASE_INFO(getnameinfo);
        }
        return ret;
    }    
    void uvExtension::_initUVResolverClass() {
        HHVM_ME(UVResolver, __construct);
        HHVM_ME(UVResolver, getaddrinfo);
        HHVM_ME(UVResolver, getnameinfo);
    }
}
