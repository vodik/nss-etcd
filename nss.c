#include <stdlib.h>
#include <stdio.h>

#include "cetcd/cetcd.h"
#include "cetcd/third-party/yajl-2.1.0/src/api/yajl_tree.h"

int main(int argc, char *argv[])
{
    cetcd_array addrs;

    cetcd_array_init(&addrs, 1);
    cetcd_array_append(&addrs, "vodik.qa.sangoma.local:2379");

    cetcd_client cli;
    cetcd_client_init(&cli, &addrs);

    cetcd_response *resp;
    resp = cetcd_get(&cli, "/address/local/sangoma/qa/tektite");
    if (resp->err) {
        printf("error :%d, %s (%s)\n", resp->err->ecode, resp->err->message, resp->err->cause);
    } else {
        printf("%s\n", resp->node->value);
    }

    yajl_val node;
    char errbuf[1024];

    node = yajl_tree_parse(resp->node->value, errbuf, sizeof(errbuf));
    if (!node) {
        printf("not found\n");
        return 0;
    }

    {
        const char* path[] = {"A", NULL};
        yajl_val v = yajl_tree_get(node, path, yajl_t_array);
        if (v) {
            size_t i;
            for (i = 0; i < v->u.array.len; ++i) {
                yajl_val obj = v->u.array.values[i];
                printf("%s: %s\n", path[0], YAJL_GET_STRING(obj));
            }
        } else {
            printf("no such node: %s\n", path[0]);
        }
    }

    {
        const char* path[] = {"AAAA", NULL};
        yajl_val v = yajl_tree_get(node, path, yajl_t_array);
        if (v) {
            size_t i;
            for (i = 0; i < v->u.array.len; ++i) {
                yajl_val obj = v->u.array.values[i];
                printf("%s: %s\n", path[0], YAJL_GET_STRING(obj));
            }
        } else {
            printf("no such node: %s\n", path[0]);
        }
    }

    cetcd_response_release(resp);
}
