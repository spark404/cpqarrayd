SUBDIRS = scripts
sbin_PROGRAMS = cpqarrayd
cpqarrayd_SOURCES = cpqarrayd.c discover.c sendtrap.c status.c

install-redhat:
	( cd scripts && $(MAKE) $(AM_MAKEFLAGS) install-redhat )

uninstall-redhat:
	( cd scripts && $(MAKE) $(AM_MAKEFLAGS) uninstall-redhat )
