/*
 * File:   CallbackResourceData.h
 * Author: ricky
 *
 * Created on 2014年5月19日, 上午 9:48
 */

#ifndef CALLBACK_RESOURCE_DATA_H
#define	CALLBACK_RESOURCE_DATA_H

#include "InternalResourceData.h"

namespace HPHP {

    class CallbackResourceData : public InternalResourceData {
        DECLARE_CALLBACK_OBJECT(Callback, callback_object)        
    public:
        DECLARE_RESOURCE_ALLOCATION(CallbackResourceData)
        CLASSNAME_IS("CallbackResourceData")
        
        CallbackResourceData(unsigned size);
        virtual ~CallbackResourceData();
    };
}

#endif	/* CALLBACK_RESOURCE_DATA_H */

