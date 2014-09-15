/*
 * File:   InternalResourceData.cpp
 * Author: ricky
 *
 * Created on 2014年5月19日, 上午 9:48
 */

#include "InternalResourceData.h"

namespace HPHP {
    IMPLEMENT_OBJECT_ALLOCATION(InternalResourceData)
    
    InternalResourceData::InternalResourceData(unsigned size) {
        resource = (void *) new char[size];
        memset(resource, '\0', size);        
    }

    InternalResourceData::~InternalResourceData() {
        delete ((char *)resource);
    }
    
    void *InternalResourceData::getInternalResourceData(){
        return resource;
    }

}
