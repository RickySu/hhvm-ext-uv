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
    public:
        DECLARE_RESOURCE_ALLOCATION(CallbackResourceData)
        CLASSNAME_IS("CallbackResourceData")
        CallbackResourceData(unsigned size);
        virtual ~CallbackResourceData();
        void setCallback(const Object &callback_object);
        Object getCallback();
    private:
        ObjectData *callback_object_data;
    };
}

#endif	/* CALLBACK_RESOURCE_DATA_H */

