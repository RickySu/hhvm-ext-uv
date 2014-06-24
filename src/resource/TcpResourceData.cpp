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
    IMPLEMENT_CALLBACK_OBJECT(TcpResourceData, ReadCallback, read_callback_object)
    IMPLEMENT_CALLBACK_OBJECT(TcpResourceData, WriteCallback, write_callback_object)
    IMPLEMENT_CALLBACK_OBJECT(TcpResourceData, ErrorCallback, error_callback_object)
    TcpResourceData::TcpResourceData(unsigned size):InternalResourceData(size) {
    }

    TcpResourceData::~TcpResourceData() {
        GC_OBJECT_DATA(connect_callback_object);
        GC_OBJECT_DATA(read_callback_object);
        GC_OBJECT_DATA(write_callback_object);
        GC_OBJECT_DATA(error_callback_object);
    }

}