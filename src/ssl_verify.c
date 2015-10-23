#include "ssl_verify.h"

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

int matches_common_name(X509 *peer, const char *subject_name) {
	char buf[1024];
	X509_NAME *cert_name;
	int cert_name_len;

	cert_name = X509_get_subject_name(peer);
	cert_name_len = X509_NAME_get_text_by_NID(cert_name, NID_commonName, buf, sizeof(buf));

	if (cert_name_len == -1) {
	    return 0;
	} 
	
	if (cert_name_len != strlen(buf)) {
	    return 0;
	}
	if (!matches_wildcard_name(subject_name, buf)) {
	    return 0;
	}

	return 1;
}
