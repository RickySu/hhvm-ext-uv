#ifndef UV_SSL_H
#define UV_SSL_H

#include "uv_tcp.h"
#include "ssl_verify.h"

#define OPENSSL_DEFAULT_STREAM_VERIFY_DEPTH 9

#define SSL_METHOD_SSLV2 0
#define SSL_METHOD_SSLV3 1
#define SSL_METHOD_SSLV23 2
#define SSL_METHOD_TLSV1 3
#define SSL_METHOD_TLSV1_1 4
#define SSL_METHOD_TLSV1_2 5

#define REGISTER_UV_SSL_CONSTANT(name) \
    Native::registerClassConstant<KindOfInt64>(s_uvssl.get(), \
        makeStaticString(#name), \
        name)

namespace HPHP
{
    typedef struct {
        const SSL_METHOD *ssl_method;
        SSL_CTX** ctx;
        int nctx;
        SSL* ssl;
        BIO* read_bio;
        BIO* write_bio;
    } ssl_ext_t;
    
    typedef struct uv_ssl_ext_s: public uv_tcp_ext_t {
        ssl_ext_t sslResource;
        Variant sslHandshakeCallback;
        Variant sslServerNameCallback;
        String sniConnectHostname;
        bool clientMode;
    } uv_ssl_ext_t;
    
    ALWAYS_INLINE void initSSLHandle(uv_ssl_ext_t *handle)
    {
        handle->sslResource = {NULL, NULL, 0, NULL, NULL, NULL};
        handle->sslHandshakeCallback.setNull();
        handle->sslServerNameCallback.setNull();
        handle->sniConnectHostname.clear();
        handle->clientMode = false;
    }
    
    ALWAYS_INLINE uv_ssl_ext_t *fetchSSLHandle(UVTcpData *data){
        return (uv_ssl_ext_t *) data->tcp_handle;
    }    
}
#endif
