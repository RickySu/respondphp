#include "respondphp.h"

#ifdef HAVE_OPENSSL
#include "internal/socket_connector.h"
#include "stream/secure.h"
#include "connector/secure.h"

DECLARE_FUNCTION_ENTRY(respond_connector_secure) =
{
    PHP_ME(respond_connector_secure, connect, ARGINFO(respond_connector_secure, connect), ZEND_ACC_PUBLIC)
    PHP_FE_END
};

static zend_object *create_respond_connector_secure_resource(zend_class_entry *class_type);
static void free_respond_connector_secure_resource(zend_object *object);
static void releaseResource(rp_connector_secure_ext_t *resource);
static void connection_cb(rp_secure_connector_t *connector, int status);
static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx);
static void handshake_read_cb(int n_param, zval *param, rp_stream_secure_ext_t *connection_secure_resource);
static zend_bool matches_wildcard_name(const char *subjectname, const char *certname);
static zend_bool matches_san_list(X509 *peer, const char *subject_name);
static zend_bool matches_common_name(X509 *peer, const char *subject_name);
static void releaseConnector(rp_secure_connector_t *connector);

CLASS_ENTRY_FUNCTION_D(respond_connector_secure)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_connector_secure, "Respond\\Connector\\Secure", create_respond_connector_secure_resource);
    OBJECT_HANDLER(respond_connector_secure).offset = XtOffsetOf(rp_connector_secure_ext_t, zo);
    OBJECT_HANDLER(respond_connector_secure).clone_obj = NULL;
    OBJECT_HANDLER(respond_connector_secure).free_obj = free_respond_connector_secure_resource;
    zend_class_implements(CLASS_ENTRY(respond_connector_secure), 1, CLASS_ENTRY(respond_socket_connector_interface));
}

static void releaseConnector(rp_secure_connector_t *connector)
{
    zend_object_ptr_dtor(connector->connector.zo);
    ZVAL_PTR_DTOR(&connector->connector.promise);
    zend_string_release(connector->server_name);
    rp_free(connector);
}

static void releaseResource(rp_connector_secure_ext_t *resource)
{
}

static zend_object *create_respond_connector_secure_resource(zend_class_entry *ce)
{
    rp_connector_secure_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_connector_secure_ext_t, ce);
    fprintf(stderr, "resource: %p\n", resource);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);    
    resource->zo.handlers = &OBJECT_HANDLER(respond_connector_secure);
    return &resource->zo;
}

static void free_respond_connector_secure_resource(zend_object *object)
{
    rp_connector_secure_ext_t *resource;
    resource = FETCH_RESOURCE(object, rp_connector_secure_ext_t);
    releaseResource(resource);
    zend_object_std_dtor(object);
}

PHP_METHOD(respond_connector_secure, connect)
{
    zval *self = getThis();
    rp_connector_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_connector_secure_ext_t);
    zend_string *host, *server_name;
    zend_long port;
    rp_secure_connector_t *connector;
    rp_reactor_addr_t addr;
    uv_tcp_t *uv_tcp;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "SlS", &host, &port, &server_name)) {
        return;
    }

    if(memchr(host->val, ':', host->len) == NULL) {
        if (uv_ip4_addr(host->val, port & 0xffff, &addr.sockaddr) != 0) {
            return;
        }
    }
    else {
        if (uv_ip6_addr(host->val, port & 0xffff, &addr.sockaddr6) != 0) {
            return;
        }
    }

    uv_tcp = rp_malloc(sizeof(uv_tcp_t));
    uv_tcp_init(&main_loop, uv_tcp);
    connector = rp_malloc(sizeof(rp_secure_connector_t));
    zend_string_addref(server_name);
    connector->server_name = server_name;

    rp_make_promise_object(&connector->connector.promise);
    connector->connector.zo = Z_OBJ_P(self);
    Z_ADDREF_P(self);

    rp_socket_connect_ex(connector, uv_tcp, &addr, connection_cb);
    fprintf(stderr, "connect: %p %p\n", connector, uv_tcp);
    RETVAL_ZVAL(&connector->connector.promise, 1, 0);
}

static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx)
{
    fprintf(stderr, "verify callback\n");
    X509_STORE_CTX_get_current_cert(ctx);
    int err = X509_STORE_CTX_get_error(ctx);
    int depth = X509_STORE_CTX_get_error_depth(ctx);
    SSL *ssl = X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
    rp_secure_connector_t *connector = SSL_get_ex_data(ssl, 0);
    fprintf(stderr, "verify connection_secure_resource: %p\n", connector->connection_secure_resource);

    if(depth > 3){
        X509_STORE_CTX_set_error(ctx, X509_V_ERR_CERT_CHAIN_TOO_LONG);
        return 0;
    }

    if(err == X509_V_OK){
        return 1;
    }
}

static void handshake_read_cb(int n_param, zval *param, rp_stream_secure_ext_t *connection_secure_resource)
{
    int ret, err;
    zval connection_secure, exception, err_message;
    X509 *peer_cert;
    rp_secure_connector_t *connector;
//    BIO_write(connection_secure_resource->read_bio, Z_STRVAL(param[1]), Z_STRLEN(param[1]));
    fprintf(stderr, "handshake read %d %p\n", Z_STRLEN(param[1]), connection_secure_resource);

    if((ret = SSL_do_handshake(connection_secure_resource->ssl)) == 1){
        connection_secure_resource->handshake = NULL;
        fprintf(stderr, "handshake ok\n");
        if(peer_cert = SSL_get_peer_certificate(connection_secure_resource->ssl)) {
            connector = SSL_get_ex_data(connection_secure_resource->ssl, 0);
            if(!matches_common_name(peer_cert, connector->server_name->val) && !matches_san_list(peer_cert, connector->server_name->val)) {
                fprintf(stderr, "server name miss match\n");
                connection_secure_resource->connection_methods.close(connection_secure_resource);

                ZVAL_STRING(&err_message, "cert not match");
                object_init_ex(&exception, zend_ce_exception);
                zend_call_method_with_1_params(&exception, NULL, &Z_OBJCE(exception)->constructor, "__construct", NULL, &err_message);
                rp_reject_promise(&connector->connector.promise, &exception);
                ZVAL_PTR_DTOR(&exception);
                ZVAL_PTR_DTOR(&err_message);

                releaseConnector(connector);
                X509_free(peer_cert);
                return;
            }
            fprintf(stderr, "server name matched\n");
            ZVAL_OBJ(&connection_secure, &connection_secure_resource->zo);
            zval_add_ref(&connection_secure);
            rp_resolve_promise(&connector->connector.promise, &connection_secure);
            ZVAL_PTR_DTOR(&connection_secure);
            releaseConnector(connector);
            X509_free(peer_cert);
        }
    }

    write_bio_to_socket(connection_secure_resource);
    err = SSL_get_error(connection_secure_resource->ssl, ret);
    switch(err){
        case SSL_ERROR_WANT_READ:
            fprintf(stderr, "ssl want read\n");
            break;
        case SSL_ERROR_WANT_WRITE:
            fprintf(stderr, "ssl want write:%d\n", err);
            break;
        default:
            fprintf(stderr, "ssl error:%d\n", err);
            break;
    }
}

static void connection_cb(rp_secure_connector_t *connector, int status)
{
    zval connection_secure, exception, param;
    rp_stream_secure_ext_t *connection_secure_resource;
    SSL_CTX *ctx;

    fprintf(stderr, "client connected: %d\n", status);

    if(status < 0) {
        ZVAL_LONG(&param, status);
        object_init_ex(&exception, zend_ce_exception);
        zend_call_method_with_1_params(&exception, NULL, &Z_OBJCE(exception)->constructor, "__construct", NULL, &param);
        rp_reject_promise(&connector->connector.promise, &exception);
        ZVAL_PTR_DTOR(&connector->connector.promise);
        zend_object_ptr_dtor(connector->connector.zo);
        uv_close(connector->connector.connect_req.handle, rp_free_cb);
        rp_free(connector);
        return;
    }

    ctx = SSL_CTX_new(SECURE_METHOD());
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, verify_callback);
    SSL_CTX_set_default_verify_paths(ctx);
    SSL_CTX_set_cipher_list(ctx, "DEFAULT");

    rp_stream_connection_factory(connector->connector.connect_req.handle, &param);
    rp_stream_secure_factory(SSL_new(ctx), &param, &connection_secure);
    connection_secure_resource = FETCH_OBJECT_RESOURCE(&connection_secure, rp_stream_secure_ext_t);
    connection_secure_resource->creater_resource = connector;
    connection_secure_resource->handshake = handshake_read_cb;
    connection_secure_resource->ctx = ctx;
    connector->connection_secure_resource = connection_secure_resource;
    SSL_set_ex_data(connection_secure_resource->ssl, 0, connector);

#ifndef OPENSSL_NO_TLSEXT
    SSL_set_tlsext_host_name(connection_secure_resource->ssl, connector->server_name->val);
#endif

    SSL_set_connect_state(connection_secure_resource->ssl);
    SSL_connect(connection_secure_resource->ssl);
    write_bio_to_socket(connection_secure_resource);
    fprintf(stderr, "connection_secure_resource: %p\n", connection_secure_resource);
}

static zend_bool matches_wildcard_name(const char *subjectname, const char *certname) {
    int match = (strcmp(subjectname, certname) == 0);
    fprintf(stderr, "match:%d\n", match);
    if (!match && strlen(certname) > 3 && certname[0] == '*' && certname[1] == '.') {
        fprintf(stderr, "wildcard matching\n");

        /* Try wildcard */
        if (strchr(certname+2, '.')){
            const char* cnmatch_str = subjectname;
            const char *tmp = strstr(cnmatch_str, certname+1);
            match = tmp && strcmp(tmp, certname+2) && tmp == strchr(cnmatch_str, '.');
        }
    }
    return match;
}

static zend_bool matches_san_list(X509 *peer, const char *subject_name) {
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

static zend_bool matches_common_name(X509 *peer, const char *subject_name) {
    char buf[256];
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

    fprintf(stderr, "name:%s %s %d\n", buf, subject_name, strncmp(subject_name, buf, cert_name_len));

    return matches_wildcard_name(subject_name, buf) != 0;
}


#endif
