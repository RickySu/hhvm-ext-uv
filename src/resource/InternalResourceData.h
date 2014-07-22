/*
 * File:   InternalResource
 * Author: ricky
 *
 * Created on 2014年5月19日, 上午 9:48
 */

#ifndef UVLOOP_RESOURCE_DATA_H
#define	UVLOOP_RESOURCE_DATA_H

#include "hphp/runtime/base/base-includes.h"
#include <uv.h>

#define DECLARE_CALLBACK_OBJECT_DATA(object) \
    private: Variant object##_data;

#define DECLARE_CALLBACK_OBJECT(method, object) \
    public: void set##method(const Variant &object); \
    public: Variant &get##method(); \
    DECLARE_CALLBACK_OBJECT_DATA(object)

#define IMPLEMENT_CALLBACK_OBJECT(classname, method, object) \
    void classname::set##method(const Variant &object) {\
            object##_data = object;\
    } \
    Variant &classname::get##method() {\
        return object##_data;\
    }
namespace HPHP {

    class InternalResourceData : public  SweepableResourceData{
    public:
        virtual const String& o_getClassNameHook() const { return classnameof(); }
        DECLARE_RESOURCE_ALLOCATION(InternalResourceData)
        CLASSNAME_IS("InternalResourceData")
        InternalResourceData(unsigned size);
        virtual ~InternalResourceData();
        void *getInternalResourceData();        
    private:        
        void *resource;
    };
}

#endif	/* UVLOOP_RESOURCE_DATA_H */

