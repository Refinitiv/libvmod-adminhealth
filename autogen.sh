#!/bin/sh

set -ex

libtoolize --copy --force
aclocal -I m4
autoheader
automake --add-missing --copy --foreign > /dev/null 2>&1
autoconf
