#pragma once

#include "cetcd/cetcd.h"

char *etcd_get_record(cetcd_client *client, const char *name,const char *type);
