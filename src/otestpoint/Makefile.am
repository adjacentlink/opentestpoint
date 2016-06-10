lib_LTLIBRARIES = libotestpoint.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libotestpoint.pc

libotestpoint_la_CPPFLAGS= \
 $(libzmq_CFLAGS) \
 $(protobuf_CFLAGS) \
 $(libuuid_CFLAGS) \
 -I@top_srcdir@/include   

BUILT_SOURCES = \
 broker.pb.cc \
 broker.pb.h \
 controller.pb.cc \
 controller.pb.h \
 discovery.pb.cc \
 discovery.pb.h \
 libotestpoint.pb.cc \
 libotestpoint.pb.h \
 probereport.pb.cc \
 probereport.pb.h \
 recorder.pb.cc \
 recorder.pb.h         

libotestpoint_la_SOURCES = \
 probebuilder.cc \
 probecontainer.cc \
 probereport.pb.cc \
 brokerbuilder.cc \
 brokerimpl.cc \
 broker.pb.cc \
 controllerimpl.cc \
 controller.pb.cc \
 libotestpoint.pb.cc \
 discovery.pb.cc \
 recorder.pb.cc \
 recorderbuilder.cc \
 recorderimpl.cc       

EXTRA_DIST = \
 brokerimpl.h \
 broker.proto \
 controllerimpl.h \
 controller.proto \
 recorder.proto \
 recorderimpl.h \
 probecontainer.h

libotestpoint_la_LDFLAGS=  \
 -avoid-version

libotestpoint.pb.cc libotestpoint.pb.h: @top_srcdir@/src/proto/libotestpoint.proto
	protoc -I=@top_srcdir@/src/proto --cpp_out=. $<

controller.pb.cc controller.pb.h: controller.proto
	protoc -I=. --cpp_out=. $<

broker.pb.cc broker.pb.h: broker.proto
	protoc -I=. --cpp_out=. $<

discovery.pb.cc discovery.pb.h: @top_srcdir@/include/otestpoint/proto/discovery.proto
	protoc -I=@top_srcdir@/include/otestpoint/proto --cpp_out=. $<

probereport.pb.cc probereport.pb.h: @top_srcdir@/include/otestpoint/proto/probereport.proto
	protoc -I=@top_srcdir@/include/otestpoint/proto --cpp_out=. $<

recorder.pb.cc recorder.pb.h: recorder.proto
	protoc -I=. --cpp_out=. $<

clean-local:
	rm -f $(BUILT_SOURCES)
