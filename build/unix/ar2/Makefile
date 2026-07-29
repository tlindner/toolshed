include ../rules.mak

vpath %.c ../../../ar2
vpath %.h ../../../ar2

BINARY	= ar2

override CFLAGS	+= -DSYSV

SRCS = ar.c arsup.c lz1.c o2u.c
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

$(BINARY)$(SUFEXE): $(OBJS) $(LIBPATHS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS) $(DEBUG)

clean:
	$(RM) *.o *.d os9 os9.exe

install: $(BINARY)
	cp $(BINARY) $(HOME)/bin

-include $(DEPS)
