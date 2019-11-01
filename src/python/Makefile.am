EXTRA_DIST= \
 AUTHORS \
 COPYING \
 MANIFEST.in \
 setup.py.in \
 otestpoint \
 scripts

BUILT_SOURCES = \
 setup.py \
 otestpoint/interface/discovery_pb2.py \
 otestpoint/interface/probereport_pb2.py \
 otestpoint/interface/measurementtable_pb2.py \
 otestpoint/probes/timeofday_pb2.py

all-local:
	$(PYTHON) setup.py build

clean-local: setup.py
	$(PYTHON) setup.py clean
	-rm -rf build
	-rm -rf dist
	-rm -f $(BUILT_SOURCES)
	-find . -name "*.pyc" -delete
	-rm -rf opentestpoint_python.egg-info
	-rm -f .installedfiles

DISTCLEANFILES=setup.py

sdist:
	$(PYTHON) setup.py sdist

edit = sed \
        -e 's|@VERSION[@]|$(VERSION)|g'\
        -e 's|@PYTHON[@]|$(PYTHON)|g' \
        -e 's|@TOPDIR[@]|$(TOPDIR)|g'

setup.py:	setup.py.in
	if test -f $@; then chmod u+w $@; fi
	$(edit) $< > $@
	chmod g-w,u-w $@

install-exec-hook: $(BUILT_SOURCES)
	$(PYTHON) setup.py install \
            --single-version-externally-managed \
	    -O1 \
            --record .installedfiles \
            --prefix=$(prefix) \
            --exec-prefix=$(exec_prefix) \
            $(if $(DESTDIR),--root=$(DESTDIR)) \
            $(if $(subst false,,$(HAVE_DEB)),--install-layout=deb)

uninstall-hook:
	if test -f .installedfiles; then xargs -a .installedfiles rm -f; fi

otestpoint/interface/discovery_pb2.py: @top_srcdir@/include/otestpoint/proto/discovery.proto
	protoc -I@top_srcdir@/include/otestpoint/proto --python_out=otestpoint/interface $<

otestpoint/interface/probereport_pb2.py: @top_srcdir@/include/otestpoint/proto/probereport.proto
	protoc -I@top_srcdir@/include/otestpoint/proto --python_out=otestpoint/interface $<

otestpoint/interface/measurementtable_pb2.py: @top_srcdir@/include/otestpoint/proto/measurementtable.proto
	protoc -I@top_srcdir@/include/otestpoint/proto --python_out=otestpoint/interface $<

otestpoint/probes/timeofday_pb2.py: @top_srcdir@/src/probe/proto/timeofday.proto
	protoc -I@top_srcdir@/src/probe/proto --python_out=otestpoint/probes $<
