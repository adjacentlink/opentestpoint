EXTRA_DIST= \
 AUTHORS \
 COPYING \
 MANIFEST.in \
 setup.py.in \
 otestpoint \
 src

BUILT_SOURCES = \
 setup.py

all-local:
	$(PYTHON) setup.py build

clean-local: setup.py
	$(PYTHON) setup.py clean
	-rm -rf build
	-rm -rf dist
	-rm -f $(BUILT_SOURCES)
	-find . -name "*.pyc" -delete
	-rm -rf opentestpoint_toolkit_python.egg-info
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

