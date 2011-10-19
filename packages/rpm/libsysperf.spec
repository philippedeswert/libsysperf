Name: libsysperf
Version: 0.2.3
Release: 1%{?dist}
Summary: Helper utilities and API library for CSV file handling
Group: Development/Tools
License: GPLv2+
URL: http://www.gitorious.org/+maemo-tools-developers/maemo-tools/libsysperf
Source: %{name}_%{version}.tar.gz
BuildRoot: {_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
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
make install ROOT=%{buildroot}

%clean
rm -rf %{buildroot}

%package -n %{name}-devel
Summary: Helper library for CSV and /proc/ file handling
Group: Development/Tools

%description  -n %{name}-devel
 API library and utilities for development of system performance tools
 dealing with CSV files.

%files -n %{name}-devel
%defattr(755,root,root,-)
%{_bindir}/sp_*
%defattr(644,root,root,-)
%{_libdir}/%{name}.a
%{_mandir}/man1/*.1.gz
%{_includedir}/%{name}/*.h
%doc COPYING 

