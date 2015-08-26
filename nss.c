#include "nss.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <cetcd.h>
#include <assert.h>

#include "etcd.h"

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

static char *etcd_lookup(const char *name, const char *type)
{
    static const char *servers[] = {
        "vodik.qa.sangoma.local:2379",
        "glados.qa.sangoma.local:2379"
    };
    static size_t server_count = sizeof(servers) / sizeof(servers[0]);

    cetcd_array addrs;
    cetcd_array_init(&addrs, server_count);
    for (size_t idx = 0; idx < server_count; ++idx)
        cetcd_array_append(&addrs, servers[idx]);

    cetcd_client client;
    cetcd_client_init(&client, &addrs);

    return etcd_get_record(&client, name, type);
}

enum nss_status _nss_etcd_gethostbyname2_r(const char *name,
                                           int af,
                                           struct hostent *result,
                                           char *buffer,
                                           size_t buflen,
                                           int *errnop,
                                           int *h_errnop)
{
    struct nss_buf buf;
    nss_alloc_init(&buf, buffer, buflen);

    char *record = etcd_lookup(name, af == AF_INET ? "A" : "AAAA");
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
    strncpy(r_name, name, name_len);

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
