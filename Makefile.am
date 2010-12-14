NULL =

SUBDIRS = \
	  src \
	  data \
	  $(NULL)

if WANT_WIRESHARK
SUBDIRS += \
	   wireshark-plugin \
	   $(NULL)
endif

if WANT_TESTS
SUBDIRS += \
	  test \
	  $(NULL)
endif

EXTRA_DIST = \
	      MAINTAINERS \
	      $(NULL)

MAINTAINERCLEANFILES = \
  aclocal.m4 compile config.guess config.sub \
  configure depcomp install-sh ltmain.sh     \
  Makefile.in missing config.h.in            \
  mkinstalldirs *~ \
  $(NULL)
