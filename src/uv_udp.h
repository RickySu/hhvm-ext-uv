#ifndef UV_UDP_H
#define UV_UDP_H

#include "ext.h"

#define UV_UDP_HANDLE_INTERNAL_REF 1
#define UV_UDP_HANDLE_START (1<<1)
#define UV_UDP_READ_START (1<<2)

namespace HPHP {

    typedef struct uv_udp_ext_s:public uv_udp_t{
        uint flag;
        ObjectData *udp_object_data;
        StringData *sockAddr;
        int sockPort;
    } uv_udp_ext_t;
    
    typedef struct send_req_s: public uv_udp_send_t {
        uv_buf_t buf;
        struct sockaddr_in addr;
    } send_req_t;

    ALWAYS_INLINE void releaseHandle(uv_udp_ext_t *handle);    
    class UVUdpData {
        public:
            uv_udp_ext_t *udp_handle = NULL;
            Variant recvCallback;
            Variant sendCallback;
            Variant errorCallback;
            UVUdpData(){
                recvCallback.setNull();
                sendCallback.setNull();
                errorCallback.setNull();
            }
            ~UVUdpData();
            void release();
    };    
}

#endif
