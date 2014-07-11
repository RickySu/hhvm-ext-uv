#include "ext.h"
#include "hphp/runtime/base/thread-init-fini.h"
#define UV_UDP_HANDLE_INTERNAL_REF 1
#define UV_UDP_HANDLE_START (1<<1)
#define UV_UDP_READ_START (1<<2)

namespace HPHP {

    typedef struct uv_udp_ext_s:public uv_udp_t{
        uint flag;
        uv_connect_t connect_req;
        uv_shutdown_t shutdown_req;
        ObjectData *udp_object_data;
        StringData *sockAddr;
        int sockPort;
    } uv_udp_ext_t;
    
    typedef struct send_req_s: public uv_udp_send_t {
        uv_buf_t buf;
        struct sockaddr_in addr;
    } send_req_t;

    ALWAYS_INLINE void releaseHandle(uv_udp_ext_t *handle) {
        if(handle->flag & UV_UDP_READ_START){
            handle->flag &= ~UV_UDP_READ_START;        
            uv_udp_recv_stop((uv_udp_t *) handle);
        }
        
        if(handle->flag & UV_UDP_HANDLE_START){
            handle->flag &= ~UV_UDP_HANDLE_START;        
            uv_unref((uv_handle_t *) handle);
        }    
        
        if(handle->flag & UV_UDP_HANDLE_INTERNAL_REF){
            handle->flag &= ~UV_UDP_HANDLE_INTERNAL_REF;
            ((uv_udp_ext_t *) handle)->udp_object_data->decRefAndRelease();
        }
        
        if(handle->sockPort != -1){
            handle->sockPort = -1;
            handle->sockAddr->decRefAndRelease();
            handle->sockAddr = NULL;
        }
    }
    
    ALWAYS_INLINE uv_udp_ext_t *initUVUdpObject(const Object &object, uv_loop_t *loop) {
        Resource resource(NEWOBJ(UdpResourceData(sizeof(uv_udp_ext_t))));
        SET_RESOURCE(object, resource, s_uvudp);
        UdpResourceData *udp_resource_data = FETCH_RESOURCE(object, UdpResourceData, s_uvudp);
        uv_udp_ext_t *udp_handle = (uv_udp_ext_t *) udp_resource_data->getInternalResourceData();
        udp_handle->flag = 0;
        udp_handle->sockAddr = NULL;
        udp_handle->sockPort = -1;                                
        udp_handle->udp_object_data = object.get();
        uv_udp_init(loop, udp_handle);
        return udp_handle;
    }    

    static void send_cb(uv_udp_send_t* sr, int status) {
        send_req_t *req = (send_req_t *) sr;
        UdpResourceData *udp_resource_data = FETCH_RESOURCE(((uv_udp_ext_t *) req->handle)->udp_object_data, UdpResourceData, s_uvudp);
        Variant callback = udp_resource_data->getSendCallback();        
        if(!callback.isNull()){
            vm_call_user_func(callback, 
                make_packed_array(
                    ((uv_udp_ext_t *) req->handle)->udp_object_data,
                    sock_addr((struct sockaddr *) &req->addr),
                    sock_port((struct sockaddr *) &req->addr),
                    status
                )
            );        
        }
        delete req->buf.base;
        delete req;
    }
    
    static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
        buf->base = new char[suggested_size];
        buf->len = suggested_size;
    }

    static void recv_cb(uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned int flags) {
        UdpResourceData *udp_resource_data = FETCH_RESOURCE(((uv_udp_ext_t *) handle)->udp_object_data, UdpResourceData, s_uvudp);
        
        if(nread > 0){
            if(!udp_resource_data->getRecvCallback().isNull()){
                vm_call_user_func(udp_resource_data->getRecvCallback(), 
                    make_packed_array(
                        ((uv_udp_ext_t *) handle)->udp_object_data,
                        sock_addr((struct sockaddr *) addr),
                        sock_port((struct sockaddr *) addr),
                        StringData::Make(buf->base, nread, CopyString),
                        (int64_t) flags
                    )
                );
            }
        }
        else{
            if(!udp_resource_data->getErrorCallback().isNull()){
                vm_call_user_func(udp_resource_data->getErrorCallback(), make_packed_array(((uv_udp_ext_t *) handle)->udp_object_data, nread, (int64_t) flags));
            }
        }
        delete buf->base;
    }
    
    static void close_cb(uv_handle_t* handle) {
       releaseHandle((uv_udp_ext_t *) handle);    
    }
    
    static void HHVM_METHOD(UVUdp, __construct, const Object &o_loop) {
        InternalResourceData *loop_resource_data = FETCH_RESOURCE(o_loop, InternalResourceData, s_uvloop);
        initUVUdpObject(this_, (uv_loop_t *) loop_resource_data->getInternalResourceData());
    }
    
    static void HHVM_METHOD(UVUdp, __destruct) {
        UdpResourceData *resource_data = FETCH_RESOURCE(this_, UdpResourceData, s_uvudp);
        uv_udp_ext_t *udp_handle = (uv_udp_ext_t *) resource_data->getInternalResourceData();
        releaseHandle(udp_handle);    
    }
    
    static bool HHVM_METHOD(UVUdp, bind, const String &host, int64_t port) {
        struct sockaddr_in addr; 
        UdpResourceData *resource_data = FETCH_RESOURCE(this_, UdpResourceData, s_uvudp);
        uv_udp_ext_t *udp_handle = (uv_udp_ext_t *) resource_data->getInternalResourceData();        
        
        if(uv_ip4_addr(host.c_str(), port&0xffff, &addr)){
            return false;
        }
        
        if(uv_udp_bind(udp_handle, (const struct sockaddr*) &addr, 0)){
            return false;
        }
        
        udp_handle->flag |= UV_UDP_HANDLE_START;
        return true;
    }
    
    static void HHVM_METHOD(UVUdp, close) {
        UdpResourceData *resource_data = FETCH_RESOURCE(this_, UdpResourceData, s_uvudp);
        uv_udp_ext_t *udp_handle = (uv_udp_ext_t *) resource_data->getInternalResourceData();
        uv_close((uv_handle_t *) udp_handle, close_cb);
    }
        
   static void HHVM_METHOD(UVUdp, setCallback, const Variant &onRecvCallback, const Variant &onSendCallback, const Variant &onErrorCallback) {
        UdpResourceData *resource_data = FETCH_RESOURCE(this_, UdpResourceData, s_uvudp);
        uv_udp_ext_t *udp_handle = (uv_udp_ext_t *) resource_data->getInternalResourceData();        
        if(uv_udp_recv_start((uv_udp_t *) udp_handle, alloc_cb, recv_cb) != 0) {
            return;
        }
        resource_data->setRecvCallback(onRecvCallback);
        resource_data->setSendCallback(onSendCallback);
        resource_data->setErrorCallback(onErrorCallback);
        udp_handle->flag |= (UV_UDP_HANDLE_START|UV_UDP_READ_START);
    }
    
    static String HHVM_METHOD(UVUdp, getSockname) {
        struct sockaddr addr;
        int addrlen;
        UdpResourceData *resource_data = FETCH_RESOURCE(this_, UdpResourceData, s_uvudp);
        uv_udp_ext_t *udp_handle = (uv_udp_ext_t *) resource_data->getInternalResourceData();
        
        if(udp_handle->sockPort == -1){
            addrlen = sizeof addr;
            if(uv_udp_getsockname(udp_handle, &addr, &addrlen)){
                return String();
            }
            udp_handle->sockAddr = sock_addr(&addr);
            udp_handle->sockPort = sock_port(&addr);
            udp_handle->sockAddr->incRefCount();
        }
        
        return udp_handle->sockAddr;
    }
    
    static int64_t HHVM_METHOD(UVUdp, getSockport) {
        struct sockaddr addr;
        int addrlen;
        UdpResourceData *resource_data = FETCH_RESOURCE(this_, UdpResourceData, s_uvudp);
        uv_udp_ext_t *udp_handle = (uv_udp_ext_t *) resource_data->getInternalResourceData();

        if(udp_handle->sockPort == -1){
            addrlen = sizeof addr;
            if(uv_udp_getsockname(udp_handle, &addr, &addrlen)){
                return -1;
            }
            udp_handle->sockAddr = sock_addr(&addr);
            udp_handle->sockPort = sock_port(&addr);
            udp_handle->sockAddr->incRefCount();
        }
        
        return udp_handle->sockPort;
    }    
    
    static bool HHVM_METHOD(UVUdp, sendTo, const String &dest, int64_t port, const String &data) {
        UdpResourceData *resource_data = FETCH_RESOURCE(this_, UdpResourceData, s_uvudp);
        uv_udp_ext_t *udp_handle = (uv_udp_ext_t *) resource_data->getInternalResourceData();        
        send_req_t *req;
        req = new send_req_t();
        req->buf.base = new char[data.size()];
        req->buf.len = data.size();
        memcpy((void *) req->buf.base, data.c_str(), data.size());
        uv_ip4_addr(dest.c_str(), port, &req->addr);
        return uv_udp_send(req, udp_handle, &req->buf, 1, (const struct sockaddr *) &req->addr, send_cb) == 0;
    }
    
    void uvExtension::_initUVUdpClass() {
        HHVM_ME(UVUdp, __construct);
        HHVM_ME(UVUdp, __destruct);
        HHVM_ME(UVUdp, bind);
        HHVM_ME(UVUdp, close);
        HHVM_ME(UVUdp, setCallback);
        HHVM_ME(UVUdp, sendTo);
        HHVM_ME(UVUdp, getSockname);
        HHVM_ME(UVUdp, getSockport);
    }
}