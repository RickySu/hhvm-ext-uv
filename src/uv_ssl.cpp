#include "uv_ssl.h"

#ifdef OPENSSL_FOUND
    #define check_ssl_support()
#else
    #define check_ssl_support() raise_error("please recompile hhvm libuv extension with opeensl support.")
#endif

namespace HPHP {
    typedef struct uv_getaddrinfo_ext_s: public uv_getaddrinfo_t{
        uv_ssl_ext_t *ssl_handle;
    } uv_getaddrinfo_ext_t;
            
    ALWAYS_INLINE uv_loop_t *fetchLoop(ObjectData *this_){
        auto v_loop = this_->o_get(s_internal_loop, false, s_uvtcp);
        if(v_loop.isNull()){
            return uv_default_loop();
        }
        Object loop = v_loop.toObject();
        auto* loop_data = Native::data<UVLoopData>(loop.get());
        return loop_data->loop;    
    }

    static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx) {
        int ret = preverify_ok;
        X509_STORE_CTX_get_current_cert(ctx);
        int err = X509_STORE_CTX_get_error(ctx);
        int depth = X509_STORE_CTX_get_error_depth(ctx);
        SSL *ssl = (SSL*) X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());

        X509_STORE_CTX_get_current_cert(ctx);        
        ssl = (SSL *) X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
        ret = preverify_ok;
        err = X509_STORE_CTX_get_error(ctx);
        depth = X509_STORE_CTX_get_error_depth(ctx);
        X509_STORE_CTX_set_error(ctx, err);
        printf("preverify: %d %d %s\n", err, X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT, X509_verify_cert_error_string(err));

        if(!preverify_ok){
            printf("error\n");
            return preverify_ok;
        }
        
        return 1;
    }

    static void releaseHook(UVTcpData *data){
        uv_ssl_ext_t *ssl_handle = fetchSSLHandle(data);
        if(ssl_handle->sslResource.ctx){
            if(ssl_handle->sslResource.ssl){
                SSL_free(ssl_handle->sslResource.ssl);
            }
            for(int i=0;i<ssl_handle->sslResource.nctx;i++){
                SSL_CTX_free(ssl_handle->sslResource.ctx[i]);
            }
            delete ssl_handle->sslResource.ctx;
            ssl_handle->sslResource.ctx = NULL;
        }
        ssl_handle->sslHandshakeCallback.unset();
        ssl_handle->sslServerNameCallback.unset();
        ssl_handle->sniConnectHostname.clear();
    }
    
    static void gcHook(UVTcpData *data){
        uv_ssl_ext_t *ssl_handle = fetchSSLHandle(data);
        if(data->tcp_handle){
            delete ssl_handle;
            data->tcp_handle = NULL;
        }
    }
    
    static int sni_cb(SSL *s, int *ad, void *arg) {
        Variant result;
        int64_t n;
        ObjectData *objectData = (ObjectData *)arg;
        auto* data = Native::data<UVTcpData>(objectData);
        uv_ssl_ext_t *ssl_handle = fetchSSLHandle(data);
        auto sslServerNameCallback = ssl_handle->sslServerNameCallback;
        const char *servername = SSL_get_servername(s, TLSEXT_NAMETYPE_host_name);
        if(servername != NULL && !sslServerNameCallback.isNull()){
            result = vm_call_user_func(sslServerNameCallback, make_packed_array(String(servername, CopyString)));
            if(result.isInteger()){
                n = result.toInt64Val();
                if(n>=0 && n<ssl_handle->sslResource.nctx){
                    SSL_set_SSL_CTX(s, ssl_handle->sslResource.ctx[n]);
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
        char buf[512];
        int hasread, ret;
        while(true){
            hasread  = BIO_read(ssl_handle->sslResource.write_bio, buf, sizeof(buf));
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

    ALWAYS_INLINE bool handleHandshake(uv_ssl_ext_t *ssl_handle, ssize_t nread, const uv_buf_t* buf){
        int ret, err;
        auto sslHandshakeCallback = ssl_handle->sslHandshakeCallback;

        if(SSL_is_init_finished(ssl_handle->sslResource.ssl)) {
            ssl_handle->flag |= UV_TCP_WRITE_CALLBACK_ENABLE;
            return true;
        }
        
        ret = SSL_do_handshake(ssl_handle->sslResource.ssl);
        write_bio_to_socket(ssl_handle);

        if(ret != 1){
            err = SSL_get_error(ssl_handle->sslResource.ssl, ret);
            switch(err){
                case SSL_ERROR_WANT_READ:
                    break;
                case SSL_ERROR_WANT_WRITE:
                    write_bio_to_socket(ssl_handle);
                    break;
                default:
                    printf("err:%d\n", err);
                    printf("message:%s\n", X509_verify_cert_error_string(SSL_get_verify_result(ssl_handle->sslResource.ssl)));
            }
            return false;
        }

        if(!sslHandshakeCallback.isNull()){
            vm_call_user_func(sslHandshakeCallback, make_packed_array(ssl_handle->tcp_object_data));
        }                
        return true;
    }
    
    static void read_cb(uv_ssl_ext_t *ssl_handle, ssize_t nread, const uv_buf_t* buf) {
        auto* data = Native::data<UVTcpData>(ssl_handle->tcp_object_data);
        auto readCallback = data->readCallback;
        auto errorCallback = data->errorCallback;
        char read_buf[256];
        int size, read_buf_index = 0;
        
        if(nread<0){
            if(!errorCallback.isNull()){
                vm_call_user_func(errorCallback, make_packed_array(ssl_handle->tcp_object_data, nread));
            }
            tcp_close_socket(ssl_handle);
            delete buf->base;
            return;
        }
        
        BIO_write(ssl_handle->sslResource.read_bio, buf->base, nread);
            
        if(!handleHandshake(ssl_handle, nread, buf)){
            delete buf->base;
            return;
        }            

        while(true){
            size = SSL_read(ssl_handle->sslResource.ssl, &read_buf[read_buf_index], sizeof(read_buf) - read_buf_index);
            if(size > 0){
                read_buf_index+=size;
            }
            if(size <= 0 || read_buf_index >= sizeof(read_buf)){
                if(!readCallback.isNull()){
                    vm_call_user_func(readCallback, make_packed_array(ssl_handle->tcp_object_data, String(read_buf, read_buf_index, CopyString)));
                }
                read_buf_index = 0;
            }
            if(size <= 0){
                break;
            }
        }

        delete buf->base;
    }
    
    static void HHVM_METHOD(UVSSL, __construct, const Variant &v_loop, int64_t method, int64_t nContexts){
        check_ssl_support();
        auto* data = Native::data<UVTcpData>(this_);
        if(v_loop.isNull()){
            initUVTcpObject(this_, uv_default_loop(), new uv_ssl_ext_t());
        }
        else{
            Object loop = v_loop.toObject();
            checkUVLoopInstance(loop, 1, s_uvssl, StaticString("__construct"));
            auto* loop_data = Native::data<UVLoopData>(loop.get());
            SET_LOOP(this_, loop, s_uvtcp);
            initUVTcpObject(this_, loop_data->loop, new uv_ssl_ext_t());
        }
        uv_ssl_ext_t *ssl_handle = fetchSSLHandle(data);
        initSSLHandle(ssl_handle);
        data->releaseHook = releaseHook;
        data->gcHook = gcHook;
        switch(method){
            case SSL_METHOD_SSLV23:
                ssl_handle->sslResource.ssl_method = SSLv23_method();
                break;
            case SSL_METHOD_SSLV2:
#ifndef OPENSSL_NO_SSL2
                ssl_handle->sslResource.ssl_method = SSLv2_method();
                break;
#else
    #ifndef OPENSSL_NO_SSL3_METHOD
                ssl_handle->sslResource.ssl_method = SSLv3_method();
                break;
    #endif
#endif
            case SSL_METHOD_SSLV3:
#ifndef OPENSSL_NO_SSL3_METHOD
                ssl_handle->sslResource.ssl_method = SSLv3_method();
                break;
#endif
            case SSL_METHOD_TLSV1:
                ssl_handle->sslResource.ssl_method = TLSv1_method();
                break;
            case SSL_METHOD_TLSV1_1:
                ssl_handle->sslResource.ssl_method = TLSv1_1_method();
                break;
            case SSL_METHOD_TLSV1_2:
                ssl_handle->sslResource.ssl_method = TLSv1_2_method();
                break;
            default:
                ssl_handle->sslResource.ssl_method = TLSv1_1_method();
                break;
        }
        ssl_handle->sslResource.ctx = new SSL_CTX *[nContexts];
        ssl_handle->sslResource.nctx = nContexts;
        for(int i = 0; i < nContexts; i++){
            ssl_handle->sslResource.ctx[i] = SSL_CTX_new(ssl_handle->sslResource.ssl_method);
            SSL_CTX_set_session_cache_mode(ssl_handle->sslResource.ctx[i], SSL_SESS_CACHE_BOTH);
        }

#ifndef OPENSSL_NO_TLSEXT
        if(method >= SSL_METHOD_TLSV1){
            SSL_CTX_set_tlsext_servername_callback(ssl_handle->sslResource.ctx[0], sni_cb);
            SSL_CTX_set_tlsext_servername_arg(ssl_handle->sslResource.ctx[0], this_);
        }
#endif
        
    }
    
    static bool HHVM_METHOD(UVSSL, setCert, const String &cert, int64_t n){
        bool result;
        X509 *pcert;
        BIO *cert_bio;
        auto* data = Native::data<UVTcpData>(this_);  
        uv_ssl_ext_t *ssl_handle = fetchSSLHandle(data);
        if(n<0 || n>=ssl_handle->sslResource.nctx){
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
        result = SSL_CTX_use_certificate(ssl_handle->sslResource.ctx[n], pcert);

        X509_free(pcert);
        return result;
    }
    
    static bool HHVM_METHOD(UVSSL, setPrivateKey, const String &privateKey, int64_t n){
        bool result;
        EVP_PKEY *pkey;
        BIO *key_bio;    
        auto* data = Native::data<UVTcpData>(this_);  
        uv_ssl_ext_t *ssl_handle = fetchSSLHandle(data);
        if(n<0 || n>=ssl_handle->sslResource.nctx){
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
        
        result = SSL_CTX_use_PrivateKey(ssl_handle->sslResource.ctx[n], pkey);

        EVP_PKEY_free(pkey);
        
        return result;
    }    
    
    static Object HHVM_METHOD(UVSSL, accept){
        uv_ssl_ext_t *server_ssl_handle, *ssl_handle;
        ObjectData *objectData = make_accepted_uv_tcp_object(this_, "UVSSL", new uv_ssl_ext_t());
        auto* data = Native::data<UVTcpData>(objectData);
        auto* server_data = Native::data<UVTcpData>(this_);
        ssl_handle = fetchSSLHandle(data);
        uv_read_start((uv_stream_t *) ssl_handle, alloc_cb, (uv_read_cb) read_cb);
        server_ssl_handle = fetchSSLHandle(server_data);
        ssl_handle->sslResource.ssl = SSL_new(server_ssl_handle->sslResource.ctx[0]);
        ssl_handle->sslResource.read_bio = BIO_new(BIO_s_mem());
        ssl_handle->sslResource.write_bio = BIO_new(BIO_s_mem());        
        SSL_set_bio(ssl_handle->sslResource.ssl, ssl_handle->sslResource.read_bio, ssl_handle->sslResource.write_bio);
        SSL_set_accept_state(ssl_handle->sslResource.ssl);
        write_bio_to_socket(ssl_handle);
        return Object(objectData);
    }
    
    static void HHVM_METHOD(UVSSL, setSSLHandshakeCallback, const Variant &callback) {
        auto* data = Native::data<UVTcpData>(this_);
        uv_ssl_ext_t *ssl_handle = fetchSSLHandle(data);
        ssl_handle->sslHandshakeCallback = callback;
    }
    
    static void HHVM_METHOD(UVSSL, setSSLServerNameCallback, const Variant &callback) {
        auto* data = Native::data<UVTcpData>(this_);
        uv_ssl_ext_t *ssl_handle = fetchSSLHandle(data);
        ssl_handle->sslServerNameCallback = callback;
    }    
    
    static int64_t HHVM_METHOD(UVSSL, write, const String &message) {
        int size;
        auto* data = Native::data<UVTcpData>(this_);
        uv_ssl_ext_t *ssl_handle = fetchSSLHandle(data);
        size = SSL_write(ssl_handle->sslResource.ssl, message.data(), message.size());
        if(size > 0){
            return write_bio_to_socket(ssl_handle);
        }
        return 0;
    }

    static void client_connection_cb(uv_connect_t* req, int status) {
        uv_ssl_ext_t *ssl_handle = (uv_ssl_ext_t *) req->handle;
        auto* data = Native::data<UVTcpData>(ssl_handle->tcp_object_data);
        auto callback = data->connectCallback;
        
        SSL_CTX_set_verify(ssl_handle->sslResource.ctx[0], SSL_VERIFY_PEER, verify_callback);
        SSL_CTX_set_default_verify_paths(ssl_handle->sslResource.ctx[0]);
        SSL_CTX_set_verify_depth(ssl_handle->sslResource.ctx[0], 9);
        SSL_CTX_set_cipher_list(ssl_handle->sslResource.ctx[0], "DEFAULT");
        
        uv_read_start((uv_stream_t *) ssl_handle, alloc_cb, (uv_read_cb) read_cb);

        ssl_handle->sslResource.ssl = SSL_new(ssl_handle->sslResource.ctx[0]);
        ssl_handle->sslResource.read_bio = BIO_new(BIO_s_mem());
        ssl_handle->sslResource.write_bio = BIO_new(BIO_s_mem());
        

        SSL_set_bio(ssl_handle->sslResource.ssl, ssl_handle->sslResource.read_bio, ssl_handle->sslResource.write_bio);

#ifndef OPENSSL_NO_TLSEXT
        SSL_set_tlsext_host_name(ssl_handle->sslResource.ssl, ssl_handle->sniConnectHostname.c_str());
#endif        
  
        SSL_set_connect_state(ssl_handle->sslResource.ssl);
        SSL_connect(ssl_handle->sslResource.ssl);
        
        write_bio_to_socket(ssl_handle);
        ssl_handle->flag |= (UV_TCP_HANDLE_START|UV_TCP_READ_START);
                    
        if(!callback.isNull()){
            vm_call_user_func(callback, make_packed_array(ssl_handle->tcp_object_data, status));
        }    
    }   

    static void on_addrinfo_resolved(uv_getaddrinfo_ext_t *info, int status, struct addrinfo *res) {
        int64_t ret;
        struct sockaddr_in addr;
        uv_ssl_ext_t *ssl_handle = info->ssl_handle;
        auto callback = Native::data<UVTcpData>(ssl_handle->tcp_object_data)->connectCallback;
        char host[17] = {'\0'};
        if( (ret = status) != 0 || 
            (ret = uv_ip4_name((struct sockaddr_in*) res->ai_addr, host, 16)) != 0 ||
            (ret = uv_ip4_addr(host, ssl_handle->port&0xffff, &addr)) != 0 ||
            (ret = uv_tcp_connect(&ssl_handle->connect_req, ssl_handle, (const struct sockaddr*) &addr, client_connection_cb)) != 0){
            vm_call_user_func(callback, make_packed_array(ssl_handle->tcp_object_data, ret));
            releaseHandle(ssl_handle);
        }
        uv_freeaddrinfo(res);
        delete info;
    }
    
    static int64_t HHVM_METHOD(UVSSL, connect, const String &host, int64_t port, const Variant &onConnectCallback) {
        int64_t ret;
        struct sockaddr_in addr;
        auto* data = Native::data<UVTcpData>(this_);
        uv_ssl_ext_t *ssl_handle = fetchSSLHandle(data);

        ssl_handle->sniConnectHostname = host;
        ssl_handle->port = port;        
        data->connectCallback = onConnectCallback;

        if((ret = uv_ip4_addr(host.c_str(), port&0xffff, &addr)) != 0){
            uv_getaddrinfo_ext_t *addrinfo = new uv_getaddrinfo_ext_t();
            addrinfo->ssl_handle = ssl_handle;
            auto loop_ext = fetchLoop(this_);
            uv_getaddrinfo(loop_ext, addrinfo, (uv_getaddrinfo_cb) on_addrinfo_resolved, host.c_str(), NULL, NULL);
            setSelfReference(data->tcp_handle);
            ssl_handle->flag |= UV_TCP_HANDLE_START;
            return ret;
        }

        if((ret = uv_tcp_connect(&ssl_handle->connect_req, (uv_tcp_t *) data->tcp_handle, (const struct sockaddr*) &addr, client_connection_cb)) != 0){
            return ret;
        }

        setSelfReference(data->tcp_handle);
        ssl_handle->flag |= UV_TCP_HANDLE_START;
        return ret;
    }

    void uvExtension::_initUVSSLClass() {
        SSL_library_init();
        REGISTER_UV_SSL_CONSTANT(SSL_METHOD_SSLV2);
        REGISTER_UV_SSL_CONSTANT(SSL_METHOD_SSLV3);
        REGISTER_UV_SSL_CONSTANT(SSL_METHOD_SSLV23);
        REGISTER_UV_SSL_CONSTANT(SSL_METHOD_TLSV1);
        REGISTER_UV_SSL_CONSTANT(SSL_METHOD_TLSV1_1);
        REGISTER_UV_SSL_CONSTANT(SSL_METHOD_TLSV1_2);
        
        HHVM_ME(UVSSL, accept);
        HHVM_ME(UVSSL, write);
        HHVM_ME(UVSSL, __construct);
        HHVM_ME(UVSSL, setCert);
        HHVM_ME(UVSSL, setPrivateKey);
        HHVM_ME(UVSSL, connect);
        HHVM_ME(UVSSL, setSSLHandshakeCallback);
        HHVM_ME(UVSSL, setSSLServerNameCallback);
    }
}
