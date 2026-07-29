# Makefile for decb
include ../rules.mak

vpath %.c ../../../decb ../../../os9

CFLAGS	+= -I../../../include -Wall -MMD -MP

SRCS = decb_main.c decbattr.c decbcopy.c decbdir.c decbdskini.c decbfree.c decbfstat.c decbhdbconv.c decbkill.c decblist.c decbrename.c os9dump.c decbdsave.c decbbinbust.c os9dsave.c

OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

LIBDEPS = libtoolshed.a libcoco.a libnative.a libcecb.a librbf.a libdecb.a libmisc.a libsys.a
LIBPATHS = $(patsubst %,../%/$$(notdir %),$(patsubst %.a,%,$(LIBDEPS)))
LIBPATHS = $(foreach lib,$(LIBDEPS),../$(basename $(lib))/$(lib))
LDFLAGS_DIRS = $(foreach lib,$(LIBDEPS),-L../$(basename $(lib)))
LDFLAGS_LIBS = $(patsubst lib%.a,-l%,$(LIBDEPS))
override LDFLAGS += $(LDFLAGS_DIRS) $(LDFLAGS_LIBS) -lm

decb$(SUFEXE): $(OBJS) $(LIBPATHS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	$(RM) *.o *.d decb decb.exe

-include $(DEPS)
