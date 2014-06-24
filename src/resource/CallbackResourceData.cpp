/*
 * File:   EventBufferEventResourceData.cpp
 * Author: ricky
 *
 * Created on 2014年5月19日, 上午 9:48
 */

#include "CallbackResourceData.h"

namespace HPHP {
    IMPLEMENT_OBJECT_ALLOCATION(CallbackResourceData)

    CallbackResourceData::CallbackResourceData(unsigned size):InternalResourceData(size) {
        callback_object_data = NULL;
    }

    void CallbackResourceData::setCallback(const Object &callback_object)
    {
        callback_object_data = callback_object.get();
        callback_object_data->incRefCount();
    }

    Object CallbackResourceData::getCallback()
    {
        return Object(callback_object_data);
    }

    CallbackResourceData::~CallbackResourceData() {
        if(callback_object_data != NULL){
            echo("release CallbackData\n");
            callback_object_data->decRefCount();
            callback_object_data = NULL;
        }
    }

}