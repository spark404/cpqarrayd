install-redhat:
	$(INSTALL_SCRIPT) cpqarrayd /etc/rc.d/init.d
	/sbin/chkconfig --add cpqarrayd

uninstall-redhat:
	/sbin/chkconfig --del cpqarrayd
	rm -f /etc/rc.d/init.d/cpqarrayd
