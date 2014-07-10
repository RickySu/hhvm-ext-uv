/*
 * File:   UdpResourceData.h
 * Author: ricky
 *
 * Created on 2014年5月19日, 上午 9:48
 */

#ifndef UDP_RESOURCE_DATA_H
#define	UDP_RESOURCE_DATA_H

#include "InternalResourceData.h"

namespace HPHP {

    class UdpResourceData : public InternalResourceData {
        DECLARE_CALLBACK_OBJECT(RecvCallback, recv_callback_object)
        DECLARE_CALLBACK_OBJECT(SendCallback, send_callback_object)
        DECLARE_CALLBACK_OBJECT(ErrorCallback, error_callback_object)
    public:
        DECLARE_RESOURCE_ALLOCATION(UdpResourceData)
        CLASSNAME_IS("UdpResourceData")
        UdpResourceData(unsigned size);
        virtual ~UdpResourceData();
    };
}

#endif	/* UDP_RESOURCE_DATA_H */
