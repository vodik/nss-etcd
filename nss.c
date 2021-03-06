#include "nss.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <cetcd.h>
#include <assert.h>

#include "etcd.h"

static bool cetcd_initialized = false;
static cetcd_client client = {};
static cetcd_array addrs = {};

struct nss_buf {
    char *block;
    size_t len;
    size_t offset;
};

static inline size_t align_ptr(size_t l)
{
    const size_t a = sizeof(void *) - 1;
    return (l + a) & ~a;
}

static void nss_alloc_init(struct nss_buf *buf, char *backing, size_t len)
{
    *buf = (struct nss_buf){
        .block = backing,
        .len = len
    };
}

static void *nss_alloc(struct nss_buf *buf, size_t len)
{
    void *mem = &buf->block[buf->offset];
    buf->offset = align_ptr(buf->offset + len);
    assert(buf->offset <= buf->len);
    return mem;
}

/* enum nss_status _nss_etcd_init(void) */
static cetcd_client *etcd_init(void)
{
    if (!cetcd_initialized) {
        cetcd_array_init(&addrs, server_count);
        for (size_t idx = 0; idx < server_count; ++idx)
            cetcd_array_append(&addrs, servers[idx]);

        cetcd_client_init(&client, &addrs);
    }

    cetcd_initialized = true;
    return &client;
}

enum nss_status _nss_etcd_quit(void)
{
    if (cetcd_initialized) {
        cetcd_array_destroy(&addrs);
        cetcd_client_destroy(&client);
    }

    cetcd_initialized = false;
    return NSS_STATUS_SUCCESS;
}

enum nss_status _nss_etcd_gethostbyname4_r(const char *name,
                                           struct gaih_addrtuple **pat,
                                           char *buffer, size_t buflen,
                                           int *errnop, int *h_errnop,
                                           int32_t *ttlp)
{
    cetcd_client *_client = etcd_init();

    struct nss_buf buf;
    nss_alloc_init(&buf, buffer, buflen);

    char *record = etcd_get_record(_client, name, "A");
    if (!record) {
        *errnop = ESRCH;
        *h_errnop = HOST_NOT_FOUND;
        return NSS_STATUS_NOTFOUND;
    }

    size_t name_len = strlen(name);
    char *r_name = nss_alloc(&buf, name_len + 1);
    memcpy(r_name, name, name_len);
    r_name[name_len] = '\0';

    struct gaih_addrtuple *r_tuple = nss_alloc(&buf, sizeof(*r_tuple));
    *r_tuple = (struct gaih_addrtuple){
        .next = NULL,
        .name = r_name,
        .family = AF_INET,
        .scopeid = 0
    };
    inet_pton(AF_INET, record, r_tuple->addr);

    /* Nscd expects us to store the first record in **pat. */
    if (*pat) {
        printf("poop\n");
        **pat = *r_tuple;
    } else {
        printf("poop2\n");
        *pat = r_tuple;
    }

    if (ttlp)
        *ttlp = 0;

    *errnop = 0;
    *h_errnop = NETDB_SUCCESS;
    h_errno = 0;

    return NSS_STATUS_SUCCESS;

}

enum nss_status _nss_etcd_gethostbyname2_r(const char *name,
                                           int af, struct hostent *result,
                                           char *buffer, size_t buflen,
                                           int *errnop, int *h_errnop)
{
    etcd_init();

    struct nss_buf buf;
    nss_alloc_init(&buf, buffer, buflen);

    char *record = etcd_get_record(&client, name, af == AF_INET ? "A" : "AAAA");
    if (!record) {
        *errnop = ESRCH;
        *h_errnop = HOST_NOT_FOUND;
        return NSS_STATUS_NOTFOUND;
    }

    /* Place the address */
    size_t addrlen = af == AF_INET ? INADDRSZ : IN6ADDRSZ;
    char *r_addr = nss_alloc(&buf, addrlen);
    inet_pton(af, record, r_addr);
    free(record);

    /* Second, place the address list */
    char **r_addr_list = nss_alloc(&buf, sizeof(char *) * 2);
    r_addr_list[0] = r_addr;
    r_addr_list[1] = NULL;

    /* Third, place the alias list */
    char **r_aliases = nss_alloc(&buf, sizeof(char *));
    r_aliases[0] = NULL;

    size_t name_len = strlen(name);
    char *r_name = nss_alloc(&buf, name_len + 1);
    memcpy(r_name, name, name_len);
    r_name[name_len] = '\0';

    *result = (struct hostent){
        .h_name = r_name,
        .h_aliases = r_aliases,
        .h_addrtype = af,
        .h_length = addrlen,
        .h_addr_list = r_addr_list,
    };

    *errnop = 0;
    *h_errnop = NETDB_SUCCESS;
    return NSS_STATUS_SUCCESS;
}
