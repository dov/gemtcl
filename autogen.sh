#!/bin/sh
# Run this to generate all the initial makefiles, etc.

DIE=0

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have autoconf installed to compile gtkimageviewer."
	echo "Download the appropriate package for your distribution,"
	echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
	DIE=1
}

(libtool --version) < /dev/null > /dev/null 2>&1 || {
	echo
	echo "You must have libtool installed to compile gtkimageviewer."
	echo "Get it from ftp://ftp.gnu.org/pub/gnu/libtool/"
	DIE=1
}
if test "$DIE" -eq 1; then
	exit 1
fi

if test -z "$*"; then
	echo "I am going to run ./configure with no arguments - if you wish "
        echo "to pass any to it, please specify them on the $0 command line."
fi

libtoolize --force
aclocal $ACLOCAL_FLAGS
automake --add-missing
autoconf
./configure "$@"

echo 
echo "Now type 'make' to compile gtkimageviewer"
