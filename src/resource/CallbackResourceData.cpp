/*
 * File:   CallbackResourceData.h
 * Author: ricky
 *
 * Created on 2014年5月19日, 上午 9:48
 */

#include "CallbackResourceData.h"

namespace HPHP {
    IMPLEMENT_OBJECT_ALLOCATION(CallbackResourceData)
    IMPLEMENT_CALLBACK_OBJECT(CallbackResourceData, Callback, callback_object)
    CallbackResourceData::CallbackResourceData(unsigned size):InternalResourceData(size) {
    }

    CallbackResourceData::~CallbackResourceData() {
    }

}