#EXEC = main
SHLIB = libcthread.so
TGTDIR = ..

CFLAGS += -I ./ -g -fPIC -D HAVE_THREAD_POOL_DEBUG

#LDFLAGS += -L ./ \
	   -lpthread

LDFLAGS += -shared -L ./ \
	   -lpthread

include $(TGTDIR)/Makefile.inc
