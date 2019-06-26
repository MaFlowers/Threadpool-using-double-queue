EXEC = main
#SHLIB = libcthread.so
TGTDIR = ..

CFLAGS += -I ./ -I ../_CThread_pool_master -g
#CFLAGS += -I ./ -I ../_CThread_pool -g

LDFLAGS += -L ./ -L ../_CThread_pool_master \
	   -lcthread

#LDFLAGS += -shared -L ./ \
	   -lpthread

include $(TGTDIR)/Makefile.inc
