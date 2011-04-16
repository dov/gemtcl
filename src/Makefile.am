SUBDIRS = plis xnm

bin_PROGRAMS = gemtcl 

EXTRA_DIST = changelog_string.i logo_150_inline.i logo_64_inline.i

BUILT_SOURCES=
DISTCLEANFILES=
gemtcl_SOURCES = \
    gemtcl.cc \
    gem-button-box.cc \
    gem-button-box-editor.cc \
    gem-button-box-button-bless.cc \
    gem-button-box-button-bless-editor.cc \
    xmlrpc-server.c \
    xmlrpc-client.c

gemtcl_LDADD  =  $(PACKAGE_LIBS) 
gemtcl_LDFLAGS = -Lplis -lplis -Lxnm -lxnm
INCLUDES = $(PACKAGE_CFLAGS) \
           -I$(top_srcdir)/src/xnm \
           -I$(top_srcdir)/src/plis 

noinst_HEADERS = gem.h gnet_xmlrpc.h gem-button-box-button-bless.h

gem_button_box_SOURCES = gem-button-box.cc gem-button-box.h gem-button-box-private.h
DISTCLEANFILES += gem-button-box.cc gem-button-box.h gem-button-box-private.h
EXTRA_DIST += gem-button-box.cc gem-button-box.h gem-button-box-private.h

gem_button_box_button_bless_editor_SOURCES = gem-button-box-button-bless-editor.cc gem-button-box-button-bless-editor.h gem-button-box-button-bless-editor-private.h
DISTCLEANFILES += gem-button-box-button-bless-editor.cc gem-button-box-button-bless-editor.h gem-button-box-button-bless-editor-private.h
EXTRA_DIST += gem-button-box-button-bless-editor.cc gem-button-box-button-bless-editor.h gem-button-box-button-bless-editor-private.h

gem_button_box_editor_SOURCES = gem-button-box-editor.cc gem-button-box-editor.h gem-button-box-editor-private.h
DISTCLEANFILES += gem-button-box-editor.cc gem-button-box-editor.h gem-button-box-editor-private.h
EXTRA_DIST += gem-button-box-editor.cc gem-button-box-editor.h gem-button-box-editor-private.h

# How do I make this happen at autoconf/automake time and not at compile time?
%.gob.stamp: %.gob
	gob2 --no-touch --for-cpp $<
	@touch $@ 

%cc %h %-private.h: %.gob
	gob2 --for-cpp $<

logo_150_inline.i: ../misc/gemtcl-logo-150.png
	gdk-pixbuf-csource --name=logo_150_inline $< > $@

logo_64_inline.i: ../misc/gemtcl-logo-64.png
	gdk-pixbuf-csource --name=logo_64_inline $< > $@
