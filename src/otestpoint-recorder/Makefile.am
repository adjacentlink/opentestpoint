bin_PROGRAMS = otestpoint-recorder

otestpoint_recorder_CPPFLAGS = \
 $(otestpoint_CFLAGS) \
 -I@top_srcdir@/include 

otestpoint_recorder_SOURCES = \
 recorderdirector.cc \
 otestpoint-recorder.cc        

EXTRA_DIST= \
 recorderdirector.h \
 otestpoint-recorder.xml

otestpoint_recorder_LDADD = \
 -L@top_srcdir@/src/otestpoint/.libs \
 -L@top_srcdir@/src/toolkit/.libs \
 $(otestpoint_LIBS)              

