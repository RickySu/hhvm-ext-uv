/*
 * File:   TcpResourceData.h
 * Author: ricky
 *
 * Created on 2014年5月19日, 上午 9:48
 */

#ifndef TCP_RESOURCE_DATA_H
#define	TCP_RESOURCE_DATA_H

#include "InternalResourceData.h"

namespace HPHP {

    class TcpResourceData : public InternalResourceData {
        DECLARE_CALLBACK_OBJECT(ConnectCallback, connect_callback_object)
        DECLARE_CALLBACK_OBJECT(ReadCallback, read_callback_object)
        DECLARE_CALLBACK_OBJECT(WriteCallback, write_callback_object)        
    public:
        DECLARE_RESOURCE_ALLOCATION(TcpResourceData)
        CLASSNAME_IS("TcpResourceData")
        TcpResourceData(unsigned size);
        virtual ~TcpResourceData();
    };
}

#endif	/* TCP_RESOURCE_DATA_H */
