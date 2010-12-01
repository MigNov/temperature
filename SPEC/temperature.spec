Name:		temperature
Version:	0.0.1
Release:	1%{?dist}
Summary:	Utility to measure temperature on CPU core or motherboard

Group:		Applications/System
License:	GPL
#URL:		
Source0:	temperature.tar.xz
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

%description
Temperature is a tool for measuring temperature on CPU core or motherboard
using the algorithm specified in the XML documentation file with possibility
to use the native support integrated directly into the binary.

%prep
%setup -q

%build
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/etc
cp data/temperature-settings.xml $RPM_BUILD_ROOT/etc/temperature-settings.xml
install -m 755 data/temperature-test-script $RPM_BUILD_ROOT/etc/temperature-test-script

%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%{_bindir}/temperature
/etc/temperature-settings.xml
/etc/temperature-test-script
%doc



%changelog
* Wed Dec 01 2010 Michal Novotny <minovotn@redhat.com>
- Initial release (v0.0.1)
