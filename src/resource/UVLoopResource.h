/*
 * File:   UVLoopResource.h
 * Author: ricky
 *
 * Created on 2014年5月19日, 上午 9:48
 */

#ifndef UVLOOP_RESOURCE_H
#define	UVLOOP_RESOURCE_H

#include <uv.h>
#include "InternalResource.h"

namespace HPHP {

    class UVLoopResource : public InternalResource {
    public:
        UVLoopResource(uv_loop_t *loop);
        virtual ~UVLoopResource();
    private:

    };
}

#endif	/* UVLOOP_RESOURCE_H */

