#include "ssl_verify.h"

int php_x509_fingerprint_cmp(X509 *peer, const char *method, const char *expected)
{
	int result = -1;
/*	zend_string *fingerprint;


	fingerprint = php_openssl_x509_fingerprint(peer, method, 0);
	if (fingerprint) {
		result = strcasecmp(expected, ZSTR_VAL(fingerprint));
		zend_string_release(fingerprint);
	}
*/
	return result;
}

int php_x509_fingerprint_match(X509 *peer, const char *hash)
{
/*	if (Z_TYPE_P(val) == IS_STRING) {
		const char *method = NULL;

		switch (Z_STRLEN_P(val)) {
			case 32:
				method = "md5";
				break;

			case 40:
				method = "sha1";
				break;
		}

		return method && php_x509_fingerprint_cmp(peer, method, Z_STRVAL_P(val)) == 0;
	} else if (Z_TYPE_P(val) == IS_ARRAY) {
		zval *current;
		zend_string *key;

		if (!zend_hash_num_elements(Z_ARRVAL_P(val))) {
			php_error_docref(NULL, E_WARNING, "Invalid peer_fingerprint array; [algo => fingerprint] form required");
			return 0;
		}

		ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(val), key, current) {
			if (key == NULL || Z_TYPE_P(current) != IS_STRING) {
				php_error_docref(NULL, E_WARNING, "Invalid peer_fingerprint array; [algo => fingerprint] form required");
				return 0;
			}
			if (php_x509_fingerprint_cmp(peer, ZSTR_VAL(key), Z_STRVAL_P(current)) != 0) {
				return 0;
			}
		} ZEND_HASH_FOREACH_END();

		return 1;
	} else {
		php_error_docref(NULL, E_WARNING,
			"Invalid peer_fingerprint value; fingerprint string or array of the form [algo => fingerprint] required");
	}
*/
	return 0;
}

int matches_wildcard_name(const char *subjectname, const char *certname) {
    int match = (strcmp(subjectname, certname) == 0);
    if (!match && strlen(certname) > 3 && certname[0] == '*' && certname[1] == '.') {
        /* Try wildcard */
        if (strchr(certname+2, '.')){
            const char* cnmatch_str = subjectname;
            const char *tmp = strstr(cnmatch_str, certname+1);
            match = tmp && strcmp(tmp, certname+2) && tmp == strchr(cnmatch_str, '.');
	}
    }
    return match;                                                     
}

int matches_san_list(X509 *peer, const char *subject_name) {
	int i, len;
	unsigned char *cert_name = NULL;
	char ipbuffer[64];

	GENERAL_NAMES *alt_names = X509_get_ext_d2i(peer, NID_subject_alt_name, 0, 0);
	int alt_name_count = sk_GENERAL_NAME_num(alt_names);

	for (i = 0; i < alt_name_count; i++) {
		GENERAL_NAME *san = sk_GENERAL_NAME_value(alt_names, i);

		if (san->type == GEN_DNS) {
			ASN1_STRING_to_UTF8(&cert_name, san->d.dNSName);
			if (ASN1_STRING_length(san->d.dNSName) != strlen((const char*)cert_name)) {
				OPENSSL_free(cert_name);
				/* prevent null-byte poisoning*/
				continue;
			}

			/* accommodate valid FQDN entries ending in "." */
			len = strlen((const char*)cert_name);
			if (len && strcmp((const char *)&cert_name[len-1], ".") == 0) {
				cert_name[len-1] = '\0';
			}

			if (matches_wildcard_name(subject_name, (const char *)cert_name)) {
				OPENSSL_free(cert_name);
				return 1;
			}
			OPENSSL_free(cert_name);
		} else if (san->type == GEN_IPADD) {
			if (san->d.iPAddress->length == 4) {
				sprintf(ipbuffer, "%d.%d.%d.%d",
					san->d.iPAddress->data[0],
					san->d.iPAddress->data[1],
					san->d.iPAddress->data[2],
					san->d.iPAddress->data[3]
				);
				if (strcasecmp(subject_name, (const char*)ipbuffer) == 0) {
					return 1;
				}
			}
			/* No, we aren't bothering to check IPv6 addresses. Why?
 * 			 * Because IP SAN names are officially deprecated and are
 * 			 			 * not allowed by CAs starting in 2015. Deal with it.
 * 			 			 			 */
		}
	}

	return 0;
}
/* }}} */

int matches_common_name(X509 *peer, const char *subject_name) {
	char buf[1024];
	X509_NAME *cert_name;
	int is_match = 0;
	int cert_name_len;

	cert_name = X509_get_subject_name(peer);
	cert_name_len = X509_NAME_get_text_by_NID(cert_name, NID_commonName, buf, sizeof(buf));

	if (cert_name_len == -1) {
		//php_error_docref(NULL, E_WARNING, "Unable to locate peer certificate CN");
	} else if (cert_name_len != strlen(buf)) {
		//php_error_docref(NULL, E_WARNING, "Peer certificate CN=`%.*s' is malformed", cert_name_len, buf);
	} else if (matches_wildcard_name(subject_name, buf)) {
		is_match = 1;
	} else {
		//php_error_docref(NULL, E_WARNING, "Peer certificate CN=`%.*s' did not match expected CN=`%s'", cert_name_len, buf, subject_name);
	}

	return is_match;
}
/* }}} */
