#include "ext.h"
#include "hphp/runtime/base/thread-init-fini.h"
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
    } uv_tcp_ext_t;
    
    typedef struct write_req_s: public uv_write_t {
        uv_buf_t buf;
    } write_req_t;

    ALWAYS_INLINE void releaseHandle(uv_tcp_ext_t *handle) {
        if(handle->flag & UV_TCP_READ_START){
            handle->flag &= ~UV_TCP_READ_START;        
            uv_read_stop((uv_stream_t *) handle);
        }
        
        if(handle->flag & UV_TCP_HANDLE_START){
            handle->flag &= ~UV_TCP_HANDLE_START;        
            uv_unref((uv_handle_t *) handle);
        }    
        
        if(handle->flag & UV_TCP_HANDLE_INTERNAL_REF){
            handle->flag &= ~UV_TCP_HANDLE_INTERNAL_REF;
            ((uv_tcp_ext_t *) handle)->tcp_object_data->decRefAndRelease();
        }
        
        if(handle->sockPort != -1){
            handle->sockPort = -1;
            handle->sockAddr->decRefAndRelease();
            handle->sockAddr = NULL;
        }
        
        if(handle->peerPort != -1){
            handle->peerPort = -1;
            handle->peerAddr->decRefAndRelease();
            handle->peerAddr = NULL;
        }        
    }
    
    ALWAYS_INLINE uv_tcp_ext_t *initUVTcpObject(const Object &object, uv_loop_t *loop) {
        Resource resource(NEWOBJ(TcpResourceData(sizeof(uv_tcp_ext_t))));
        SET_RESOURCE(object, resource, s_uvtcp);
        TcpResourceData *tcp_resource_data = FETCH_RESOURCE(object, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) tcp_resource_data->getInternalResourceData();
        tcp_handle->flag = 0;
        tcp_handle->sockAddr = tcp_handle->peerAddr = NULL;
        tcp_handle->sockPort = tcp_handle->peerPort = -1;                                
        tcp_handle->tcp_object_data = object.get();
        uv_tcp_init(loop, tcp_handle);
        return tcp_handle;
    }    

    static void write_cb(uv_write_t *wr, int status) {      
        write_req_t *req = (write_req_t *) wr;
        TcpResourceData *tcp_resource_data = FETCH_RESOURCE(((uv_tcp_ext_t *) req->handle)->tcp_object_data, TcpResourceData, s_uvtcp);
        Variant callback = tcp_resource_data->getWriteCallback();
        if(!callback.isNull()){
            vm_call_user_func(callback, make_packed_array(((uv_tcp_ext_t *) req->handle)->tcp_object_data, status, req->buf.len));
        }
        delete req->buf.base;
        delete req;
    }
    
    static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
        buf->base = new char[suggested_size];
        buf->len = suggested_size;
    }

    static void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
        TcpResourceData *tcp_resource_data = FETCH_RESOURCE(((uv_tcp_ext_t *) stream)->tcp_object_data, TcpResourceData, s_uvtcp);
        if(nread > 0){
            if(!tcp_resource_data->getReadCallback().isNull()){
                vm_call_user_func(tcp_resource_data->getReadCallback(), make_packed_array(((uv_tcp_ext_t *) stream)->tcp_object_data, StringData::Make(buf->base, nread, CopyString)));
            }
        }
        else{
            if(!tcp_resource_data->getErrorCallback().isNull()){
                vm_call_user_func(tcp_resource_data->getErrorCallback(), make_packed_array(((uv_tcp_ext_t *) stream)->tcp_object_data, nread));
            }
        }
        delete buf->base;
    }
    
    static void close_cb(uv_handle_t* handle) {
       releaseHandle((uv_tcp_ext_t *) handle);    
    }
    
    static void client_connection_cb(uv_connect_t* req, int status) {
        TcpResourceData *tcp_resource_data = FETCH_RESOURCE(((uv_tcp_ext_t *) req->handle)->tcp_object_data, TcpResourceData, s_uvtcp);
        Variant callback = tcp_resource_data->getConnectCallback();
        if(!callback.isNull()){
            vm_call_user_func(callback, make_packed_array(((uv_tcp_ext_t *) req->handle)->tcp_object_data, status));
        }    
    }
    
    static void shutdown_cb(uv_shutdown_t* req, int status) {
        TcpResourceData *tcp_resource_data = FETCH_RESOURCE(((uv_tcp_ext_t *) req->handle)->tcp_object_data, TcpResourceData, s_uvtcp);
        Variant callback = tcp_resource_data->getShutdownCallback();
        if(!callback.isNull()){
            vm_call_user_func(callback, make_packed_array(((uv_tcp_ext_t *) req->handle)->tcp_object_data, status));
        }    
    }    
    
    static void connection_cb(uv_stream_t* server, int status) {
        TcpResourceData *tcp_resource_data = FETCH_RESOURCE(((uv_tcp_ext_t *) server)->tcp_object_data, TcpResourceData, s_uvtcp);
        Variant callback = tcp_resource_data->getConnectCallback();
        if(!callback.isNull()){
            vm_call_user_func(callback, make_packed_array(((uv_tcp_ext_t *) server)->tcp_object_data, status));
        }
    }
            
    static void HHVM_METHOD(UVTcp, __construct) {
        initUVTcpObject(this_, uv_default_loop());
    }
    
    static void HHVM_METHOD(UVTcp, __destruct) {
        TcpResourceData *resource_data = FETCH_RESOURCE(this_, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();
        releaseHandle(tcp_handle);    
    }
    
    static int64_t HHVM_METHOD(UVTcp, listen, const String &host, int64_t port, const Variant &onConnectCallback) {
        int64_t ret;
        struct sockaddr_in addr;
        TcpResourceData *resource_data = FETCH_RESOURCE(this_, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();
        
        if((ret = uv_ip4_addr(host.c_str(), port&0xffff, &addr)) != 0){
            return ret;
        }

        if((ret = uv_tcp_bind(tcp_handle, (const struct sockaddr*) &addr, 0)) != 0){
            return ret;
        }
        
        if((ret = uv_listen((uv_stream_t *) tcp_handle, SOMAXCONN, connection_cb)) != 0){
            return ret;
        }
        
        resource_data->setConnectCallback(onConnectCallback);        
        tcp_handle->flag |= UV_TCP_HANDLE_START;
        return ret;
    }
    
    static Object HHVM_METHOD(UVTcp, accept) {
        TcpResourceData *resource_data = FETCH_RESOURCE(this_, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *server_tcp_handle, *client_tcp_handle;
        server_tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();
        Object client = makeObject("UVTcp", false);
        client_tcp_handle = initUVTcpObject(client, server_tcp_handle->loop);
        if(uv_accept((uv_stream_t *) server_tcp_handle, (uv_stream_t *) client_tcp_handle)) {
            return NULL;
        }
        client.get()->incRefCount();
        client_tcp_handle->flag |= UV_TCP_HANDLE_INTERNAL_REF;
        return client;
    }
    
    static void HHVM_METHOD(UVTcp, close) {
        TcpResourceData *resource_data = FETCH_RESOURCE(this_, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();
        uv_close((uv_handle_t *) tcp_handle, close_cb);
    }
    
    static int64_t HHVM_METHOD(UVTcp, setCallback, const Variant &onReadCallback, const Variant &onWriteCallback, const Variant &onErrorCallback) {
        int64_t ret;
        TcpResourceData *resource_data = FETCH_RESOURCE(this_, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();
        if((ret = uv_read_start((uv_stream_t *) tcp_handle, alloc_cb, read_cb)) == 0){
            resource_data->setReadCallback(onReadCallback);
            resource_data->setWriteCallback(onWriteCallback);
            resource_data->setErrorCallback(onErrorCallback);
            tcp_handle->flag |= (UV_TCP_HANDLE_START|UV_TCP_READ_START);
        }
        return ret;
    }
    
    static int64_t HHVM_METHOD(UVTcp, write, const String &data) {
        TcpResourceData *resource_data = FETCH_RESOURCE(this_, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();
        write_req_t *req;
        req = new write_req_t();
        req->buf.base = new char[data.size()];
        req->buf.len = data.size();
        memcpy((void *) req->buf.base, data.c_str(), data.size());        
        return uv_write((uv_write_t *) req, (uv_stream_t *) tcp_handle, &req->buf, 1, write_cb);
    }
    
    static String HHVM_METHOD(UVTcp, getSockname) {
        struct sockaddr addr;
        int addrlen;
        TcpResourceData *resource_data = FETCH_RESOURCE(this_, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();
        
        if(tcp_handle->sockPort == -1){
            addrlen = sizeof addr;
            if(uv_tcp_getsockname((const uv_tcp_t *)tcp_handle, &addr, &addrlen)){
                return String();
            }
            tcp_handle->sockAddr = sock_addr(&addr);
            tcp_handle->sockPort = sock_port(&addr);
            tcp_handle->sockAddr->incRefCount();
        }
        
        return tcp_handle->sockAddr;
    }

    static String HHVM_METHOD(UVTcp, getPeername) {
        struct sockaddr addr;
        int addrlen;
        TcpResourceData *resource_data = FETCH_RESOURCE(this_, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();
        
        if(tcp_handle->peerPort == -1){
            addrlen = sizeof addr;
            if(uv_tcp_getpeername((const uv_tcp_t *)tcp_handle, &addr, &addrlen)){
                return String();
            }
            
            tcp_handle->peerAddr = sock_addr(&addr);
            tcp_handle->peerPort = sock_port(&addr);
            tcp_handle->peerAddr->incRefCount();
        }
        
        return tcp_handle->peerAddr;
    }
    
    static int64_t HHVM_METHOD(UVTcp, getSockport) {
        struct sockaddr addr;
        int addrlen;
        TcpResourceData *resource_data = FETCH_RESOURCE(this_, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();

        if(tcp_handle->sockPort == -1){
            addrlen = sizeof addr;
            if(uv_tcp_getsockname((const uv_tcp_t *)tcp_handle, &addr, &addrlen)){
                return -1;
            }
            tcp_handle->sockAddr = sock_addr(&addr);
            tcp_handle->sockPort = sock_port(&addr);
            tcp_handle->sockAddr->incRefCount();
        }
        
        return tcp_handle->sockPort;
    }    
    
    static int64_t HHVM_METHOD(UVTcp, getPeerport) {
        struct sockaddr addr;
        int addrlen;
        TcpResourceData *resource_data = FETCH_RESOURCE(this_, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();

        if(tcp_handle->peerPort == -1){
            addrlen = sizeof addr;
            if(uv_tcp_getpeername((const uv_tcp_t *)tcp_handle, &addr, &addrlen)){
                return -1;
            }
            tcp_handle->peerAddr = sock_addr(&addr);
            tcp_handle->peerPort = sock_port(&addr);
            tcp_handle->peerAddr->incRefCount();
        }
        
        return tcp_handle->peerPort;
    }

    static int64_t HHVM_METHOD(UVTcp, connect, const String &host, int64_t port, const Variant &onConnectCallback) {
        int64_t ret;
        struct sockaddr_in addr;
        TcpResourceData *resource_data = FETCH_RESOURCE(this_, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();
        
        if((ret = uv_ip4_addr(host.c_str(), port&0xffff, &addr)) != 0){
            return ret;
        }
        if((ret = uv_tcp_connect(&tcp_handle->connect_req, (uv_tcp_t *) tcp_handle, (const struct sockaddr*) &addr, client_connection_cb)) != 0){
            return ret;
        }
        
        resource_data->setConnectCallback(onConnectCallback);        
        tcp_handle->flag |= UV_TCP_HANDLE_START;
        return ret;
    }

    static int64_t HHVM_METHOD(UVTcp, shutdown,const Variant &onShutdownCallback) {
        int64_t ret;
        TcpResourceData *resource_data = FETCH_RESOURCE(this_, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();
        
        if((ret = uv_shutdown(&tcp_handle->shutdown_req, (uv_stream_t *) tcp_handle, shutdown_cb)) != 0){
            return ret;
        }
        
        resource_data->setShutdownCallback(onShutdownCallback);
        tcp_handle->flag |= UV_TCP_HANDLE_START;
        return ret;
    }

    
    void uvExtension::_initUVTcpClass() {
        HHVM_ME(UVTcp, __construct);
        HHVM_ME(UVTcp, __destruct);        
        HHVM_ME(UVTcp, listen);
        HHVM_ME(UVTcp, connect);
        HHVM_ME(UVTcp, accept);
        HHVM_ME(UVTcp, close);
        HHVM_ME(UVTcp, shutdown);
        HHVM_ME(UVTcp, setCallback);
        HHVM_ME(UVTcp, write);
        HHVM_ME(UVTcp, getSockname);
        HHVM_ME(UVTcp, getPeername);
        HHVM_ME(UVTcp, getSockport);
        HHVM_ME(UVTcp, getPeerport);                
    }
}