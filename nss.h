#pragma once

#include <stddef.h>
#include <nss.h>
#include <netdb.h>

enum nss_status _nss_etcd_init(void);
enum nss_status _nss_etcd_quit(void);
enum nss_status _nss_etcd_gethostbyname2_r(const char *name,
                                           int af,
                                           struct hostent *result,
                                           char *buffer,
                                           size_t buflen,
                                           int *errnop,
                                           int *h_errnop);
