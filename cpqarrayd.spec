# Note that this is NOT a relocatable package
%define name		cpqarrayd
%define version		1.1
%define release		3

# defaults for redhat
%define prefix		/usr
%define sysconfdir	/etc

Summary: Cpqarrayd monitors SmartArray controllers for you and notifies by sending SNMP traps and via syslog.
Name: %{name}
Version: %{version}
Release: %{release}
Copyright: GPL
Group: Applications/System
URL: http://starbreeze.knoware.nl/compaq
Source: ftp://starbreeze.knoware.nl/pub/hugo/cpqarrayd/cpqarrayd.tar.gz
Requires: ucd-snmp
Packager: Hugo Trippaers <spark@knoware.nl>
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
# this next bit fails because the makefile does not install relative to build root
# and also tries to chkconfig
##make prefix=$RPM_BUILD_ROOT%{prefix} sysconfdir=$RPM_BUILD_ROOT%{sysconfdir} install-redhat

## The startup file has a *nasty* requirement that /usr/local/sbin/sshd is there
## and fails to substitute a path correctly
perl -i -n -e 's:\@installroot\@:%{prefix}:;print unless (m:/usr/local/sbin/sshd:)' scripts/cpqarrayd

# install the startup script manually
mkdir -p $RPM_BUILD_ROOT/etc/rc.d/init.d/
install scripts/cpqarrayd $RPM_BUILD_ROOT/etc/rc.d/init.d/

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
###################################################################
%changelog

