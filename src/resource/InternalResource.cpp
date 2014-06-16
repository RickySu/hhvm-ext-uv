/*
 * File:   InternalResource.cpp
 * Author: ricky
 *
 * Created on 2014年5月14日, 下午 12:13
 */

#include "InternalResource.h"

namespace HPHP {
    IMPLEMENT_OBJECT_ALLOCATION(InternalResource)

    InternalResource::InternalResource(void *resource) {
        this->resource = resource;
    }

    void * InternalResource::getInternalResource() {
        return this->resource;
    }

    void InternalResource::setObject(const Object &obj) {
        this->object = &obj;
    }

    const Object *InternalResource::getObject() {
        return object;
    }

    InternalResource::~InternalResource() {
    }

}