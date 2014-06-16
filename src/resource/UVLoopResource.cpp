/*
 * File:   UVLoopResource.cpp
 * Author: ricky
 *
 * Created on 2014年5月19日, 上午 9:48
 */

#include "UVLoopResource.h"

namespace HPHP {

    UVLoopResource::UVLoopResource(uv_loop_t *loop):InternalResource((void*) loop) {
    }

    UVLoopResource::~UVLoopResource() {
    }

}