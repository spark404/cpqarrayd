# Note that this is NOT a relocatable package
%define name		cpqarrayd
%define version		0.1
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
URL: http://starbreeze.knoware.nl/compaq
Source: ftp://starbreeze.knoware.nl/pub/hugo/cpqarrayd/cpqarrayd.tar.gz
Requires: Kernel source with SmartArray support
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
make prefix=$RPM_BUILD_ROOT%{prefix} sysconfdir=$RPM_BUILD_ROOT%{sysconfdir} install-redhat

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc AUTHORS COPYING ChangeLog INSTALL NEWS README
%{prefix}/sbin/cpqarrayd

###################################################################
%changelog

