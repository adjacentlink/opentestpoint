bin_PROGRAMS = otestpointd

otestpointd_CPPFLAGS = \
 $(otestpoint_CFLAGS) \
 -I@top_srcdir@/include 

otestpointd_SOURCES = \
 probedirector.cc \
 otestpointd.cc

EXTRA_DIST= \
 probedirector.h \
 otestpointd.xml

otestpointd_LDADD = \
 -L@top_srcdir@/src/otestpoint/.libs \
 -L@top_srcdir@/src/toolkit/.libs \
 $(otestpoint_LIBS)
