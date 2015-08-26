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

    char *p = keyfile;

    *p++ = '/';
    p = mempcpy(p, prefix, prefix_len);
    *p++ = '/';

    const char *f = name;

    for (;;) {
        const char *point = memrchr(f, '.', name_len);
        if (!point) {
            p = mempcpy(p, f, name_len);
            *p = 0;
            break;
        }

        point++;
        size_t end_len = name_len - (point - f);

        p = mempcpy(p, point, end_len);
        *p++ = '/';

        name_len -= end_len + 1;
    }

    return keyfile;
}

char *etcd_get_record(cetcd_client *client, const char *name,const char *type)
{
    char *keyfile = name_to_key("address", name);
    cetcd_response *resp = cetcd_get(client, keyfile);
    free(keyfile);
    if (!resp || resp->err) {
        return NULL;
    }

    char errbuf[1024];
    yajl_val node = yajl_tree_parse(resp->node->value, errbuf, sizeof(errbuf));
    if (!node) {
        return NULL;
    }

    const char* path[] = {type, NULL};
    yajl_val v = yajl_tree_get(node, path, yajl_t_array);
    if (v) {
        yajl_val obj = v->u.array.values[0];
        if (!obj)
            return NULL;

        return strdup(YAJL_GET_STRING(obj));
    } else {
        return NULL;
    }

    cetcd_response_release(resp);
}
