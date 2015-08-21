#include "uv_udp.h"

namespace HPHP {
    
    UVUdpData::~UVUdpData(){
        sweep();
    }
    
    void UVUdpData::sweep(){
        recvCallback.releaseForSweep();
        sendCallback.releaseForSweep();
        errorCallback.releaseForSweep();
        release();
    }
    
    void UVUdpData::release(){
        if(udp_handle){
            releaseHandle(udp_handle);
            delete udp_handle;
            udp_handle = NULL;
        }
    }

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
    }
    
    ALWAYS_INLINE uv_udp_ext_t *initUVUdpObject(ObjectData *objectData, uv_loop_t *loop) {
        auto* data = Native::data<UVUdpData>(objectData);
        data->udp_handle = new uv_udp_ext_t();
        data->udp_handle->udp_object_data = objectData;
        uv_udp_init(loop, data->udp_handle);
        return data->udp_handle;
    }    

    static void send_cb(uv_udp_send_t* sr, int status) {
        send_req_t *req = (send_req_t *) sr;
        uv_udp_ext_t *udp_handle = (uv_udp_ext_t *) req->handle;
        auto* data = Native::data<UVUdpData>(udp_handle->udp_object_data);
        auto callback = data->sendCallback;
        if(!callback.isNull()){
            vm_call_user_func(callback, 
                make_packed_array(
                    udp_handle->udp_object_data,
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

    static void recv_cb(uv_udp_ext_t* udp_handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned int flags) {
        auto* data = Native::data<UVUdpData>(udp_handle->udp_object_data);
        auto recvCallback = data->recvCallback;
        auto errorCallback = data->errorCallback;
        if(nread > 0){
            if(!recvCallback.isNull()){
                vm_call_user_func(recvCallback, 
                    make_packed_array(
                        udp_handle->udp_object_data,
                        sock_addr((struct sockaddr *) addr),
                        sock_port((struct sockaddr *) addr),
                        String(buf->base, nread, CopyString),
                        (int64_t) flags
                    )
                );
            }
        }
        else{
            if(!errorCallback.isNull()){
                vm_call_user_func(errorCallback, make_packed_array(udp_handle->udp_object_data, nread, (int64_t) flags));
            }
        }
        delete buf->base;
    }
    
    static void close_cb(uv_handle_t* handle) {
       releaseHandle((uv_udp_ext_t *) handle);    
    }
    
    static void HHVM_METHOD(UVUdp, __construct, const Object &loop) {
        auto* loop_data = Native::data<UVLoopData>(loop.get());
        SET_LOOP(this_, loop, s_uvudp);
        initUVUdpObject(this_, loop_data->loop);
    }
    
    static int64_t HHVM_METHOD(UVUdp, bind, const String &host, int64_t port) {
        int64_t ret;        
        struct sockaddr_in addr; 
        auto* data = Native::data<UVUdpData>(this_);
        
        if((ret = uv_ip4_addr(host.c_str(), port&0xffff, &addr)) != 0){
            return ret;
        }
        
        if((ret = uv_udp_bind(data->udp_handle, (const struct sockaddr*) &addr, 0)) != 0){
            return ret;
        }
        
        data->udp_handle->flag |= UV_UDP_HANDLE_START;
        return ret;
    }
    
    static void HHVM_METHOD(UVUdp, close) {
        auto* data = Native::data<UVUdpData>(this_);
        uv_close((uv_handle_t *) data->udp_handle, close_cb);
    }
        
   static int64_t HHVM_METHOD(UVUdp, setCallback, const Variant &onRecvCallback, const Variant &onSendCallback, const Variant &onErrorCallback) {
        int64_t ret;
        auto* data = Native::data<UVUdpData>(this_);
        if((ret = uv_udp_recv_start((uv_udp_t *) data->udp_handle, alloc_cb, (uv_udp_recv_cb) recv_cb)) == 0) {
            data->recvCallback = onRecvCallback;
            data->sendCallback = onSendCallback;
            data->errorCallback = onErrorCallback;
            data->udp_handle->flag |= (UV_UDP_HANDLE_START|UV_UDP_READ_START|UV_UDP_HANDLE_INTERNAL_REF);
            this_->incRefCount();
        }
        return ret;
    }
    
    static String HHVM_METHOD(UVUdp, getSockname) {
        struct sockaddr addr;
        int addrlen;
        auto* data = Native::data<UVUdpData>(this_);
        
        if(data->udp_handle->sockPort == 0){
            addrlen = sizeof addr;
            if(uv_udp_getsockname(data->udp_handle, &addr, &addrlen)){
                return String();
            }
            data->udp_handle->sockAddr = sock_addr(&addr);
            data->udp_handle->sockPort = sock_port(&addr);
        }
        
        return data->udp_handle->sockAddr;
    }
    
    static int64_t HHVM_METHOD(UVUdp, getSockport) {
        auto* data = Native::data<UVUdpData>(this_);
        struct sockaddr addr;
        int addrlen;

        if(data->udp_handle->sockPort == 0){
            addrlen = sizeof addr;
            if(uv_udp_getsockname(data->udp_handle, &addr, &addrlen)){
                return -1;
            }
            data->udp_handle->sockAddr = sock_addr(&addr);
            data->udp_handle->sockPort = sock_port(&addr);
        }
        
        return data->udp_handle->sockPort;
    }    
    
    static int64_t HHVM_METHOD(UVUdp, sendTo, const String &dest, int64_t port, const String &message) {
        auto* data = Native::data<UVUdpData>(this_);
        int64_t ret;
        send_req_t *req;
        req = new send_req_t();
        req->buf.base = new char[message.size()];
        req->buf.len = message.size();
        memcpy((void *) req->buf.base, message.c_str(), message.size());
        if((ret = uv_ip4_addr(dest.c_str(), port, &req->addr)) != 0){
            return ret;
        }
        return uv_udp_send(req, data->udp_handle, &req->buf, 1, (const struct sockaddr *) &req->addr, send_cb);
    }
    
    void uvExtension::_initUVUdpClass() {
        HHVM_ME(UVUdp, __construct);
        HHVM_ME(UVUdp, bind);
        HHVM_ME(UVUdp, close);
        HHVM_ME(UVUdp, setCallback);
        HHVM_ME(UVUdp, sendTo);
        HHVM_ME(UVUdp, getSockname);
        HHVM_ME(UVUdp, getSockport);
        Native::registerNativeDataInfo<UVUdpData>(s_uvudp.get());
    }
}