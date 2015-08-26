CFLAGS := -std=c11 -fPIC \
	-Wall -Wextra -pedantic \
	-Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes \
	-D_GNU_SOURCE \
	-ggdb3 -O0 \
	$(CFLAGS)

LDLIBS= -lcetcd -lcurl
MODULE = libnss_etcd.so.2

$(MODULE): nss.o etcd.o
	$(CC) -fPIC -Wall -shared -o $(MODULE) -Wl,-soname,$(MODULE) $^ $(LDLIBS)

clean:
	$(RM) search *.o

.PHONY: clean
