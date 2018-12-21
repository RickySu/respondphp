#include "respondphp.h"
#ifdef HAVE_OPENSSL
#include "server/secure.h"

DECLARE_FUNCTION_ENTRY(respond_server_secure) =
{
    PHP_ME(respond_server_secure, __construct, ARGINFO(respond_server_secure, __construct), ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(respond_server_secure, close, NULL, ZEND_ACC_PUBLIC)
    TRAIT_FUNCTION_ENTRY_ME(respond_server_secure, event_emitter)
    PHP_FE_END
};

static zend_object *create_respond_server_secure_resource(zend_class_entry *class_type);
static void free_respond_server_secure_resource(zend_object *object);
static void accepted_cb(int n_param, zval *param, rp_server_secure_ext_t *resource);
static void releaseResource(rp_server_secure_ext_t *resource);
static void ssl_ctx_ht_free(zval *item);
static void ssl_ctx_parse(zval *array, rp_server_secure_ext_t *resource);
static zend_bool ssl_ctx_set_pkey(SSL_CTX *ctx, zval *pem_pkey, zval *passphrase);
static zend_bool ssl_ctx_set_cert(SSL_CTX *ctx, zval *pem_cert);
static int ssl_ctx_set_pkey_password_cb(char *buf, int size, int rwflag, void *userdata);
static int ssl_sni_cb(SSL *ssl, int *ad, rp_server_secure_ext_t *resource);
static void ssl_set_ciphers(zval *pStruct, rp_server_secure_ext_t *resource);

CLASS_ENTRY_FUNCTION_D(respond_server_secure)
{
    REGISTER_CLASS_WITH_OBJECT_NEW(respond_server_secure, "Respond\\Server\\Secure", create_respond_server_secure_resource);
    OBJECT_HANDLER(respond_server_secure).offset = XtOffsetOf(rp_server_secure_ext_t, zo);
    OBJECT_HANDLER(respond_server_secure).clone_obj = NULL;
    OBJECT_HANDLER(respond_server_secure).free_obj = free_respond_server_secure_resource;
    zend_class_implements(CLASS_ENTRY(respond_server_secure), 1, CLASS_ENTRY(respond_event_event_emitter_interface));
}

static void releaseResource(rp_server_secure_ext_t *resource)
{
}

static zend_object *create_respond_server_secure_resource(zend_class_entry *ce)
{
    rp_server_secure_ext_t *resource;
    resource = ALLOC_RESOURCE(rp_server_secure_ext_t, ce);
    zend_object_std_init(&resource->zo, ce);
    object_properties_init(&resource->zo, ce);    
    resource->zo.handlers = &OBJECT_HANDLER(respond_server_secure);
    rp_event_hook_init(&resource->event_hook);
    zend_hash_init(&resource->ssl_ctx_ht, 5, ssl_ctx_ht_free, NULL, 0);
    return &resource->zo;
}

static void ssl_ctx_ht_free(zval *item)
{
    SSL_CTX_free(Z_PTR_P(item));
}

static void free_respond_server_secure_resource(zend_object *object)
{
    rp_server_secure_ext_t *resource;
    resource = FETCH_RESOURCE(object, rp_server_secure_ext_t);
    if(resource->socket_zo) {
        zend_object_ptr_dtor(resource->socket_zo);
    }
    releaseResource(resource);
    rp_event_hook_destroy(&resource->event_hook);
    zend_hash_destroy(&resource->ssl_ctx_ht);
    zend_object_std_dtor(object);
}

static int ssl_ctx_set_pkey_password_cb(char *buf, int size, int rwflag, void *userdata)
{
    zend_string *passphrase = (zend_string *) userdata;
    size = (passphrase->len > size) ? size : passphrase->len;
    memcpy(buf, passphrase->val, size);
    return size;
}

static zend_bool ssl_ctx_set_pkey(SSL_CTX *ctx, zval *pem_pkey, zval *passphrase)
{
    int ret;
    BIO *key_bio;
    EVP_PKEY *pkey = NULL;

    if(pem_pkey == NULL || Z_TYPE_P(pem_pkey) != IS_STRING){
        return 0;
    }

    convert_to_string(pem_pkey);

    key_bio = BIO_new(BIO_s_mem());

    if(BIO_write(key_bio, Z_STRVAL_P(pem_pkey), Z_STRLEN_P(pem_pkey)) <= 0){
        BIO_free(key_bio);
        return 0;
    }

    if(passphrase && Z_TYPE_P(passphrase) == IS_STRING) {
        pkey = PEM_read_bio_PrivateKey(key_bio, NULL, ssl_ctx_set_pkey_password_cb, Z_STR_P(passphrase));
    }
    else {
        pkey = PEM_read_bio_PrivateKey(key_bio, NULL, NULL, NULL);
    }

    BIO_free(key_bio);

    if(pkey == NULL){
        return 0;
    }

    ret = SSL_CTX_use_PrivateKey(ctx, pkey);
    EVP_PKEY_free(pkey);
    return ret;
}

static zend_bool ssl_ctx_set_cert(SSL_CTX *ctx, zval *pem_cert)
{
    long status;
    BIO *cert_bio;
    X509 *cert;

    if(pem_cert == NULL || Z_TYPE_P(pem_cert) != IS_STRING){
        return 0;
    }

    cert_bio = BIO_new(BIO_s_mem());

    if(BIO_write(cert_bio, Z_STRVAL_P(pem_cert), Z_STRLEN_P(pem_cert)) <= 0){
        BIO_free(cert_bio);
        return 0;
    }

    cert = PEM_read_bio_X509_AUX(cert_bio, NULL, NULL, NULL);

    if(cert == NULL){
        BIO_free(cert_bio);
        return 0;
    }

    if(SSL_CTX_use_certificate(ctx, cert) == 0){
        X509_free(cert);
        BIO_free(cert_bio);
        return 0;
    }
//    X509_free(cert);

    while(1){
        cert = PEM_read_bio_X509(cert_bio, NULL, NULL, NULL);

        if(cert == NULL){
            break;
        }

#ifdef SSL_CTRL_CHAIN_CERT
        status = SSL_CTX_add0_chain_cert(ctx, cert);
#else
        status = SSL_CTX_add_extra_chain_cert(ctx, cert);
#endif
        if(status == 0){
            X509_free(cert);
            BIO_free(cert_bio);
            return 0;
        }
    }

    X509_free(cert);
    BIO_free(cert_bio);
    return 1;
}

static int ssl_sni_cb(SSL *ssl, int *ad, rp_server_secure_ext_t *resource)
{
    zval *current = NULL;
    zend_string *ctx_hostname;
    zend_ulong index = 0;
    const char *servername = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);

    for(zend_hash_internal_pointer_reset(&resource->ssl_ctx_ht); current = zend_hash_get_current_data(&resource->ssl_ctx_ht); zend_hash_move_forward(&resource->ssl_ctx_ht)) {
        if(zend_hash_get_current_key(&resource->ssl_ctx_ht, &ctx_hostname, index) == HASH_KEY_IS_STRING){
            if(servername && strncmp(ctx_hostname->val, servername, ctx_hostname->len) == 0){
                break;
            }
        }
    }

    if(current) {
        SSL_set_SSL_CTX(ssl, Z_PTR_P(current));
    }
    return SSL_TLSEXT_ERR_OK;
}

static void ssl_ctx_parse(zval *array, rp_server_secure_ext_t *resource)
{
    HashTable *array_ht = Z_ARRVAL_P(array);
    zval *current, *hostname;
    SSL_CTX *ctx = NULL;
    for(zend_hash_internal_pointer_reset(array_ht); current = zend_hash_get_current_data(array_ht); zend_hash_move_forward(array_ht)) {
        ctx = SSL_CTX_new(SECURE_METHOD());
        if(!(
            ssl_ctx_set_pkey(ctx, zend_hash_str_find(Z_ARRVAL_P(current), ZEND_STRL("local_pk")), zend_hash_str_find(Z_ARRVAL_P(current), ZEND_STRL("passphrase"))) &&
            ssl_ctx_set_cert(ctx, zend_hash_str_find(Z_ARRVAL_P(current), ZEND_STRL("local_cert")))
        )){
            SSL_CTX_free(ctx);
            continue;
        }
        SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_BOTH);

        hostname = zend_hash_str_find(Z_ARRVAL_P(current), ZEND_STRL("hostname"));
        if(!hostname || Z_TYPE_P(hostname) != IS_STRING){
            zend_hash_str_update_ptr(&resource->ssl_ctx_ht, ZEND_STRL("*"), ctx);
        }
        else{
            zend_hash_update_ptr(&resource->ssl_ctx_ht, Z_STR_P(hostname), ctx);
        }

        resource->ctx = ctx;
    }

#ifdef HAVE_OPENSSL_TLS_SNI
    if(resource->ctx){
        SSL_CTX_set_tlsext_servername_callback(resource->ctx, ssl_sni_cb);
        SSL_CTX_set_tlsext_servername_arg(resource->ctx, resource);
    }
#endif
}

static void ssl_set_ciphers(zval *cipher_list, rp_server_secure_ext_t *resource)
{
    char *cipher_list_string = "DEFAULT";

    if(cipher_list) {
        cipher_list_string = Z_STRVAL_P(cipher_list);
    }

    SSL_CTX_set_cipher_list(resource->ctx, cipher_list_string);
}

PHP_METHOD(respond_server_secure, __construct)
{
    zval *self = getThis();
    zval *socket, *options;
    rp_server_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_secure_ext_t);
    event_hook_t *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "za", &socket, &options)) {
        return;
    }

    hook = (event_hook_t *) ((void *) Z_OBJ_P(socket) - sizeof(event_hook_t));
    rp_event_emitter_on_intrenal_ex(hook, ZEND_STRL("connect"), (rp_event_emitter_internal_cb) accepted_cb, resource);
    resource->socket_zo = Z_OBJ_P(socket);
    Z_ADDREF_P(socket);
    ssl_ctx_parse(zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("ssl")), resource);
    ssl_set_ciphers(zend_hash_str_find(Z_ARRVAL_P(options), ZEND_STRL("ciphers")), resource);
}

static void handshake_read_cb(int n_param, zval *param, rp_stream_secure_ext_t *connection_secure_resource)
{
    int ret, err;
    zval connection_secure;

    if((ret = SSL_do_handshake(connection_secure_resource->ssl)) == 1){
        connection_secure_resource->handshake = NULL;
        ZVAL_OBJ(&connection_secure, &connection_secure_resource->zo);
        zval_add_ref(&connection_secure);
        rp_event_emitter_emit_internal(&((rp_server_secure_ext_t *)connection_secure_resource->creater_resource)->event_hook, ZEND_STRL("connect"), 1, &connection_secure);
        ZVAL_PTR_DTOR(&connection_secure);
    }

    write_bio_to_socket(connection_secure_resource);
    err = SSL_get_error(connection_secure_resource->ssl, ret);
    switch(err){
        case SSL_ERROR_SSL:
            connection_secure_resource->connection->connection_methods.close(connection_secure_resource->connection);
            break;
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
        default:
            break;
    }
}

static void accepted_cb(int n_param, zval *param, rp_server_secure_ext_t *server_resource)
{
    rp_stream_connection_ext_t *connection_connection_resource = FETCH_OBJECT_RESOURCE(&param[0], rp_stream_connection_ext_t);
    rp_stream_secure_ext_t *connection_secure_resource;
    zval connection_secure;
    rp_stream_secure_factory(SSL_new(server_resource->ctx), &param[0], &connection_secure);
    connection_secure_resource = FETCH_OBJECT_RESOURCE(&connection_secure, rp_stream_secure_ext_t);
    connection_secure_resource->creater_resource = server_resource;
    connection_secure_resource->handshake = handshake_read_cb;
    SSL_set_accept_state(connection_secure_resource->ssl);
}

PHP_METHOD(respond_server_secure, close)
{
//    zval *self = getThis();
//    rp_server_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_secure_ext_t);
}

PHP_METHOD(respond_server_secure, on)
{
    zval *self = getThis();
    rp_server_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_secure_ext_t);
    zend_string *event;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sz", &event, &hook)) {
        return;
    }
    rp_event_emitter_on(&resource->event_hook, event->val, event->len, hook);
}

PHP_METHOD(respond_server_secure, off)
{
    zval *self = getThis();
    rp_server_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_secure_ext_t);
    zend_string *event;
    zval *hook;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "Sz", &event, &hook)) {
        return;
    }
    rp_event_emitter_off(&resource->event_hook, event->val, event->len, hook);
}

PHP_METHOD(respond_server_secure, removeListeners)
{
    zval *self = getThis();
    rp_server_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_secure_ext_t);
    zend_string *event;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &event)) {
        return;
    }

    rp_event_emitter_removeListeners(&resource->event_hook, event->val, event->len);
}

PHP_METHOD(respond_server_secure, getListeners)
{
    zval *self = getThis();
    zval *listeners;
    rp_server_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_secure_ext_t);
    zend_string *event;

    if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "S", &event)) {
        return;
    }

    listeners = rp_event_emitter_getListeners(&resource->event_hook, event->val, event->len);

    if(listeners == NULL){
        RETURN_NULL();
    }

    RETURN_ZVAL(listeners, 1, 0);
}

PHP_METHOD(respond_server_secure, emit)
{
    zval *self = getThis();
    rp_server_secure_ext_t *resource = FETCH_OBJECT_RESOURCE(self, rp_server_secure_ext_t);
    zval *params;
    int n_params;

    ZEND_PARSE_PARAMETERS_START(2, -1)
        Z_PARAM_VARIADIC('+', params, n_params)
    ZEND_PARSE_PARAMETERS_END_EX();
    convert_to_string_ex(&params[0]);
    rp_event_emitter_emit(&resource->event_hook, Z_STRVAL(params[0]), Z_STRLEN(params[0]), n_params - 1, &params[1]);
}

#endif
