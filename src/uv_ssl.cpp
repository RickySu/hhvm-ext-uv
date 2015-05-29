#include "uv_ssl.h"

#ifdef OPENSSL_FOUND
    #define check_ssl_support()
#else
    #define check_ssl_support() raise_error("please recompile hhvm libuv extension with opeensl support.")
#endif

namespace HPHP {
    static int sni_cb(SSL *s, int *ad, void *arg) {
        Variant result;
        int64_t n;
        ObjectData *objectData = (ObjectData *)arg;
        auto* data = Native::data<UVTcpData>(objectData);
        uv_ssl_ext_t *ssl_handle = fetchSSLHandle(data);
        auto sslServerNameCallback = ssl_handle->sslServerNameCallback;
        const char *servername = SSL_get_servername(s, TLSEXT_NAMETYPE_host_name);
        if(servername != NULL && !sslServerNameCallback.isNull()){
            result = vm_call_user_func(sslServerNameCallback, make_packed_array(StringData::Make(servername, CopyString)));
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
        char buf[1024];
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

    static void read_cb(uv_ssl_ext_t *ssl_handle, ssize_t nread, const uv_buf_t* buf) {
        auto* data = Native::data<UVTcpData>(ssl_handle->tcp_object_data);
        auto readCallback = data->readCallback;
        auto errorCallback = data->errorCallback;
        auto sslHandshakeCallback = ssl_handle->sslHandshakeCallback;
        char read_buf[256];
        int size, err, ret, read_buf_index;
        if(nread > 0){
            BIO_write(ssl_handle->sslResource.read_bio, buf->base, nread);
            if (!SSL_is_init_finished(ssl_handle->sslResource.ssl)) {
                ret = SSL_do_handshake(ssl_handle->sslResource.ssl);
                write_bio_to_socket(ssl_handle);
                if(ret != 1){
                    err = SSL_get_error(ssl_handle->sslResource.ssl, ret);
                    if (err == SSL_ERROR_WANT_READ) {
                    }
                    else if(err == SSL_ERROR_WANT_WRITE){
                        write_bio_to_socket(ssl_handle);
                    }
                    else{
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
            read_buf_index = 0;
            while(true){
                size = SSL_read(ssl_handle->sslResource.ssl, &read_buf[read_buf_index], sizeof(read_buf) - read_buf_index);
                if(size > 0){
                    read_buf_index+=size;
                }
                if((size <= 0 || read_buf_index >= sizeof(read_buf)) && !readCallback.isNull()){
                    vm_call_user_func(readCallback, make_packed_array(ssl_handle->tcp_object_data, StringData::Make(read_buf, read_buf_index, CopyString)));
                    read_buf_index = 0;
                }
                if(size <= 0){
                    break;
                }
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
        auto* data = Native::data<UVTcpData>(this_);
        initUVTcpObject(this_, uv_default_loop(), new uv_ssl_ext_t());        
        uv_ssl_ext_t *ssl_handle = fetchSSLHandle(data);
        initSSLHandle(ssl_handle);
        switch(method){
            case 0:  //SSL_METHOD_SSLV2
#ifdef OPENSSL_NO_SSL2
                ssl_handle->sslResource.ssl_method = SSLv3_method();
                break;
#else
                ssl_handle->sslResource.ssl_method = SSLv2_method();
                break;
#endif
            case 1:  //SSL_METHOD_SSLV3
                ssl_handle->sslResource.ssl_method = SSLv3_method();
                break;
            case 2:  //SSL_METHOD_SSLV23
                ssl_handle->sslResource.ssl_method = SSLv23_method();
                break;
            case 3:  //SSL_METHOD_TLSV1
                ssl_handle->sslResource.ssl_method = TLSv1_method();
                break;
            case 4:  //SSL_METHOD_TLSV1_1
                ssl_handle->sslResource.ssl_method = TLSv1_1_method();
                break;
            case 5: //SSL_METHOD_TLSV1_2
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
        if(method > 2){ // tls
            SSL_CTX_set_tlsext_servername_callback(ssl_handle->sslResource.ctx[0], sni_cb);
            SSL_CTX_set_tlsext_servername_arg(ssl_handle->sslResource.ctx[0], this_);
        }
#endif
    }
    
    static void HHVM_METHOD(UVSSL, __destruct){
        auto* data = Native::data<UVTcpData>(this_);
        uv_ssl_ext_t *ssl_handle = fetchSSLHandle(data);
        if(ssl_handle->sslResource.ctx){
        //use SSL_free instead
/*
            if(ssl->read_bio){
                BIO_free(ssl->read_bio);                
            }
            if(ssl->write_bio){
                BIO_free(ssl->write_bio);
            }*/
            if(ssl_handle->sslResource.ssl){
                SSL_free(ssl_handle->sslResource.ssl);
            }
            for(int i=0;i<ssl_handle->sslResource.nctx;i++){
                SSL_CTX_free(ssl_handle->sslResource.ctx[i]);
            }
            delete ssl_handle->sslResource.ctx;
            ssl_handle->sslResource.ctx = NULL;
        }
        data->release();
        if(data->tcp_handle){
            delete data->tcp_handle;
        }
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
        ssl_handle->flag |= (UV_TCP_HANDLE_START|UV_TCP_READ_START);
        return objectData;
    }

    static int64_t HHVM_METHOD(UVSSL, setCallback, const Variant &onReadCallback, const Variant &onWriteCallback, const Variant &onErrorCallback) {
        auto* data = Native::data<UVTcpData>(this_);
        data->readCallback = onReadCallback;
        data->writeCallback = onWriteCallback;
        data->errorCallback = onErrorCallback;
        return 0;
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
        uv_read_start((uv_stream_t *) ssl_handle, alloc_cb, (uv_read_cb) read_cb);
        ssl_handle->sslResource.ssl = SSL_new(ssl_handle->sslResource.ctx[0]);
        ssl_handle->sslResource.read_bio = BIO_new(BIO_s_mem());
        ssl_handle->sslResource.write_bio = BIO_new(BIO_s_mem());        
        SSL_set_bio(ssl_handle->sslResource.ssl, ssl_handle->sslResource.read_bio, ssl_handle->sslResource.write_bio);
        SSL_set_connect_state(ssl_handle->sslResource.ssl);
        SSL_connect(ssl_handle->sslResource.ssl);
        write_bio_to_socket(ssl_handle);
        if(!callback.isNull()){
            vm_call_user_func(callback, make_packed_array(ssl_handle->tcp_object_data, status));
        }    
    }   
    
    static int64_t HHVM_METHOD(UVSSL, connect, const String &host, int64_t port, const Variant &onConnectCallback) {
        int64_t ret;
        struct sockaddr_in addr;
        auto* data = Native::data<UVTcpData>(this_);
        uv_ssl_ext_t *ssl_handle = fetchSSLHandle(data);
        
        if((ret = uv_ip4_addr(host.c_str(), port&0xffff, &addr)) != 0){
            return ret;
        }
        if((ret = uv_tcp_connect(&ssl_handle->connect_req, (uv_tcp_t *) data->tcp_handle, (const struct sockaddr*) &addr, client_connection_cb)) != 0){
            return ret;
        }
        
        data->connectCallback = onConnectCallback;
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
        HHVM_ME(UVSSL, setSSLHandshakeCallback);
        HHVM_ME(UVSSL, setSSLServerNameCallback);
    }
}
