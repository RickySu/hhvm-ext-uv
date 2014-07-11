/*
 * File:   ResolverResourceData.h
 * Author: ricky
 *
 * Created on 2014年5月19日, 上午 9:48
 */

#ifndef RESOLVER_RESOURCE_DATA_H
#define	RESOLVER_RESOURCE_DATA_H

#include "InternalResourceData.h"

namespace HPHP {

    class ResolverResourceData : public InternalResourceData {
        DECLARE_CALLBACK_OBJECT(getnameinfoCallback, getnameinfo_callback_object)
    public:
        DECLARE_RESOURCE_ALLOCATION(ResolverResourceData)
        CLASSNAME_IS("ResolverResourceData")
        ResolverResourceData(unsigned size);
        virtual ~ResolverResourceData();
    };
}

#endif	/* RESOLVER_RESOURCE_DATA_H */
