# Makefile for mamou
include ../rules.mak

vpath %.c ../../../mamou

DEBUG	= -g
CFLAGS	+= -DLINUX -I../../../include $(DEBUG) -Wall
SRCS = mamou_main.c evaluator.c pseudo.c h6309.c ffwd.c print.c util.c symbol_bucket.c
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)
LIBDEPS = libcoco.a libnative.a libcecb.a librbf.a libdecb.a libmisc.a libsys.a
LIBPATHS = $(patsubst %,../%/$$(notdir %),$(patsubst %.a,%,$(LIBDEPS)))
LIBPATHS = $(foreach lib,$(LIBDEPS),../$(basename $(lib))/$(lib))
LDFLAGS_DIRS = $(foreach lib,$(LIBDEPS),-L../$(basename $(lib)))
LDFLAGS_LIBS = $(patsubst lib%.a,-l%,$(LIBDEPS))
override LDFLAGS += $(LDFLAGS_DIRS) $(LDFLAGS_LIBS) -lm $(DEBUG)

mamou$(SUFEXE): $(OBJS) $(LIBPATHS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	$(RM) *.o mamou mamou.exe

install: mamou
	cp mamou $(HOME)/bin

-include $(DEPS)
