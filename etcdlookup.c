#include <stdlib.h>
#include <stdio.h>
#include <resolv.h>
#include <arpa/inet.h>

#include "nss.h"

int main(int argc, char *argv[])
{
    const char *name = argv[1];
    if (argc != 2)
        return 1;

    struct hostent result;
    char buffer[2048];
    int errnop, h_errnop;

    _nss_etcd_init();
    enum nss_status status = _nss_etcd_gethostbyname2_r(name,
                                                        AF_INET,
                                                        &result,
                                                        buffer,
                                                        sizeof(buffer),
                                                        &errnop,
                                                        &h_errnop);
    _nss_etcd_quit();

    switch (status) {
    case NSS_STATUS_SUCCESS:
        printf("successfully resolved\n");

        char address[IN6ADDRSZ];
        inet_ntop(result.h_addrtype,
                  result.h_addr_list[0],
                  address,
                  sizeof(address));
        printf("%s -> %s\n", address, name);
        break;
    default:
        break;
    }
}
