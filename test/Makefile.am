NULL = 

AM_CFLAGS = \
	    -I$(top_srcdir) \
	    -I$(top_srcdir)/src \
	    $(GLIB_CFLAGS) \
	    $(GOBJECT_CFLAGS) \
	    -std=c99 \
	    $(NULL)

VALAC_ARGS = \
	     --vapidir=$(top_srcdir)/data \
	     --pkg=libisi \
	     -C \
	     $(NULL)

sbin_PROGRAMS = $(TEST_PROGS)

progs_ldadd = \
		$(GLIB_LIBS)  \
		$(GOBJECT_LIBS) \
		../src/libisi.la \
		$(NULL)

TEST_PROGS = network
network_VALASOURCES = network.vala
network_SOURCES = network.c

$(network_SOURCES): $(network_VALASOURCES)
	$(VALAC) $(VALAC_ARGS) $^
	touch $@

network_LDADD = $(progs_ldadd)
EXTRA_DIST = $(network_VALASOURCES)

TEST_PROGS += authenticate
authenticate_VALASOURCES = authenticate.vala
authenticate_SOURCES = authenticate.c

$(authenticate_SOURCES): $(authenticate_VALASOURCES)
	$(VALAC) $(VALAC_ARGS) $^
	touch $@

authenticate_LDADD = $(progs_ldadd)
EXTRA_DIST += $(authenticate_VALASOURCES)

CLEANFILES = \
	     $(authenticate_SOURCES) \
	     $(network_SOURCES) \
	     $(NULL)
