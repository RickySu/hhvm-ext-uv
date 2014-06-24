#include "ext.h"
#include "hphp/runtime/base/thread-init-fini.h"

namespace HPHP {

    typedef struct uv_tcp_ext_s:public uv_tcp_t{
        bool start;
        ObjectData *tcp_object_data;
        TcpResourceData *tcp_resource_data;
    } uv_tcp_ext_t;
    
    typedef struct write_req_s: public uv_write_t {
        uv_buf_t buf;
    } write_req_t;

    ALWAYS_INLINE uv_tcp_ext_t *initUVTcpObject(const Object &object, uv_loop_t *loop) {
        Resource resource(NEWOBJ(TcpResourceData(sizeof(uv_tcp_ext_t))));
        SET_RESOURCE(object, resource, s_uvtcp);
        TcpResourceData *tcp_resource_data = FETCH_RESOURCE(object, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) tcp_resource_data->getInternalResourceData();
        tcp_handle->start = false;
        tcp_handle->tcp_resource_data = tcp_resource_data;
        tcp_handle->tcp_object_data = object.get();
        uv_tcp_init(loop, tcp_handle);
        return tcp_handle;
    }    

    static void onwrite(uv_write_t *wr, int status) {
        echo("write ok\n");
        write_req_t *req = (write_req_t *) wr;
        TcpResourceData *tcp_resource_data = ((uv_tcp_ext_t *) req->handle)->tcp_resource_data;
        Object callback = tcp_resource_data->getWriteCallback();
        vm_call_user_func(callback, make_packed_array(((uv_tcp_ext_t *) req->handle)->tcp_object_data, status));
        delete req->buf.base;        
        delete req;
    }
    
    static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
        buf->base = new char[suggested_size];
        buf->len = suggested_size;
    }

    static void read_cb(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
        echo("native read\n");
        TcpResourceData *tcp_resource_data = ((uv_tcp_ext_t *) stream)->tcp_resource_data;        
        if(nread > 0){
            vm_call_user_func(tcp_resource_data->getReadCallback(), make_packed_array(((uv_tcp_ext_t *) stream)->tcp_object_data, StringData::Make(buf->base, nread, CopyString)));
        }
        else{
            vm_call_user_func(tcp_resource_data->getErrorCallback(), make_packed_array(((uv_tcp_ext_t *) stream)->tcp_object_data, nread));
        }
        delete buf->base;
    }
    
    static void close_cb(uv_handle_t* handle) {
       ((uv_tcp_ext_t *) handle)->tcp_object_data->decRefAndRelease();
       echo("closed\n");
    }
    
    static void connection_cb(uv_stream_t* server, int status) {
        TcpResourceData *tcp_resource_data = ((uv_tcp_ext_t *) server)->tcp_resource_data;
        Object callback = tcp_resource_data->getConnectCallback();
        if(!callback.isNull()){
            vm_call_user_func(callback, make_packed_array(((uv_tcp_ext_t *) server)->tcp_object_data, status));
        }
    }
            
    static void HHVM_METHOD(UVTcp, __construct, const Object *o_loop) {
        InternalResourceData *loop_resource_data = FETCH_RESOURCE(o_loop, InternalResourceData, s_uvloop);
        initUVTcpObject(this_, (uv_loop_t *) loop_resource_data->getInternalResourceData());
    }
    
    static void HHVM_METHOD(UVTcp, __destruct) {
        TcpResourceData *resource_data = FETCH_RESOURCE(this_, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();
        echo("UVTCP gc...\n");
        if(tcp_handle->start){
            uv_unref((uv_handle_t *) tcp_handle);
        }
    }
    
    static bool HHVM_METHOD(UVTcp, listen, const String host, int64_t port, const Object &onConnectCallback) {
        struct sockaddr_in addr;
        TcpResourceData *resource_data = FETCH_RESOURCE(this_, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();
        
        if(uv_ip4_addr(host.c_str(), port&0xffff, &addr)){
            return false;
        }

        if(uv_tcp_bind(tcp_handle, (const struct sockaddr*) &addr, 0)){
            return false;
        }
        
        if(uv_listen((uv_stream_t *) tcp_handle, SOMAXCONN, connection_cb)){
            return false;
        }
        
        resource_data->setConnectCallback(onConnectCallback);
        
        tcp_handle->start = true;
        return true;
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
        client_tcp_handle->start = true;
        uv_read_start((uv_stream_t *) client_tcp_handle, alloc_cb, read_cb);
        return client;
    }
    
    static void HHVM_METHOD(UVTcp, close) {
        TcpResourceData *resource_data = FETCH_RESOURCE(this_, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();
        uv_close((uv_handle_t *) tcp_handle, close_cb);
    }
    
    static void HHVM_METHOD(UVTcp, setCallback, const Object &onReadCallback, const Object &onWriteCallback, const Object &onErrorCallback) {
        TcpResourceData *resource_data = FETCH_RESOURCE(this_, TcpResourceData, s_uvtcp);
        resource_data->setReadCallback(onReadCallback);
        resource_data->setWriteCallback(onWriteCallback);
        resource_data->setErrorCallback(onErrorCallback);        
    }
    
    static bool HHVM_METHOD(UVTcp, write, const String &data) {
        TcpResourceData *resource_data = FETCH_RESOURCE(this_, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();
        write_req_t *req;
        req = new write_req_t();
        req->buf.base = new char[data.size()];
        req->buf.len = data.size();
        memcpy((void *) req->buf.base, data.c_str(), data.size());        
        return uv_write((uv_write_t *) req, (uv_stream_t *) tcp_handle, &req->buf, 1, onwrite) == 0;
    }
    
    void uvExtension::_initUVTcpClass() {
        HHVM_ME(UVTcp, __construct);
        HHVM_ME(UVTcp, __destruct);        
        HHVM_ME(UVTcp, listen);
        HHVM_ME(UVTcp, accept);
        HHVM_ME(UVTcp, close);
        HHVM_ME(UVTcp, setCallback);
        HHVM_ME(UVTcp, write);
    }
}