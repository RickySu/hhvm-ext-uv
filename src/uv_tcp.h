#ifndef UV_TCP_H
#define UV_TCP_H

#include "ext.h"

#define UV_TCP_HANDLE_INTERNAL_REF 1
#define UV_TCP_HANDLE_START (1<<1)
#define UV_TCP_READ_START (1<<2)
#define UV_TCP_CLOSING_START (1<<3)
#define UV_TCP_WRITE_CALLBACK_ENABLE (1<<4)
//#define UV_TCP_CLIENT_CONNECT_START (1<<3)

#define releaseObjectData(o) if(o != NULL) { \
    o->decRefAndRelease(); \
    o = NULL; \
}
    
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
    } uv_tcp_ext_t;
    
    typedef struct write_req_s: public uv_write_t {
        uv_buf_t buf;
    } write_req_t;
                                                                         

    int64_t tcp_write_raw(uv_stream_t * handle, const char *data, int64_t size);
    void tcp_close_cb(uv_handle_t* handle);
    void HHVM_METHOD(UVTcp, __destruct);
    void HHVM_METHOD(UVTcp, __construct);
    int64_t HHVM_METHOD(UVTcp, listen, const String &host, int64_t port, const Variant &onConnectCallback);
    uv_tcp_ext_t *initUVTcpObject(ObjectData *objectData, uv_loop_t *loop);
    
    ALWAYS_INLINE void tcp_close_socket(uv_tcp_ext_t *handle){
        if(handle->flag & UV_TCP_CLOSING_START){
            return;
        }
        handle->flag |= UV_TCP_CLOSING_START;
        uv_close((uv_handle_t *) handle, tcp_close_cb);        
    }
    
    ALWAYS_INLINE void releaseHandle(uv_tcp_ext_t *handle);
    
    class UVTcpData {
        public:
            uv_tcp_ext_t *tcp_handle = NULL;
            Variant connectCallback;
            Variant readCallback;
            Variant writeCallback;
            Variant errorCallback;
            Variant shutdownCallback;
            UVTcpData(){
                connectCallback.setNull();
                readCallback.setNull();
                writeCallback.setNull();
                errorCallback.setNull();
                shutdownCallback.setNull();
            }
            ~UVTcpData();
            void release();
    };    

    ObjectData *make_accepted_uv_tcp_object(ObjectData *objectData, const String &class_name, uv_tcp_ext_t *tcp_handler);
    uv_tcp_ext_t *initUVTcpObject(ObjectData *objectData, uv_loop_t *loop, uv_tcp_ext_t *tcp_handler);

    ALWAYS_INLINE ObjectData *make_accepted_uv_tcp_object(ObjectData *objectData, const String &class_name) {
        return make_accepted_uv_tcp_object(objectData, class_name, new uv_tcp_ext_t());
    }
    
    ALWAYS_INLINE uv_tcp_ext_t *initUVTcpObject(ObjectData *objectData, uv_loop_t *loop) {
         return initUVTcpObject(objectData, loop, new uv_tcp_ext_t());
    }    
}

#endif
