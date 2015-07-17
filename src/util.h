#ifndef UTIL_H
#define	UTIL_H

#define TIMEVAL_SET(tv, t) \
    do {                                             \
        tv.tv_sec  = (long) t;                       \
        tv.tv_usec = (long) ((t - tv.tv_sec) * 1e6); \
    } while (0)

#define TIMEVAL_TO_DOUBLE(tv) (tv.tv_sec + tv.tv_usec * 1e-6)
    
namespace HPHP
{

    ALWAYS_INLINE ObjectData *makeObject(const String &ClassName, const Array arg, bool init){
        Class* cls = Unit::lookupClass(ClassName.get());
        ObjectData* ret;
        Object o = ret = ObjectData::newInstance(cls);        
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
}

#endif	/* UTIL_H */
