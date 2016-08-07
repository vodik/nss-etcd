#pragma once

#include <stddef.h>
#include <nss.h>
#include <netdb.h>

static const char *servers[] = {
    "chaos.qa.sangoma.local:2379",
};

static size_t server_count = sizeof(servers) / sizeof(servers[0]);

enum nss_status _nss_etcd_quit(void);
enum nss_status _nss_etcd_gethostbyname2_r(const char *name,
                                           int af,
                                           struct hostent *result,
                                           char *buffer,
                                           size_t buflen,
                                           int *errnop,
                                           int *h_errnop);
enum nss_status _nss_etcd_gethostbyname4_r(const char *name,
                                           struct gaih_addrtuple **pat,
                                           char *buffer, size_t buflen,
                                           int *errnop, int *h_errnop,
                                           int32_t *ttlp);
