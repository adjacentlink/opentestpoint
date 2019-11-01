bin_PROGRAMS = otestpoint-probe

otestpoint_probe_CPPFLAGS = \
 $(otestpoint_CFLAGS) \
 $(python_CFLAGS) \
 -I@top_srcdir@/include  

BUILT_SOURCES = \
 libotestpoint.pb.cc \
 libotestpoint.pb.h \
 probereport.pb.cc \
 probereport.pb.h        

otestpoint_probe_SOURCES = \
 otestpoint-probe.cc \
 pluginprobeadapter.cc \
 probeserviceimpl.cc \
 pythonprobeadapter.cc \
 probemanager.cc \
 libotestpoint.pb.cc \
 probereport.pb.cc

EXTRA_DIST= \
 pluginprobeadapter.h \
 probemanager.h \
 probeserviceimpl.h \
 pythonprobeadapter.h

otestpoint_probe_LDADD = \
 -L@top_srcdir@/src/otestpoint/.libs \
 -L@top_srcdir@/src/toolkit/.libs \
 $(otestpoint_LIBS) \
 $(python_LIBS) \
 -lrt -ldl -lpthread

libotestpoint.pb.cc libotestpoint.pb.h: @top_srcdir@/src/proto/libotestpoint.proto
	protoc -I=@top_srcdir@/src/proto --cpp_out=. $<

probereport.pb.cc probereport.pb.h: @top_srcdir@/include/otestpoint/proto/probereport.proto
	protoc -I=@top_srcdir@/include/otestpoint/proto --cpp_out=. $<

clean-local:
	rm -f $(BUILT_SOURCES)

