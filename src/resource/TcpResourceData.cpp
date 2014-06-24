/*
 * File:   TcpResourceData.cpp
 * Author: ricky
 *
 * Created on 2014年5月19日, 上午 9:48
 */

#include "TcpResourceData.h"

namespace HPHP {
    IMPLEMENT_OBJECT_ALLOCATION(TcpResourceData)
    IMPLEMENT_CALLBACK_OBJECT(TcpResourceData, ConnectCallback, connect_callback_object)
    TcpResourceData::TcpResourceData(unsigned size):InternalResourceData(size) {
    }

    TcpResourceData::~TcpResourceData() {
        GC_OBJECT_DATA(connect_callback_object);
    }

}