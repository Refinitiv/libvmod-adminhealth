AM_CPPFLAGS = -I$(VARNISHSRC)/include -I$(VARNISHSRC) -I$(VARNISHSRC)/bin/varnishd/

if VARNISH3
AM_CPPFLAGS+=-I$(VARNISHSRC)/bin/varnishd/cache
endif

vmoddir = $(VMODDIR)
vmod_LTLIBRARIES = libvmod_adminhealth.la

libvmod_adminhealth_la_LDFLAGS = -module -export-dynamic -avoid-version

libvmod_adminhealth_la_SOURCES = \
	vcc_if.c \
	vcc_if.h \
	vmod_adminhealth.c

vcc_if.c: vcc_if.h

vcc_if.h: @VMODTOOL@ $(top_srcdir)/src/vmod_adminhealth.vcc
	@VMODTOOL@ $(top_srcdir)/src/vmod_adminhealth.vcc

VMOD_TESTS = tests/*.vtc
.PHONY: $(VMOD_TESTS)

tests/*.vtc:
	$(VARNISHSRC)/bin/varnishtest/varnishtest -l -Dvarnishd=$(VARNISHSRC)/bin/varnishd/varnishd -Dvmod_topbuild=$(abs_top_builddir) $@

check: $(VMOD_TESTS)

EXTRA_DIST = \
	vmod_adminhealth.vcc \
	$(VMOD_TESTS)

CLEANFILES = $(builddir)/vcc_if.c $(builddir)/vcc_if.h
