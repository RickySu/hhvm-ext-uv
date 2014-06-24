#ifndef EXT_H_
#define EXT_H_
#include "../config.h"
#include "hphp/runtime/base/base-includes.h"
#include "hphp/runtime/base/socket.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "resource/InternalResourceData.h"
#include "resource/CallbackResourceData.h"
#include "resource/TcpResourceData.h"
#include "common.h"
#include "util.h"
#include <uv.h>

namespace HPHP
{
    class uvExtension: public Extension
    {
        public:
            uvExtension(): Extension("uv"){}
            virtual void moduleInit()
            {
                _initUVUtilClass();            
                _initUVLoopClass();
                _initUVSignalClass();
                _initUVTcpClass();
                loadSystemlib();
            }
        private:
            void _initUVUtilClass();
            void _initUVLoopClass();
            void _initUVSignalClass();
            void _initUVTcpClass();
    };
}
#endif
