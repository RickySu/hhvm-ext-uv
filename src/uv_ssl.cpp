#include <openssl/ssl.h>
#include "ext.h"
#include "hphp/runtime/base/thread-init-fini.h"
#include "uv_tcp.h"

#ifdef OPENSSL_FOUND
    #define check_ssl_support()
#else
    #define check_ssl_support() raise_error("please recompile hhvm libuv extension with opeensl support.")
#endif

namespace HPHP {

    typedef struct {
        const SSL_METHOD *ssl_method;
        SSL_CTX** ctx;
        int nctx;
        SSL* ssl;
        BIO* read_bio;
        BIO* write_bio;
    } ssl_ext_t;

    typedef struct uv_ssl_ext_s:public uv_tcp_ext_t{
        ssl_ext_t ssl;
    } uv_ssl_ext_t;
    
    ALWAYS_INLINE ssl_ext_t *fetchSSLResource(const Object &obj){
        InternalResourceData *resource_data = FETCH_RESOURCE(obj, InternalResourceData, s_uvssl);
        uv_ssl_ext_t *ssl_handle = (uv_ssl_ext_t *) resource_data->getInternalResourceData();
        return (ssl_ext_t *) &ssl_handle->ssl;
    }
        
    static int sni_cb(SSL *s, int *ad, void *arg) {
        Variant result;
        int64_t n;
        ObjectData *object_data = (ObjectData *)arg;
        ssl_ext_t *ssl = fetchSSLResource(object_data);
        auto sslServerNameCallback = object_data->o_get("sslServerNameCallback", false, s_uvssl);
        const char *servername = SSL_get_servername(s, TLSEXT_NAMETYPE_host_name);
        if(servername != NULL && !sslServerNameCallback.isNull()){
            result = vm_call_user_func(sslServerNameCallback, make_packed_array(StringData::Make(servername, CopyString)));
            if(result.isInteger()){
                n = result.toInt64Val();
                if(n>=0 && n<ssl->nctx){
                    SSL_set_SSL_CTX(s, ssl->ctx[n]);
                }
            }
        }
        return SSL_TLSEXT_ERR_OK;
    }
    
    static void alloc_cb(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
        buf->base = new char[suggested_size];
        buf->len = suggested_size;
    }

    int64_t write_bio_to_socket(uv_ssl_ext_t *ssl_handle){
        char buf[1024];
        int hasread, ret;
        while(true){
            hasread  = BIO_read(ssl_handle->ssl.write_bio, buf, sizeof(buf));
            if(hasread <= 0){
                return 0;
            }
            ret = tcp_write_raw((uv_stream_t *) ssl_handle, buf, hasread);
            if(ret != 0){
                break;
            }
        }
        return ret;
    }    

    static void read_cb(uv_ssl_ext_t *ssl_handle, ssize_t nread, const uv_buf_t* buf) {
        auto readCallback = ssl_handle->tcp_object_data->o_get("readCallback", false, s_uvssl);
        auto errorCallback = ssl_handle->tcp_object_data->o_get("errorCallback", false, s_uvssl);
        auto sslHandshakeCallback = ssl_handle->tcp_object_data->o_get("sslHandshakeCallback", false, s_uvssl);
        char read_buf[1024];
        int size, err, ret;
        if(nread > 0){
            BIO_write(ssl_handle->ssl.read_bio, buf->base, nread);
            if (!SSL_is_init_finished(ssl_handle->ssl.ssl)) {
                ret = SSL_do_handshake(ssl_handle->ssl.ssl);
                write_bio_to_socket(ssl_handle);
                if(ret != 1){
                    err = SSL_get_error(ssl_handle->ssl.ssl, ret);
                    if (err == SSL_ERROR_WANT_READ) {
                        return;
                    }
                    else if(err == SSL_ERROR_WANT_WRITE){
                        write_bio_to_socket(ssl_handle);
                    }
                }
                else{
                    if(!sslHandshakeCallback.isNull()){
                        vm_call_user_func(sslHandshakeCallback, make_packed_array(ssl_handle->tcp_object_data));
                    }                
                }
                return;
            }
            ssl_handle->flag |= UV_TCP_WRITE_CALLBACK_ENABLE;
            size = SSL_read(ssl_handle->ssl.ssl, read_buf, sizeof(read_buf));
            if(size > 0 && !readCallback.isNull()) {
                vm_call_user_func(readCallback, make_packed_array(ssl_handle->tcp_object_data, StringData::Make(read_buf, size, CopyString)));
            }
        }
        else if(nread<0){
            if(!errorCallback.isNull()){
                vm_call_user_func(errorCallback, make_packed_array(ssl_handle->tcp_object_data, nread));
            }
            tcp_close_socket(ssl_handle);
        }
        delete buf->base;
    }                                 
    
    static void HHVM_METHOD(UVSSL, __construct, int64_t method, int64_t nContexts){
        check_ssl_support();
        ssl_ext_t *ssl;
        initUVTcpObject(this_, uv_default_loop(), sizeof(uv_ssl_ext_t));
        ssl = fetchSSLResource(this_);
        switch(method){
            case 0:  //SSL_METHOD_SSLV2
#ifdef OPENSSL_NO_SSL2
                ssl->ssl_method = SSLv3_method();
                break;
#else
                ssl->ssl_method = SSLv2_method();
                break;
#endif
            case 1:  //SSL_METHOD_SSLV3
                ssl->ssl_method = SSLv3_method();
                break;
            case 2:  //SSL_METHOD_SSLV23
                ssl->ssl_method = SSLv23_method();
                break;
            case 3:  //SSL_METHOD_TLSV1
                ssl->ssl_method = TLSv1_method();
                break;
            case 4:  //SSL_METHOD_TLSV1_1
                ssl->ssl_method = TLSv1_1_method();
                break;
            case 5: //SSL_METHOD_TLSV1_2
                ssl->ssl_method = TLSv1_2_method();
                break;
            default:
                ssl->ssl_method = TLSv1_1_method();
                break;
        }
        ssl->ctx = new SSL_CTX *[nContexts];
        ssl->nctx = nContexts;
        for(int i = 0; i < nContexts; i++){
            ssl->ctx[i] = SSL_CTX_new(ssl->ssl_method);
            SSL_CTX_set_session_cache_mode(ssl->ctx[i], SSL_SESS_CACHE_BOTH);
        }
#ifndef OPENSSL_NO_TLSEXT        
        if(method > 2){ // tls
            SSL_CTX_set_tlsext_servername_callback(ssl->ctx[0], sni_cb);
            SSL_CTX_set_tlsext_servername_arg(ssl->ctx[0], this_);
        }
#endif        
    }
    
    static void HHVM_METHOD(UVSSL, __destruct){
        ssl_ext_t *ssl = fetchSSLResource(this_);
        if(ssl->ctx){
        //use SSL_free instead
/*
            if(ssl->read_bio){
                BIO_free(ssl->read_bio);                
            }
            if(ssl->write_bio){
                BIO_free(ssl->write_bio);
            }*/
            if(ssl->ssl){
                SSL_free(ssl->ssl);
            }
            for(int i=0;i<ssl->nctx;i++){
                SSL_CTX_free(ssl->ctx[i]);
            }
            delete ssl->ctx;
            ssl->ctx = NULL;
        }
        HHVM_MN(UVTcp, __destruct)(this_);
    }

    static bool HHVM_METHOD(UVSSL, setCert, const String &cert, int64_t n){
        bool result;
        X509 *pcert;
        BIO *cert_bio;
        ssl_ext_t *ssl = fetchSSLResource(this_);        
        if(n<0 || n>=ssl->nctx){
            return false;
        }
        cert_bio = BIO_new(BIO_s_mem());
        if(BIO_write(cert_bio, (void *)cert.data(), cert.size()) <= 0){
            return false;
        }

        pcert = PEM_read_bio_X509_AUX(cert_bio, NULL, NULL, NULL);
        BIO_free(cert_bio);
        
        if(pcert == NULL){
            return false;
        }
        result = SSL_CTX_use_certificate(ssl->ctx[n], pcert);

        X509_free(pcert);
        return result;
    }
    
    static bool HHVM_METHOD(UVSSL, setPrivateKey, const String &privateKey, int64_t n){
        bool result;
        EVP_PKEY *pkey;
        BIO *key_bio;    
        ssl_ext_t *ssl = fetchSSLResource(this_);
        if(n<0 || n>=ssl->nctx){
            return false;
        }
        key_bio = BIO_new(BIO_s_mem());        
        if(BIO_write(key_bio, (void *)privateKey.data(), privateKey.size()) <= 0){
            return false;
        }

        pkey = PEM_read_bio_PrivateKey(key_bio, NULL, NULL, NULL);
        BIO_free(key_bio);
        
        if(pkey == NULL){
            return false;
        }
        
        result = SSL_CTX_use_PrivateKey(ssl->ctx[n], pkey);

        EVP_PKEY_free(pkey);
        
        return result;
    }    
    
    static Object HHVM_METHOD(UVSSL, accept){
        ssl_ext_t *server_ssl, *ssl;
        Object obj = make_accepted_uv_tcp_object(this_, "UVSSL", sizeof(uv_ssl_ext_t));
        InternalResourceData *resource_data = FETCH_RESOURCE(obj, InternalResourceData, s_uvssl);
        uv_ssl_ext_t *ssl_handle = (uv_ssl_ext_t *) resource_data->getInternalResourceData();
        uv_read_start((uv_stream_t *) ssl_handle, alloc_cb, (uv_read_cb) read_cb);
        server_ssl = fetchSSLResource(this_);                
        ssl = fetchSSLResource(obj);
        ssl->ssl = SSL_new(server_ssl->ctx[0]);
        ssl->read_bio = BIO_new(BIO_s_mem());
        ssl->write_bio = BIO_new(BIO_s_mem());        
        SSL_set_bio(ssl->ssl, ssl->read_bio, ssl->write_bio);
        SSL_set_accept_state(ssl->ssl);
        write_bio_to_socket(ssl_handle);
        ssl_handle->flag |= (UV_TCP_HANDLE_START|UV_TCP_READ_START);
        return obj;
    }

    static int64_t HHVM_METHOD(UVSSL, setCallback, const Variant &onReadCallback, const Variant &onWriteCallback, const Variant &onErrorCallback) {
        this_->o_set("readCallback", onReadCallback, s_uvssl);
        this_->o_set("writeCallback", onWriteCallback, s_uvssl);
        this_->o_set("errorCallback", onErrorCallback, s_uvssl);        
        return 0;
    }
    
    static int64_t HHVM_METHOD(UVSSL, write, const String &data) {
        ssl_ext_t *ssl;
        int size;
        InternalResourceData *resource_data = FETCH_RESOURCE(this_, InternalResourceData, s_uvssl);
        uv_ssl_ext_t *ssl_handle = (uv_ssl_ext_t *) resource_data->getInternalResourceData();
        ssl = &ssl_handle->ssl;
        size = SSL_write(ssl->ssl, data.data(), data.size()); 
        if(size > 0){
            return write_bio_to_socket(ssl_handle);                        
        }
        return 0;
    }

    static void client_connection_cb(uv_connect_t* req, int status) {
        uv_ssl_ext_t *ssl_handle = (uv_ssl_ext_t *) req->handle;
        ssl_ext_t *ssl = &ssl_handle->ssl;
        auto callback = ssl_handle->tcp_object_data->o_get("connectCallback", false, s_uvssl);
        uv_read_start((uv_stream_t *) ssl_handle, alloc_cb, (uv_read_cb) read_cb);
        ssl->ssl = SSL_new(ssl->ctx[0]);
        ssl->read_bio = BIO_new(BIO_s_mem());
        ssl->write_bio = BIO_new(BIO_s_mem());        
        SSL_set_bio(ssl->ssl, ssl->read_bio, ssl->write_bio);
        SSL_set_connect_state(ssl->ssl);
        SSL_connect(ssl->ssl);
        write_bio_to_socket(ssl_handle);
        if(!callback.isNull()){
            vm_call_user_func(callback, make_packed_array(ssl_handle->tcp_object_data, status));
        }    
    }   
    
    static int64_t HHVM_METHOD(UVSSL, connect, const String &host, int64_t port, const Variant &onConnectCallback) {
        int64_t ret;
        struct sockaddr_in addr;
        InternalResourceData *resource_data = FETCH_RESOURCE(this_, InternalResourceData, s_uvssl);
        uv_ssl_ext_t *ssl_handle = (uv_ssl_ext_t *) resource_data->getInternalResourceData();
        
        if((ret = uv_ip4_addr(host.c_str(), port&0xffff, &addr)) != 0){
            return ret;
        }
        if((ret = uv_tcp_connect(&ssl_handle->connect_req, (uv_tcp_t *) ssl_handle, (const struct sockaddr*) &addr, client_connection_cb)) != 0){
            return ret;
        }
        
        this_->o_set("connectCallback", onConnectCallback, s_uvssl);
        ssl_handle->flag |= UV_TCP_HANDLE_START;
        return ret;
    }
    
    void uvExtension::_initUVSSLClass() {
        HHVM_ME(UVSSL, accept);
        HHVM_ME(UVSSL, setCallback);
        HHVM_ME(UVSSL, write);
        HHVM_ME(UVSSL, __construct);
        HHVM_ME(UVSSL, __destruct);
        HHVM_ME(UVSSL, setCert);
        HHVM_ME(UVSSL, setPrivateKey);
        HHVM_ME(UVSSL, connect);
    }
}
