ifeq ($(TGTDIR), )
TGTDIR	= $(shell pwd)/..
endif
 
INSDIR	= $(TGTDIR)/rootfs

export TGTDIR LNXDIR INSDIR

CROSS_COMPILE   = 

AS      = $(CROSS_COMPILE)as
LD      = $(CROSS_COMPILE)ld
CC      = $(CROSS_COMPILE)gcc
CPP     = $(CC) -E
AR      = $(CROSS_COMPILE)ar
NM      = $(CROSS_COMPILE)nm
STRIP   = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
RANLIB  = $(CROSS_COMPILE)ranlib

CFLAGS	+= -Wall  

VERBOSE	= 0 

ifeq ($(SRCS), )
SRCS    = $(wildcard *.c)
endif

OBJS    = $(patsubst %.c, %.o, $(SRCS))
DEPS    = $(patsubst %.c, .%.d, $(SRCS))

#.PHONY: $(SUBDIRS) $(DEPS) $(OBJS) $(EXEC) $(SLIB) $(SHLIB)

all: $(SUBDIRS) $(DEPS) $(OBJS) $(EXEC) $(SLIB) $(SHLIB)

ifeq ($(VERBOSE), 0)

%.o : %.c
	@echo -e "  CC\t$@"
	@$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

.%.d : %.c
	@echo -e "  DEP\t$@" 
	@$(CC) -c -MM $(CFLAGS) $(CPPFLAGS) $< -o $@

$(EXEC): $(OBJS)
	@echo -e "  LD\t$@"
	@$(CC) $(OBJS) $(LDFLAGS) -o $@

$(SHLIB): $(OBJS)
	@echo -e "  LD\t$@"
	@$(CC) $(OBJS) $(LDFLAGS) -o $@

$(SLIB): $(OBJS)
	@echo -e "  AR\t$@"
	@$(AR) cr $@ $(OBJS)
	@$(RANLIB) $@

$(SUBDIRS)::
	@echo -e "MAKE\t[$@]"
	@$(MAKE) --no-print-directory -C $@ 

else

%.o : %.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

.%.d : %.c
	$(CC) -c -MM $(CFLAGS) $(CPPFLAGS) $< -o $@

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(SHLIB): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(SLIB): $(OBJS)
	$(AR) cr $@ $(OBJS)
	$(RANLIB) $@

$(SUBDIRS)::
	$(MAKE) -C $@ 


endif

install_dirs := $(addprefix _install_, $(SUBDIRS))
$(install_dirs):
	@$(MAKE) -C $(patsubst _install_%, %, $@) install

install: $(install_dirs)
ifneq ($(EXEC), )
	cp $(EXEC) $(INSDIR)/xlrose
endif
ifneq ($(SHLIB), )
	cp $(SHLIB) $(INSDIR)/lib
endif

clean_dirs := $(addprefix _clean_, $(SUBDIRS))

$(clean_dirs):
	@$(MAKE) -C $(patsubst _clean_%, %, $@) clean

clean: $(clean_dirs)
	rm -f .*.d *.o *.a *.so $(EXEC) $(SHLIB) $(SLIB)


-include $(DEPS)
