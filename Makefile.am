
AUTOMAKE_OPTIONS=foreign subdir-objects

if IS_DEBUGON
SUBDIRS= \
    include/xtl \
    src/libtest \
    test
else
SUBDIRS= \
    include/xtl
    src/libtest
endif

# Handle automake/make conflicts
TARGETDIR=$(strip $(if $(findstring $(abs_top_srcdir), $(abs_top_builddir)), $(abs_top_builddir)/bin, $(abs_top_builddir)))

clean-local:
	rm -f $(TARGERDIR)/testrunner

if IS_DEBUGON
# Create a link to the test executable
all-local-dbg:
	@mkdir -p $(TARGETDIR)
	@if [ -e $(abs_top_builddir)/test/testrunner ]; \
	then \
		ln -f -s $(abs_top_builddir)/test/testrunner $(TARGETDIR)/testrunner; \
	fi

else
all-local-dbg:

endif

all-local:	all-local-dbg

