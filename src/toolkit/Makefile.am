SUBDIRS= \
 . \
 python

lib_LTLIBRARIES = libotestpoint-toolkit.la

libotestpoint_toolkit_la_CPPFLAGS= \
 $(libzmq_CFLAGS) \
 $(protobuf_CFLAGS) \
 $(libuuid_CFLAGS) \
 $(python_CFLAGS) \
 -I@top_srcdir@/include     

BUILT_SOURCES = \
 loglevel.pb.cc \
 loglevel.pb.h \
 logserviceimpl.pb.cc \
 logserviceimpl.pb.h \
 logservice.pb.cc \
 logservice.pb.h                      

libotestpoint_toolkit_la_SOURCES= \
 addrinfo.cc \
 application.cc \
 logclientbuilder.cc \
 logclientimpl.cc \
 loglevel.pb.cc \
 logservicebuilder.cc \
 logserviceimpl.cc \
 logserviceimpl.pb.cc \
 logservice.pb.cc \
 pythonutils.cc \
 servicesingleton.cc

EXTRA_DIST= \
 logclientimpl.h \
 loglevel.proto \
 logserviceimpl.h \
 logserviceimpl.proto \
 logservice.proto \
 logutils.h

libotestpoint_toolkit_la_LDFLAGS=  \
 -avoid-version

libotestpoint_toolkit_la_LIBADD = \
 $(protobuf_LIBS) \
 $(python_LIBS)

loglevel.pb.cc loglevel.pb.h: loglevel.proto
	protoc -I=. --cpp_out=. $<

logserviceimpl.pb.cc logserviceimpl.pb.h: logserviceimpl.proto
	protoc -I=. --cpp_out=. $<

logservice.pb.cc logservice.pb.h: logservice.proto
	protoc -I=. --cpp_out=. $<

clean-local:
	rm -f $(BUILT_SOURCES)
