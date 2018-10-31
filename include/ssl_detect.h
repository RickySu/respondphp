#ifndef RESPONDPHP_SSL_DETECT_H
#define RESPONDPHP_SSL_DETECT_H

#ifdef HAVE_OPENSSL_TLS_METHOD
    #define SECURE_SERVER_METHOD TLS_server_method
#else
    #ifdef HAVE_OPENSSL_TLSV1_2_METHOD
        #define SECURE_SERVER_METHOD TLSv1_2_method
    #else
        #ifdef HAVE_OPENSSL_TLSV1_1_METHOD
            #define SECURE_SERVER_METHOD TLSv1_1_method
        #else
            #ifdef HAVE_OPENSSL_TLSV1_METHOD
                #define SECURE_SERVER_METHOD TLSv1_method
            #else
                #error no tls support
            #endif
        #endif
    #endif
#endif

#ifdef SSL_CTX_set_tlsext_servername_callback
    #define HAVE_OPENSSL_TLS_SNI 1
#endif

#endif //RESPONDPHP_SSL_DETECT_H
