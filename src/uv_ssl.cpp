#include <openssl/ssl.h>
#include "ext.h"
#include "hphp/runtime/base/thread-init-fini.h"
#include "uv_tcp.h"

#ifdef OPENSSL_FOUND
    #define check_ssl_support()
#else
    #define check_ssl_support() raise_error("please recompile hhvm libuv extension with opeensl support.")
#endif

typedef struct {
    SSL_CTX* ctx;
    SSL* ssl;
    BIO* read_bio;
    BIO* write_bio;
} ssl_ext_t;

namespace HPHP {
    
    ALWAYS_INLINE ssl_ext_t *fetchSSLResource(const Object &obj){
        TcpResourceData *resource_data = FETCH_RESOURCE(obj, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();        
        return (ssl_ext_t *) tcp_handle->custom_data;    
    }
/*
    static bool HHVM_METHOD(UVSSL, initSSL, bool isServerMode){

        if(isServerMode){
            ssl->ctx = SSL_CTX_new(SSLv23_server_method());
        }
        else{
            ssl->ctx = SSL_CTX_new(SSLv23_method());
        }
        return ssl->ctx != NULL;
    }
    static bool HHVM_METHOD(UVSSL, setCertFile, String &cert_file) {
        ssl_ext_t *ssl = fetchSSLResource(this_);
        return SSL_CTX_use_certificate_file(ssl->ctx, cert_file.c_str(), SSL_FILETYPE_PEM) == 1;
    }
   
    static bool HHVM_METHOD(UVSSL, setCertChainFile, String &cert_chain_file) {
        ssl_ext_t *ssl = fetchSSLResource(this_);        
        return SSL_CTX_use_certificate_chain_file(ssl->ctx, cert_chain_file.c_str()) == 1;
    }
    
    static bool HHVM_METHOD(UVSSL, setPrivateKeyFile, String &private_key_file) {
        ssl_ext_t *ssl = fetchSSLResource(this_);
        return SSL_CTX_use_PrivateKey_file(ssl->ctx, private_key_file.c_str(), SSL_FILETYPE_PEM) == 1;
    }
*/    
    ALWAYS_INLINE void initSSL(const Object &obj) {
        TcpResourceData *resource_data = FETCH_RESOURCE(obj, TcpResourceData, s_uvtcp);
        uv_tcp_ext_t *tcp_handle = (uv_tcp_ext_t *) resource_data->getInternalResourceData();        
        ssl_ext_t *ssl = new ssl_ext_t();
        tcp_handle->custom_data = (void *) ssl;
        memset(ssl, 0, sizeof(ssl_ext_t));
    }
    
    static void HHVM_METHOD(UVSSL, __destruct){
        ssl_ext_t *ssl = fetchSSLResource(this_);
        if(ssl){
            if(ssl->ctx){
                SSL_CTX_free(ssl->ctx);
            }
            if(ssl->ssl){
                SSL_free(ssl->ssl);
            }
            if(ssl->read_bio){
                BIO_free(ssl->read_bio);                
            }
            if(ssl->write_bio){
                BIO_free(ssl->write_bio);
            }
            delete ssl;
        }
        HHVM_MN(UVTcp, __destruct)(this_);
    }

    static int64_t HHVM_METHOD(UVSSL, listen, const String &host, int64_t port, const Variant &onConnectCallback) {
        ssl_ext_t *ssl;
        String certFile = this_->o_get("certFile", false, "UVSSL");
        String certChainFile = this_->o_get("certChainFile", false, "UVSSL");
        String privateKeyFile = this_->o_get("privateKeyFile", false, "UVSSL");
        initSSL(this_);
        ssl = fetchSSLResource(this_);
        ssl->ctx = SSL_CTX_new(SSLv23_method());
        SSL_CTX_use_certificate_file(ssl->ctx, certFile.c_str(), SSL_FILETYPE_PEM);
        SSL_CTX_use_certificate_chain_file(ssl->ctx, certChainFile.c_str());
        SSL_CTX_use_PrivateKey_file(ssl->ctx, privateKeyFile.c_str(), SSL_FILETYPE_PEM);
        return HHVM_MN(UVTcp, listen)(this_, host, port, onConnectCallback);
    }
    
    static Object HHVM_METHOD(UVSSL, accept){
        ssl_ext_t *server_ssl, *ssl;
        Object obj = make_accepted_uv_tcp_object(this_, "UVSSL");
        initSSL(obj);
        server_ssl = fetchSSLResource(this_);                
        ssl = fetchSSLResource(obj);
        ssl->ssl = SSL_new(server_ssl->ctx);
        ssl->read_bio = BIO_new(BIO_s_mem());
        ssl->write_bio = BIO_new(BIO_s_mem());
        SSL_set_bio(ssl->ssl, ssl->read_bio, ssl->write_bio);
        SSL_set_accept_state(ssl->ssl);
        return obj;
    }
    
    void uvExtension::_initUVSSLClass() {
        HHVM_ME(UVSSL, accept);
        HHVM_ME(UVSSL, listen);
        HHVM_ME(UVSSL, __destruct);
    }
}
