/*
 * File:   ResolverResourceData.cpp
 * Author: ricky
 *
 * Created on 2014年5月19日, 上午 9:48
 */

#include "ResolverResourceData.h"

namespace HPHP {
    IMPLEMENT_OBJECT_ALLOCATION(ResolverResourceData)
    IMPLEMENT_CALLBACK_OBJECT(ResolverResourceData, getnameinfoCallback, getnameinfo_callback_object)
    ResolverResourceData::ResolverResourceData(unsigned size):InternalResourceData(size) {
    }

    ResolverResourceData::~ResolverResourceData() {
        GC_OBJECT_DATA(getnameinfo_callback_object);
    }

}