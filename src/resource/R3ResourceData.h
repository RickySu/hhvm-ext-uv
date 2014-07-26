/*
 * File:   R3Resource
 * Author: ricky
 *
 * Created on 2014年5月19日, 上午 9:48
 */

#ifndef UVLOOP_R3RESOURCE_DATA_H
#define	UVLOOP_R3RESOURCE_DATA_H
#include<r3/r3.h>
#include "hphp/runtime/base/base-includes.h"

namespace HPHP {

    class R3ResourceData : public  SweepableResourceData{
    public:
        virtual const String& o_getClassNameHook() const { return classnameof(); }
        DECLARE_RESOURCE_ALLOCATION(R3ResourceData)
        CLASSNAME_IS("R3ResourceData")
        R3ResourceData(int count);
        virtual ~R3ResourceData();
        node *getR3ResourceData();
    private:        
        node *n;
    };
}

#endif

