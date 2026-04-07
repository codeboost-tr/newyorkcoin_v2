#!/bin/sh
export LC_ALL=C
set -e
srcdir="$(dirname "$0")"
cd "$srcdir"
if [ -z "${LIBTOOLIZE}" ] && GLIBTOOLIZE="$(command -v glibtoolize)"; then
  LIBTOOLIZE="${GLIBTOOLIZE}"
  export LIBTOOLIZE
fi
command -v autoreconf >/dev/null || \
  (echo "configuration failed, please install autoconf first" && exit 1)
# Run libtoolize WITHOUT --automake to skip the ACLOCAL_AMFLAGS conflict
# check added in libtoolize 2.4.7.
${LIBTOOLIZE:-libtoolize} --force --copy
# Pass LIBTOOLIZE=true so autoreconf does NOT re-run libtoolize.
LIBTOOLIZE=true autoreconf --install --force --warnings=all
