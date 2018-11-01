#ifndef RESPONDPHP_SSL_DETECT_H
#define RESPONDPHP_SSL_DETECT_H

#ifdef HAVE_OPENSSL_TLS_METHOD
#ifndef SECURE_SERVER_METHOD
#define SECURE_SERVER_METHOD TLS_server_method
#endif
#endif

#ifdef HAVE_OPENSSL_TLSV1_3_METHOD
#ifndef SECURE_SERVER_METHOD
#define SECURE_SERVER_METHOD TLSv1_3_method
#endif
#endif

#ifdef HAVE_OPENSSL_TLSV1_2_METHOD
#ifndef SECURE_SERVER_METHOD
#define SECURE_SERVER_METHOD TLSv1_2_method
#endif
#endif

#ifdef HAVE_OPENSSL_TLSV1_1_METHOD
#ifndef SECURE_SERVER_METHOD
#define SECURE_SERVER_METHOD TLSv1_1_method
#endif
#endif

#ifdef HAVE_OPENSSL_TLSV1_METHOD
#ifndef SECURE_SERVER_METHOD
#define SECURE_SERVER_METHOD TLSv1_method
#endif
#endif

//#error no tls support

#ifdef SSL_CTX_set_tlsext_servername_callback
    #define HAVE_OPENSSL_TLS_SNI 1
#endif

#endif //RESPONDPHP_SSL_DETECT_H
