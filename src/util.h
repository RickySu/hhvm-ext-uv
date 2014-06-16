#ifndef UTIL_H
#define	UTIL_H

#define TIMEVAL_SET(tv, t) \
    do {                                             \
        tv.tv_sec  = (long) t;                       \
        tv.tv_usec = (long) ((t - tv.tv_sec) * 1e6); \
    } while (0)

#define TIMEVAL_TO_DOUBLE(tv) (tv.tv_sec + tv.tv_usec * 1e-6)

#define FETCH_RESOURCE(obj, resource_class, ctx) ({\
    auto __var__get = obj->o_get(s_internal_resource, false, ctx); \
    if(__var__get.isNull()) raise_error("resource is invalid."); \
    __var__get.asCResRef().getTyped<resource_class>(); \
    })
 #define SET_RESOURCE(obj, resource, ctx) \
    obj->o_set(s_internal_resource, resource, ctx); \
    resource.getTyped<InternalResource>()->setObject(obj);

#endif	/* UTIL_H */

