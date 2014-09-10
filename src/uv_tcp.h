#ifndef UV_TCP__H
#define UV_TCP_H

#define UV_TCP_HANDLE_INTERNAL_REF 1
#define UV_TCP_HANDLE_START (1<<1)
#define UV_TCP_READ_START (1<<2)
//#define UV_TCP_CLIENT_CONNECT_START (1<<3)

namespace HPHP {

    typedef struct uv_tcp_ext_s:public uv_tcp_t{
        uint flag;
        uv_connect_t connect_req;
        uv_shutdown_t shutdown_req;
        ObjectData *tcp_object_data;
        StringData *sockAddr;
        StringData *peerAddr;
        int sockPort;
        int peerPort;
        void *custom_data;
    } uv_tcp_ext_t;
    
    typedef struct write_req_s: public uv_write_t {
        uv_buf_t buf;
    } write_req_t;
    Object make_accepted_uv_tcp_object(const Object &obj, const String &class_name);
    
    void HHVM_METHOD(UVTcp, __destruct);
    void HHVM_METHOD(UVTcp, __construct);
    int64_t HHVM_METHOD(UVTcp, listen, const String &host, int64_t port, const Variant &onConnectCallback);
}

#endif
