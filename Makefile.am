AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS =
EXTRA_DIST =

AM_CFLAGS  = -std=c99
AM_CFLAGS += -I$(top_srcdir)/contrib -I$(top_srcdir)/src
AM_CFLAGS += -I/usr/include/libzfs -I/usr/include/libzfs/sys -I/usr/include/libzfs/linux -I/usr/include/libspl -DHAVE_IOCTL_IN_UNISTD_H

AM_LDFLAGS =

noinst_PROGRAMS =
bin_PROGRAMS =
lib_LTLIBRARIES =
man_MANS =
noinst_LTLIBRARIES =

EXTRA_DIST += $(shell git ls-files)

include $(top_srcdir)/contrib/Makefile.am
include $(top_srcdir)/src/Makefile.am
include $(top_srcdir)/src/ganglia/Makefile.am



DEBSOURCEPKG=$(PACKAGE)_$(VERSION).orig.tar.gz
DEBSOURCEDIR=$(PACKAGE)-$(VERSION)
deb: dist
	@$(ECHO) "Building Debian package..."; \
	if $(TEST) ! -d debian; then \
		$(ECHO) "The files needed for building a Debian package are not" \
		"included by default in the distribution. To build a package, check" \
		"out the project from source control."; \
		exit 1; \
	fi; \
	$(MV) $(PACKAGE)-$(VERSION).tar.gz $(DEBSOURCEPKG) && \
	$(TAR) --extract --gunzip --file $(DEBSOURCEPKG) && \
	$(MKDIR_P) $(DEBSOURCEDIR)/debian && \
	$(CP) \
		debian/changelog \
		debian/compat \
		debian/control \
		debian/copyright \
		debian/docs \
		debian/rules \
		$(DEBSOURCEDIR)/debian && \
	cd $(DEBSOURCEDIR) && \
	DEB_DH_SHLIBDEPS_ARGS_ALL=--dpkg-shlibdeps-params=--ignore-missing-info \
	$(DEBUILD) -rfakeroot -D -us -uc; \
	cd $(top_srcdir); \
	$(ECHO) "Clean up..."; \
	rm -rf $(DEBSOURCEPKG); \
	rm -rf $(DEBSOURCEDIR)
