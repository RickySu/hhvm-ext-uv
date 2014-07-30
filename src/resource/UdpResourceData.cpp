/*
 * File:   UdpResourceData.cpp
 * Author: ricky
 *
 * Created on 2014年5月19日, 上午 9:48
 */

#include "UdpResourceData.h"

namespace HPHP {
    IMPLEMENT_OBJECT_ALLOCATION(UdpResourceData)
    IMPLEMENT_CALLBACK_OBJECT(UdpResourceData, RecvCallback, recv_callback_object)
    IMPLEMENT_CALLBACK_OBJECT(UdpResourceData, SendCallback, send_callback_object)
    IMPLEMENT_CALLBACK_OBJECT(UdpResourceData, ErrorCallback, error_callback_object)
    UdpResourceData::UdpResourceData(unsigned size):InternalResourceData(size) {
    }

    UdpResourceData::~UdpResourceData() {
        SWEEP_CALLBACK_OBJECT_DATA(recv_callback_object);
        SWEEP_CALLBACK_OBJECT_DATA(send_callback_object);
        SWEEP_CALLBACK_OBJECT_DATA(error_callback_object);
    }

}