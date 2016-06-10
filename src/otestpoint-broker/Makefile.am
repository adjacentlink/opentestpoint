bin_PROGRAMS = otestpoint-broker

otestpoint_broker_CPPFLAGS = \
 $(otestpoint_CFLAGS) \
 -I@top_srcdir@/include 

otestpoint_broker_SOURCES = \
 brokerdirector.cc \
 otestpoint-broker.cc        

EXTRA_DIST= \
 brokerdirector.h \
 otestpoint-broker.xml

otestpoint_broker_LDADD = \
 -L@top_srcdir@/src/otestpoint/.libs \
 -L@top_srcdir@/src/toolkit/.libs \
 $(otestpoint_LIBS)
