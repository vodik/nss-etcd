#include "etcd.h"

#include <string.h>
#include <cetcd.h>
#include <yajl/yajl_tree.h>

static char *name_to_key(const char *prefix, const char *name)
{
    size_t name_len = strlen(name);
    size_t prefix_len = strlen(prefix);
    char *keyfile = malloc(name_len + prefix_len + 3);
    if (!keyfile) {
        return NULL;
    }

    char *ptr = keyfile;
    *ptr++ = '/';
    ptr = mempcpy(ptr, prefix, prefix_len);
    *ptr++ = '/';

    const char *dot;
    while ((dot = memrchr(name, '.', name_len))) {
        dot++;

        size_t end_len = name_len - (dot - name);
        ptr = mempcpy(ptr, dot, end_len);
        *ptr++ = '/';

        name_len -= end_len + 1;
    }

    ptr = mempcpy(ptr, name, name_len);
    *ptr = '\0';
    return keyfile;
}

char *etcd_get_record(cetcd_client *client, const char *name,const char *type)
{
    char *keyfile = name_to_key("address", name);
    cetcd_response *resp = cetcd_get(client, keyfile);
    free(keyfile);

    if (!resp) {
        return NULL;
    } else if (resp->err) {
        cetcd_response_release(resp);
        return NULL;
    }

    char errbuf[1024];
    yajl_val node = yajl_tree_parse(resp->node->value, errbuf, sizeof(errbuf));
    if (!node) {
        cetcd_response_release(resp);
        return NULL;
    }

    char *record = NULL;
    const char* path[] = {type, NULL};

    yajl_val v = yajl_tree_get(node, path, yajl_t_array);
    if (v) {
        yajl_val obj = v->u.array.values[0];

        if (obj) {
            record = strdup(YAJL_GET_STRING(obj));
        }
    }

    yajl_tree_free(node);
    cetcd_response_release(resp);
    return record;
}
