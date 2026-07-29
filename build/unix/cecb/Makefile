# Makefile for cecb
include ../rules.mak

vpath %.c ../../../cecb ../../../os9

CFLAGS	+= -I../../../include -Wall -MMD -MP

# one line
SRCS = cecbbulkerase.c cecbdir.c cecbfstat.c cecb_main.c cecbcopy.c cecbleader.c cecbdumpblock.c ../os9/os9dump.c ../decb/decblist.c
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

# one line
LIBDEPS = libtoolshed.a libcoco.a libnative.a libcecb.a librbf.a libdecb.a libmisc.a libsys.a
LIBPATHS = $(patsubst %,../%/$$(notdir %),$(patsubst %.a,%,$(LIBDEPS)))
LIBPATHS = $(foreach lib,$(LIBDEPS),../$(basename $(lib))/$(lib))
LDFLAGS_DIRS = $(foreach lib,$(LIBDEPS),-L../$(basename $(lib)))
LDFLAGS_LIBS = $(patsubst lib%.a,-l%,$(LIBDEPS))
override LDFLAGS += $(LDFLAGS_DIRS) $(LDFLAGS_LIBS) -lm

cecb$(SUFEXE): $(OBJS) $(LIBPATHS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	$(RM) *.o *.d cecb cecb.exe

-include $(DEPS)
