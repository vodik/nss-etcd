CFLAGS := -std=c11 -fPIC \
	-Wall -Wextra -pedantic \
	-Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes \
	-D_GNU_SOURCE \
	-ggdb3 -O0 \
	$(CFLAGS)

LDLIBS= -lcetcd -lcurl

all: libnss_etcd.so.2 etcdlookup
etcdlookup: nss.o etcd.o etcdlookup.o
libnss_etcd.so.2: nss.o etcd.o
	$(CC) -fPIC -Wall -shared -o libnss_etcd.so.2 -Wl,-soname,libnss_etcd.so.2 $^ $(LDLIBS)

clean:
	$(RM) etcdlookup libnss_etcd.so.2 *.o

.PHONY: clean
