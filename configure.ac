%AC_COPYRIGHT([
 Copyright (c) 2014-2020 - Adjacent Link LLC, Bridgewater, New Jersey
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:
 
 * Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in
   the documentation and/or other materials provided with the
   distribution.
 * Neither the name of Adjacent Link LLC nor the names of its
   contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

 See toplevel COPYING for more information.
])

AC_INIT([opentestpoint],1.0.11,[labs at adjacent link dot com])

AC_CONFIG_SRCDIR([src])

AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([])

AC_PROG_CXX

AC_PROG_CXXCPP

AC_GNU_SOURCE

AX_CXX_COMPILE_STDCXX_11([noext],[mandatory])

AC_PROG_INSTALL

AC_PROG_LIBTOOL

AC_LANG([C++])

AC_ARG_WITH(profile,
           [  --with-profile          add profiling support (off)])

AC_ARG_WITH(debug,
           [  --with-debug            add debug support (off)])

AC_ARG_WITH(python2,
           [  --with-python2          use python2 instead of python3 (off)])

PKG_CHECK_MODULES([libxml2],libxml-2.0)
PKG_CHECK_MODULES([protobuf],protobuf)
PKG_CHECK_MODULES([libzmq],libzmq)
PKG_CHECK_MODULES([sqlite3],sqlite3)
PKG_CHECK_MODULES([libuuid], uuid)

if test "$with_python2" = "yes"
then
PKG_CHECK_MODULES([python2],python2)
AM_PATH_PYTHON([2.7])
AC_SUBST(python_CFLAGS,["${python2_CFLAGS}"])
AC_SUBST(python_LIBS,["${python2_LIBS}"])
AC_SUBST(python_PYTHON,["python2"])
AM_CONDITIONAL(HAVE_PYTHON2,true)
else
PKG_CHECK_MODULES([python3], [python3-embed],[],
   [PKG_CHECK_MODULES([python3], [python3])])
AM_PATH_PYTHON([3.5])
AC_SUBST(python_CFLAGS,["${python3_CFLAGS}"])
AC_SUBST(python_LIBS,["${python3_LIBS}"])
AC_SUBST(python_PYTHON,["python3"])
AM_CONDITIONAL(HAVE_PYTHON2,false)
fi

AC_SUBST(otestpoint_CFLAGS,
["${libxml2_CFLAGS} ${protobuf_CFLAGS} ${libzmq_CFLAGS} ${libuuid_CFLAGS} ${sqlite3_CFLAGS}"])

AC_SUBST(otestpoint_LIBS,
["-lotestpoint-toolkit -lotestpoint ${libxml2_LIBS} ${protobuf_LIBS} ${libzmq_LIBS} ${libuuid_LIBS} ${sqlite3_LIBS}"])

AC_CHECK_FUNCS([__secure_getenv secure_getenv])

# compiler options
CXXFLAGS="$CXXFLAGS -W -Wall -g0 -O3"

# options for use with gprof
if test "$with_profile" = "yes"
then
CPPFLAGS="$CPPFLAGS -g -pg"
LDFLAGS="$LDFLAGS -g -pg"
fi

# options for use with debug
if test "$with_debug" = "yes"
then
CPPFLAGS="$CPPFLAGS -g -O0"
LDFLAGS="$LDFLAGS -g -O0"
fi

if test "$topdir" = ""
then
topdir="`pwd`"
fi

AC_SUBST(TOPDIR,${topdir})

LANG=C
AC_SUBST(LANG)

# check for rpmbuild
AC_CHECK_PROG(HAVE_RPMBUILD, rpmbuild, true, false)
AM_CONDITIONAL(HAVE_RPMBUILD,$HAVE_RPMBUILD)

# check for deb
AC_CHECK_PROG(HAVE_DEB, dh_clean, true, false)
AM_CONDITIONAL(HAVE_DEB,$HAVE_DEB)

AC_CHECK_FILE(/etc/debian_version,[DEBIAN_VERSION=`cat /etc/debian_version | awk -F/ '{print $1}'`],)
AC_SUBST(DEBIAN_VERSION)

AC_SUBST(DATE_RFC2822,`date -R`)

AC_OUTPUT(
 Makefile
 include/Makefile
 include/otestpoint/Makefile
 include/otestpoint/proto/Makefile
 include/otestpoint/toolkit/Makefile
 include/otestpoint/toolkit/log/Makefile
 src/Makefile
 src/probe/Makefile
 src/probe/plugin/Makefile
 src/probe/plugin/timeofday/Makefile
 src/probe/proto/Makefile
 src/proto/Makefile
 src/otestpoint/Makefile
 src/otestpointd/Makefile
 src/otestpoint-broker/Makefile
 src/otestpoint-probe/Makefile
 src/otestpoint-recorder/Makefile
 src/python/Makefile
 src/toolkit/Makefile
 src/toolkit/python/Makefile
 src/otestpoint/libotestpoint.pc
)
