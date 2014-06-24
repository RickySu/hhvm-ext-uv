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

