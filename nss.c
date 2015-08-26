#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <nss.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "cetcd/cetcd.h"
#include "cetcd/third-party/yajl-2.1.0/src/api/yajl_tree.h"

#define IN6ADDRSZ 16
#define INADDRSZ 4


enum nss_status _nss_blacklist_gethostbyname2_r(const char *name,
                                                int af,
                                                struct hostent *result,
                                                char *buffer,
                                                size_t buflen,
                                                int *errnop,
                                                int *h_errnop);

/* static inline size_t align_ptr(size_t l) */
/* { */
/*     const size_t a = sizeof(void *) - 1; */
/*     return (l + a) & ~a; */
/* } */

static void cetcd_setup(cetcd_client *client)
{
    cetcd_array addrs;
    cetcd_array_init(&addrs, 1);
    cetcd_array_append(&addrs, "vodik.qa.sangoma.local:2379");

    cetcd_client_init(client, &addrs);
}

enum nss_status _nss_blacklist_gethostbyname2_r(const char *name,
                                                int af,
                                                struct hostent *result,
                                                char *buffer,
                                                size_t buflen,
                                                int *errnop,
                                                int *h_errnop)
{
    cetcd_client client;
    cetcd_response *resp;

    cetcd_setup(&client);
    resp = cetcd_get(&client, "/address/local/sangoma/qa/tektite");
    if (resp->err) {
        *errnop = ESRCH;
        *h_errnop = HOST_NOT_FOUND;
        return NSS_STATUS_NOTFOUND;
    }

    memset(buffer, '\0', buflen);

    size_t idx = 0;
    size_t addrlen;
    char *r_name, **r_aliases, *r_addr, **r_addr_list;

    size_t l = strlen(name);

    char errbuf[1024];
    yajl_val node = yajl_tree_parse(resp->node->value, errbuf, sizeof(errbuf));
    if (!node) {
        *errnop = ESRCH;
        *h_errnop = HOST_NOT_FOUND;
        return NSS_STATUS_NOTFOUND;
    }

    const char* path[] = {af == AF_INET ? "A" : "AAAA", NULL};
    yajl_val v = yajl_tree_get(node, path, yajl_t_array);
    if (v) {
        yajl_val obj = v->u.array.values[0];

        addrlen = af == AF_INET ? INADDRSZ : IN6ADDRSZ;
        r_addr = buffer;
        inet_pton(af, YAJL_GET_STRING(obj), r_addr);
    } else {
        *errnop = ESRCH;
        *h_errnop = HOST_NOT_FOUND;
        return NSS_STATUS_NOTFOUND;
    }
    cetcd_response_release(resp);

    /* Second, place the address list */
    r_addr_list = (char **)(buffer + idx);
    r_addr_list[0] = r_addr;
    r_addr_list[1] = NULL;
    idx += sizeof(char *) * 2;

    /* Third, place the alias list */
    r_aliases = (char **)(buffer + idx);
    r_aliases[0] = NULL;
    idx += sizeof(char *);

    r_name = buffer + idx;
    memcpy(r_name, name, l + 1);

    result->h_name = r_name;
    result->h_aliases = (char **)r_aliases;
    result->h_addrtype = af;
    result->h_length = addrlen;
    result->h_addr_list = (char **)r_addr_list;

    *errnop = 0;
    *h_errnop = NETDB_SUCCESS;
    h_errno = 0;

    return NSS_STATUS_SUCCESS;
}
