#pragma once

#include <stddef.h>
#include <nss.h>
#include <netdb.h>

static const char *servers[] = {
    "vodik.qa.sangoma.local:2379",
    "glados.qa.sangoma.local:2379"
};

static size_t server_count = sizeof(servers) / sizeof(servers[0]);

enum nss_status _nss_etcd_init(void);
enum nss_status _nss_etcd_quit(void);
enum nss_status _nss_etcd_gethostbyname2_r(const char *name,
                                           int af,
                                           struct hostent *result,
                                           char *buffer,
                                           size_t buflen,
                                           int *errnop,
                                           int *h_errnop);
