# Makefile for os9
include ../rules.mak

vpath %.c ../../../os9

CFLAGS	+= -I../../../include -Wall -MMD -MP

# one line
SRCS = os9copy.c os9dsave.c os9gen.c os9modbust.c os9dcheck.c os9dump.c os9id.c os9padrom.c os9_main.c os9del.c os9format.c os9ident.c os9rename.c os9attr.c os9deldir.c os9free.c os9list.c os9cmp.c os9dir.c os9fstat.c os9makdir.c os9reveal.c
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

# one line
LIBDEPS = libtoolshed.a libcoco.a libnative.a libcecb.a librbf.a libdecb.a libmisc.a libsys.a
LIBPATHS = $(patsubst %,../%/$$(notdir %),$(patsubst %.a,%,$(LIBDEPS)))
LIBPATHS = $(foreach lib,$(LIBDEPS),../$(basename $(lib))/$(lib))
LDFLAGS_DIRS = $(foreach lib,$(LIBDEPS),-L../$(basename $(lib)))
LDFLAGS_LIBS = $(patsubst lib%.a,-l%,$(LIBDEPS))
override LDFLAGS += $(LDFLAGS_DIRS) $(LDFLAGS_LIBS) -lm

os9$(SUFEXE): $(OBJS) $(LIBPATHS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	$(RM) *.o *.d os9 os9.exe

-include $(DEPS)
