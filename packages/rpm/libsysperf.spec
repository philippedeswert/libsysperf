Name: libsysperf
Version: 0.2.4
Release: 0%{?dist}
Summary: Helper utilities and API library for CSV file handling
Group: Development/Tools
License: GPLv2+
URL: http://www.gitorious.org/+maemo-tools-developers/maemo-tools/libsysperf
Source: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-build
BuildRequires: python

%description
 API library and utilities for development of system performance tools
 dealing with CSV files.

%prep
%setup -q -n %{name}

%build
make


%install
rm -rf %{buildroot}
make install DESTDIR=%{buildroot}

%clean
rm -rf %{buildroot}

%package -n %{name}-devel
Summary: Helper library for CSV and /proc/ file handling
Group: Development/Tools

%description  -n %{name}-devel
 API library and utilities for development of system performance tools
 dealing with CSV files.

%files -n %{name}-devel
%defattr(-,root,root,-)
%{_bindir}/sp_*
%{_libdir}/%{name}.a
%{_mandir}/man1/*.1.gz
%{_includedir}/%{name}/*.h
%doc COPYING 


%changelog
* Mon Dec 9 2011 Eero Tamminen <eero.tamminen@nokia.com> 0.2.4
  * v0.2.4 release, fix non-standard/unsafe GCC extension use

* Mon Dec 9 2011 Eero Tamminen <eero.tamminen@nokia.com> 0.2.3-3
  * OBS works best with tarballs that don't have version at all.

* Mon Dec 7 2011 Eero Tamminen <eero.tamminen@nokia.com> 0.2.3-2
  * Some RPM tools expect {name}-{version}, not {name}_{version}

* Mon Dec 5 2011 Eero Tamminen <eero.tamminen@nokia.com> 0.2.3
  * Fixes to Makefile for building with new GCC version
  * Fixes to changelog
