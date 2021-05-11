bin_PROGRAMS = 
EXTRA_DIST =
linted_files =
gperf_files =

include $(srcdir)/src/Makefile.am

include $(srcdir)/utils/gperf.am


if ENABLE_LINTERS
include $(srcdir)/utils/lint.am
endif

if ENABLE_TESTS
DISTCHECK_CONFIGURE_FLAGS = --enable-tests
include $(srcdir)/tests/Makefile.am
endif
