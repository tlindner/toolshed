include ../rules.mak

vpath %.c ../../../tocgen
vpath %.h ../../../tocgen

CFLAGS  += -I../../../include -Wall
BINARY	= tocgen
OBJS	= tocgen_main.o

LIBDEPS = libtoolshed.a libcoco.a libnative.a libcecb.a librbf.a libdecb.a libmisc.a libsys.a
LIBPATHS = $(patsubst %,../%/$$(notdir %),$(patsubst %.a,%,$(LIBDEPS)))
LIBPATHS = $(foreach lib,$(LIBDEPS),../$(basename $(lib))/$(lib))
LDFLAGS_DIRS = $(foreach lib,$(LIBDEPS),-L../$(basename $(lib)))
LDFLAGS_LIBS = $(patsubst lib%.a,-l%,$(LIBDEPS))
override LDFLAGS += $(LDFLAGS_DIRS) $(LDFLAGS_LIBS) -lm

$(BINARY)$(SUFEXE): $(OBJS) $(LIBPATHS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)  $(DEBUG)

$(OBJS):	Makefile

clean:
	$(RM) $(BINARY) $(BINARY).exe *.o

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin


