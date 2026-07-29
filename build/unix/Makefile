# Makefile for ToolShed

include rules.mak

vpath %.c ../../mamou:../../os9

PACKAGENAME_UNIX	= toolshed-unix64-$(VERSION).tgz
PACKAGENAME_WIN		= toolshed-win64-$(VERSION).zip
PACKAGENAME_MAC_UNIVERSAL = toolshed-mac-universal-$(VERSION).zip

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

APPS 		= ar2 os9 mamou cecb decb tocgen makewav dis68 lst2cmt cocofuse

DIRS		= libtoolshed libnative libcecb librbf libcoco libdecb libmisc libsys \
		decb cecb os9 makewav tocgen lst2cmt \
		dis68 mamou ar2 cocofuse unittest\
		doc

ifneq ($(WIN),)
PACKAGENAME	= $(PACKAGENAME_WIN)
MAKEPACKAGE = zip -9
else
PACKAGENAME	?= $(PACKAGENAME_UNIX)
MAKEPACKAGE ?= tar czvf

ifneq ($(NOUNITTESTS),)
    DIRS := $(filter-out unittest,$(DIRS))
endif

# Filter out cocofuse if NO_COCOFUSE is passed (e.g., NO_COCOFUSE=1)
ifneq ($(NO_COCOFUSE),)
    DIRS := $(filter-out cocofuse,$(DIRS))
    APPS := $(filter-out cocofuse/cocofuse cocofuse,$(APPS))
endif

endif

APPS		:= $(foreach item,$(APPS),$(item)/$(item)$(SUFEXE))

all:
	$(foreach dir, $(DIRS), (cd $(dir); $(MAKE));)

install: all
	$(INSTALL) -d $(INSTALLDIR) $(DOCDIR)
	$(INSTALL) $(APPS) $(INSTALLDIR)
	$(INSTALL) -m 0644 doc/ToolShed.html $(DOCDIR)

package: INSTALLDIR=toolshed-$(VERSION)
package: DOCDIR = $(INSTALLDIR)
package: install
	$(MAKEPACKAGE) $(PACKAGENAME) toolshed-$(VERSION)/*
	rm -rf toolshed-$(VERSION)

cleandirs:
	$(foreach dir, $(DIRS), (cd $(dir); $(MAKE) clean);)
	
clean: cleandirs
	$(RM) $(PACKAGENAME_WIN) $(PACKAGENAME_UNIX) $(PACKAGENAME_MAC_UNIVERSAL)

unixpackage:
	$(MAKE) -f ./Makefile package

winpackage:
	$(MAKE) -f ./Makefile cleandirs
	$(MAKE) -f ./Makefile CC=x86_64-w64-mingw32-gcc WIN=1 package

# Exclude cocofuse and unittest binaries from packaging loops
RELEASE_APPS = $(filter-out cocofuse/cocofuse cocofuse unittest/unittest unittest,$(APPS))

macpackage-universal:
	# 1. Clean & build x86_64 executables (skipping cocofuse and unittests)
	$(MAKE) -f ./Makefile cleandirs
	$(MAKE) -f ./Makefile NO_COCOFUSE=1 NOUNITTESTS=1 CFLAGS="$(CFLAGS) -arch x86_64" LDFLAGS="$(LDFLAGS) -arch x86_64" all
	mkdir -p bin_x86
	for app in $(RELEASE_APPS); do \
		cp $$app bin_x86/$$(basename $$app); \
	done

	# 2. Clean & build arm64 executables (skipping cocofuse and unittests)
	$(MAKE) -f ./Makefile cleandirs
	$(MAKE) -f ./Makefile NO_COCOFUSE=1 NOUNITTESTS=1 CFLAGS="$(CFLAGS) -arch arm64" LDFLAGS="$(LDFLAGS) -arch arm64" all
	mkdir -p bin_arm
	for app in $(RELEASE_APPS); do \
		cp $$app bin_arm/$$(basename $$app); \
	done

	# 3. Merge architecture binaries with lipo
	mkdir -p toolshed-$(VERSION)
	for app in $(RELEASE_APPS); do \
		exe=$$(basename $$app); \
		lipo -create bin_x86/$$exe bin_arm/$$exe -output toolshed-$(VERSION)/$$exe; \
	done

	# 4. Copy docs, zip package, and clean staging directories
	cp doc/ToolShed.html toolshed-$(VERSION)/ 2>/dev/null || true
	zip -9 $(PACKAGENAME_MAC_UNIVERSAL) toolshed-$(VERSION)/*
	rm -rf bin_x86 bin_arm toolshed-$(VERSION)

allpackages: clean unixpackage winpackage macpackage-universal
