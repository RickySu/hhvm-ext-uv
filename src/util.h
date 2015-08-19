#ifndef UTIL_H
#define	UTIL_H

#include "uv_loop_data.h"

#define TIMEVAL_SET(tv, t) \
    do {                                             \
        tv.tv_sec  = (long) t;                       \
        tv.tv_usec = (long) ((t - tv.tv_sec) * 1e6); \
    } while (0)

#define TIMEVAL_TO_DOUBLE(tv) (tv.tv_sec + tv.tv_usec * 1e-6)



#define FETCH_LOOP_EXT(obj, resource_class, ctx, rs) ({\
    auto __var__get = obj->o_get(rs, false, ctx); \
    if(__var__get.isNull()) raise_error("resource is invalid."); \
        __var__get.asCResRef().getTyped<resource_class>(); \
    })
                
#define FETCH_LOOP(obj, resource_class, ctx) FETCH_LOOP_EXT(obj, resource_class, ctx, s_internal_loop)

#define SET_LOOP_EXT(obj, resource, ctx, rs) \
    obj->o_set(rs, resource, ctx)
                        
#define SET_LOOP(obj, resource, ctx) SET_LOOP_EXT(obj, resource, ctx, s_internal_loop)
    
namespace HPHP
{

    ALWAYS_INLINE ObjectData *makeObject(const String &ClassName, const Array arg, bool init){
        Class* cls = Unit::lookupClass(ClassName.get());
        ObjectData* ret = ObjectData::newInstance(cls);
        Object o(ret);
        if(init){
            TypedValue dummy;
            g_context->invokeFunc(&dummy, cls->getCtor(), arg, ret);
        }
        return o.detach();
    }

    ALWAYS_INLINE ObjectData *makeObject(const String &ClassName, bool init = true){
        return makeObject(ClassName, Array::Create(), init);
    }

    ALWAYS_INLINE ObjectData *makeObject(const char *ClassName, const Array arg){
        return makeObject(String(ClassName), arg, true);
    }

    ALWAYS_INLINE ObjectData *makeObject(const char *ClassName, bool init = true){
        return makeObject(String(ClassName), Array::Create(), init);
    }

    ALWAYS_INLINE StringData *sock_addr(struct sockaddr *addr) {
        struct sockaddr_in addr_in = *(struct sockaddr_in *) addr;
        char ip[20];
        uv_ip4_name(&addr_in, ip, sizeof ip);
        return StringData::Make(ip, CopyString);
    }
    
    ALWAYS_INLINE int sock_port(struct sockaddr *addr) {
        struct sockaddr_in addr_in = *(struct sockaddr_in *) addr;
        return ntohs(addr_in.sin_port);
    }
    
    ALWAYS_INLINE ObjectData* getThisOjectData(const Object &obj){
        return obj.get();
    }
    
    ALWAYS_INLINE ObjectData* getThisObjectData(ObjectData *objdata){
        return objdata;
    }
    
    ALWAYS_INLINE UVLoopData* getLoopData(ObjectData *objectdata){
        auto loop = objectdata->o_get("loop", false, s_uvresolver).toObject();
        return Native::data<UVLoopData>(loop.get());
    }
    
}

#endif	/* UTIL_H */
