#include "nss.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <resolv.h>
#include <arpa/inet.h>

#include "etcd.h"
#include "cetcd/cetcd.h"
#include "cetcd/third-party/yajl-2.1.0/src/api/yajl_tree.h"

/* static inline size_t align_ptr(size_t l) */
/* { */
/*     const size_t a = sizeof(void *) - 1; */
/*     return (l + a) & ~a; */
/* } */

enum nss_status _nss_etcd_gethostbyname2_r(const char *name,
                                           int af,
                                           struct hostent *result,
                                           char *buffer,
                                           size_t buflen,
                                           int *errnop,
                                           int *h_errnop)
{
    cetcd_array addrs;
    cetcd_array_init(&addrs, 1);
    cetcd_array_append(&addrs, "vodik.qa.sangoma.local:2379");

    cetcd_client client;
    cetcd_client_init(&client, &addrs);

    char *record = etcd_get_record(&client, name,
                                   af == AF_INET ? "A" : "AAAA");

    if (!record) {
        *errnop = ESRCH;
        *h_errnop = HOST_NOT_FOUND;
        return NSS_STATUS_NOTFOUND;
    }

    size_t idx = 0;
    char *r_name, **r_aliases, *r_addr, **r_addr_list;

    size_t name_len = strlen(name);
    r_addr = buffer;

    size_t addrlen = af == AF_INET ? INADDRSZ : IN6ADDRSZ;
    inet_pton(af, record, r_addr);
    free(record);

    /* Second, place the address list */
    buffer += addrlen;
    r_addr_list = (char **)(buffer + idx);
    r_addr_list[0] = r_addr;
    r_addr_list[1] = NULL;
    idx += sizeof(char *) * 2;

    /* Third, place the alias list */
    r_aliases = (char **)(buffer + idx);
    r_aliases[0] = NULL;
    idx += sizeof(char *);

    r_name = buffer + idx;
    memcpy(r_name, name, name_len + 1);

    result->h_name = r_name;
    result->h_aliases = (char **)r_aliases;
    result->h_addrtype = af;
    result->h_length = addrlen;
    result->h_addr_list = (char **)r_addr_list;

    *errnop = 0;
    *h_errnop = NETDB_SUCCESS;
    return NSS_STATUS_SUCCESS;
}
