# Note that this is NOT a relocatable package
%define name		cpqarrayd
%define version		2.2
%define release		1

# defaults for redhat
%define prefix		/usr
%define sysconfdir	/etc

Summary: Cpqarrayd monitors SmartArray controllers for you and notifies by sending SNMP traps and via syslog.
Name: %{name}
Version: %{version}
Release: %{release}
Copyright: GPL
Group: Applications/System
URL: http://www.strocamp.net/opensource/cpqarrayd.php
Source: http://www.strocamp.net/opensource/compaq/downloads/cpqarrayd-2.2.tar.gz
Requires: net-snmp
Packager: Hugo Trippaers <opensource@strocamp.net>
BuildRoot: /var/tmp/%{name}-%{version}-root

%description
Cpqarrayd monitors SmartArray controllers for you and notifies by sending SNMP traps and via syslog.

%prep
%setup -q

# seems as if xss support is broken on alpha :-(
if [ ! -f configure ]; then
  CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh $ARCH_FLAGS --prefix=%{prefix} --sysconfdir=%{sysconfdir}
else
  CFLAGS="$RPM_OPT_FLAGS" ./configure $ARCH_FLAGS --prefix=%{prefix} --sysconfdir=%{sysconfdir}
fi

%build
make

%install
make prefix=$RPM_BUILD_ROOT%{prefix} sysconfdir=$RPM_BUILD_ROOT%{sysconfdir}  install-strip

perl -i -p -e 's:\@installroot\@:%{prefix}:;' scripts/cpqarrayd

# install the startup script manually
mkdir -p $RPM_BUILD_ROOT/etc/rc.d/init.d/
mkdir -p $RPM_BUILD_ROOT/etc/sysconfig/
install scripts/cpqarrayd $RPM_BUILD_ROOT/etc/rc.d/init.d/
install scripts/cpqarrayd.sysconfig $RPM_BUILD_ROOT/etc/sysconfig/cpqarrayd

%post
/sbin/chkconfig --add cpqarrayd

%preun
if [ "$1" = 0 ]
then
        /etc/rc.d/init.d/cpqarrayd stop >/dev/null >&2
        /sbin/chkconfig --del cpqarrayd
fi

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc AUTHORS COPYING ChangeLog INSTALL NEWS README
%{prefix}/sbin/cpqarrayd
/etc/rc.d/init.d/cpqarrayd
/etc/sysconfig/cpqarrayd
/usr/man/man1/cpqarrayd.1.gz

###################################################################
%changelog

