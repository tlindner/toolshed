# Makefile for ToolShed

include rules.mak

vpath %.c ../../mamou:../../os9

PACKAGENAME_UNIX	= toolshed-unix64-$(VERSION).tgz
PACKAGENAME_WIN		= toolshed-win64-$(VERSION).zip

prefix      ?= /usr/local
bindir      ?= $(prefix)/bin
docdir      ?= $(prefix)/share/doc/toolshed

ifneq ($(DESTDIR),)
INSTALLDIR  = $(DESTDIR)$(bindir)
DOCDIR      = $(DESTDIR)$(docdir)
else
INSTALLDIR  = $(bindir)
DOCDIR      = $(docdir)
endif

APPS 		= ar2 os9 mamou cecb decb tocgen makewav dis68 lst2cmt dgndos

DIRS		= libtoolshed libnative libcecb librbf libcoco libdecb libdiskimage libdragondos libmisc libsys \
		decb cecb os9 makewav tocgen lst2cmt dgndos \
		dis68 mamou ar2 \
		doc

ifneq ($(WIN),)
PACKAGENAME	= $(PACKAGENAME_WIN)
MAKEPACKAGE = zip -9
else
PACKAGENAME	= $(PACKAGENAME_UNIX)
MAKEPACKAGE = tar czvf
ifneq ($(NO_COCOFUSE),)
DIRS		:= $(DIRS) unittest
else
DIRS		:= $(DIRS) cocofuse unittest
APPS		:= $(APPS) cocofuse
endif
endif

LIBDIRS		= $(filter lib%,$(DIRS))
APPDIRS		= $(filter-out $(LIBDIRS) doc unittest,$(DIRS))
CLEANDIRS	= $(addprefix clean-,$(DIRS))

APPS		:= $(foreach item,$(APPS),$(item)/$(item)$(SUFEXE))

.PHONY: all $(DIRS) install package cleandirs $(CLEANDIRS) clean \
	unixpackage winpackage allpackages

all: $(DIRS)

# Recursive makes share the jobserver.  Libraries must be ready before
# linking applications, and command tests need the applications built first.
$(APPDIRS): $(LIBDIRS)
unittest: $(LIBDIRS) $(APPDIRS)

$(DIRS):
	$(MAKE) -C $@

install: all
	$(INSTALL) -d $(INSTALLDIR) $(DOCDIR)
	$(INSTALL) $(APPS) $(INSTALLDIR)
	$(INSTALL) -m 0644 doc/ToolShed.html $(DOCDIR)

package: INSTALLDIR=toolshed-$(VERSION)
package: DOCDIR = $(INSTALLDIR)
package: install
	$(MAKEPACKAGE) $(PACKAGENAME) toolshed-$(VERSION)/*
	rm -rf toolshed-$(VERSION)

cleandirs: $(CLEANDIRS)

$(CLEANDIRS):
	$(MAKE) -C $(patsubst clean-%,%,$@) clean
	
clean: cleandirs
	$(RM) $(PACKAGENAME_WIN) $(PACKAGENAME_UNIX)

unixpackage:
	$(MAKE) -f ./Makefile package

winpackage:
	$(MAKE) -f ./Makefile cleandirs
	$(MAKE) -f ./Makefile CC=x86_64-w64-mingw32-gcc WIN=1 package

# Both packages use the same build tree, so do not run them concurrently.
allpackages:
	$(MAKE) clean
	$(MAKE) unixpackage
	$(MAKE) winpackage


