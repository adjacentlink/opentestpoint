lib_LTLIBRARIES = libprobeplugintimeofday.la

libprobeplugintimeofday_la_CPPFLAGS= \
 $(protobuf_CFLAGS) \
 -I@top_srcdir@/include          

BUILT_SOURCES = \
 timeofday.pb.cc \
 timeofday.pb.h        

libprobeplugintimeofday_la_SOURCES = \
 timeofday.cc \
 timeofday.pb.cc

EXTRA_DIST = \
 timeofday.h                        

libprobeplugintimeofday_la_LDFLAGS= \
 -avoid-version

timeofday.pb.cc timeofday.pb.h: @top_srcdir@/src/probe/proto/timeofday.proto
	protoc -I=@top_srcdir@/src/probe/proto --cpp_out=. $<

clean-local:
	rm -f $(BUILT_SOURCES)
