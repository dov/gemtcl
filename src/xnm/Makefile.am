## Process this file with automake to produce Makefile.in
noinst_LIBRARIES = libxnm.a

INCLUDES = \
	-DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	$(PACKAGE_CFLAGS)

libxnm_a_inc_HEADERS = xnm.h 
libxnm_a_incdir = $(includedir)/xnm

EXTRA_DIST = xnm_private.h

libxnm_a_SOURCES =      \
        xnm-parser.y         \
        xnm-struct.c         \
        xnm-retrieve.c         

