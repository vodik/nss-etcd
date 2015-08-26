CFLAGS := -std=c11 -fPIC \
	-Wall -Wextra -pedantic \
	-Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes \
	-D_GNU_SOURCE \
	-ggdb3 -O0 \
	$(CFLAGS)

LDFLAGS= -Lcetcd

LDLIBS=-lcetcd -lcurl

MODULE = libnss_etcd.so.2

all: nss #$(MODULE)
nss: nss.o

$(MODULE): nss.o
	$(CC) -fPIC -Wall -shared -o $(MODULE) -Wl,-soname,$(MODULE) $^

clean:
	$(RM) search *.o

.PHONY: clean
