/* Symbol-level stubs for the NeL emscripten build.
 *
 * curl and openssl are linked by NeL::web / NLGUI but no network backend
 * exists in the wasm build; every entry point reports failure the way the
 * real library would, so callers take their normal error paths instead of
 * hitting emscripten's aborting missing-function imports. The SysV IPC
 * stubs cover nelmisc shared-memory helpers that are never used in wasm.
 */

#include <stddef.h>

/* ---- curl ---- */

typedef struct
{
	int age;
	const char *version;
	unsigned int version_num;
	const char *host;
	int features;
	const char *ssl_version;
	long ssl_version_num;
	const char *libz_version;
	const char *const *protocols;
	/* generous zero padding for later ages */
	const char *pad[16];
} nl_stub_curl_version_info_data;

static nl_stub_curl_version_info_data s_curlVersion = {
	0, "0.0.0-nel-wasm-stub", 0, "wasm32-emscripten", 0,
	NULL, 0, NULL, NULL,
	{ NULL }
};

void *curl_easy_init(void) { return NULL; }
int curl_easy_setopt(void *h, int opt, ...) { (void)h; (void)opt; return 2 /* CURLE_FAILED_INIT */; }
int curl_easy_getinfo(void *h, int info, ...) { (void)h; (void)info; return 2; }
void curl_easy_cleanup(void *h) { (void)h; }
char *curl_easy_escape(void *h, const char *s, int l) { (void)h; (void)s; (void)l; return NULL; }
const char *curl_easy_strerror(int c) { (void)c; return "curl disabled (NeL wasm stub)"; }
void curl_free(void *p) { (void)p; }
long curl_getdate(const char *p, const void *unused) { (void)p; (void)unused; return -1; }
void *curl_multi_init(void) { return NULL; }
int curl_multi_add_handle(void *m, void *h) { (void)m; (void)h; return 1; }
int curl_multi_remove_handle(void *m, void *h) { (void)m; (void)h; return 1; }
int curl_multi_perform(void *m, int *running) { (void)m; if (running) *running = 0; return 1; }
void *curl_multi_info_read(void *m, int *queued) { (void)m; if (queued) *queued = 0; return NULL; }
int curl_multi_cleanup(void *m) { (void)m; return 0; }
void *curl_slist_append(void *l, const char *s) { (void)l; (void)s; return NULL; }
void curl_slist_free_all(void *l) { (void)l; }
void *curl_version_info(int age) { (void)age; return &s_curlVersion; }

/* ---- openssl (only reached through curl certificate plumbing) ---- */

int ASN1_STRING_to_UTF8(unsigned char **out, void *in) { (void)out; (void)in; return -1; }
void BIO_free(void *b) { (void)b; }
void *BIO_new_mem_buf(const void *buf, int len) { (void)buf; (void)len; return NULL; }
void CRYPTO_free(void *p, const char *f, int l) { (void)p; (void)f; (void)l; }
void ERR_error_string_n(unsigned long e, char *buf, size_t len) { (void)e; if (buf && len) buf[0] = 0; }
unsigned long ERR_get_error(void) { return 0; }
int OPENSSL_sk_num(const void *sk) { (void)sk; return 0; }
void OPENSSL_sk_pop_free(void *sk, void (*fn)(void *)) { (void)sk; (void)fn; }
void *OPENSSL_sk_value(const void *sk, int i) { (void)sk; (void)i; return NULL; }
void *PEM_X509_INFO_read_bio(void *bio, void *sk, void *cb, void *u) { (void)bio; (void)sk; (void)cb; (void)u; return NULL; }
void *SSL_CTX_get_cert_store(const void *ctx) { (void)ctx; return NULL; }
void X509_INFO_free(void *i) { (void)i; }
void *X509_NAME_ENTRY_get_data(void *e) { (void)e; return NULL; }
int X509_NAME_entry_count(const void *n) { (void)n; return 0; }
void *X509_NAME_get_entry(void *n, int loc) { (void)n; (void)loc; return NULL; }
int X509_STORE_add_cert(void *store, void *x) { (void)store; (void)x; return 0; }
void *X509_dup(void *x) { (void)x; return NULL; }
void X509_free(void *x) { (void)x; }
void *X509_get_subject_name(const void *x) { (void)x; return NULL; }

/* ---- SysV IPC (nelmisc shared memory, unused in wasm) ---- */

int semctl(int id, int num, int cmd, ...) { (void)id; (void)num; (void)cmd; return -1; }
int semget(int key, int nsems, int flg) { (void)key; (void)nsems; (void)flg; return -1; }
int semop(int id, void *ops, size_t n) { (void)id; (void)ops; (void)n; return -1; }
void *shmat(int id, const void *addr, int flg) { (void)id; (void)addr; (void)flg; return (void *)-1; }
int shmctl(int id, int cmd, void *buf) { (void)id; (void)cmd; (void)buf; return -1; }
int shmdt(const void *addr) { (void)addr; return -1; }
int shmget(int key, size_t size, int flg) { (void)key; (void)size; (void)flg; return -1; }
