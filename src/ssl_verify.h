#ifndef SSL_VERIFY_H
#define SSL_VERIFY_H

#include <openssl/ssl.h>
#include <openssl/x509v3.h>

#ifdef __cplusplus
extern "C" {
#endif

int matches_wildcard_name(const char *subjectname, const char *certname);

int matches_san_list(X509 *peer, const char *subject_name);

int matches_common_name(X509 *peer, const char *subject_name);

#ifdef __cplusplus 
}
#endif

#endif