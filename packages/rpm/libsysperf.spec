Name: libsysperf
Version: 0.2.2
Release: 1%{?dist}
Summary: Helper utilities and API library for CSV file handling
Group: Development/Tools
License: GPLv2+
URL: http://www.gitorious.org/+maemo-tools-developers/maemo-tools/libsysperf
Source: %{name}_%{version}.tar.gz
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
%defattr(-,root,root,-)
%{_bindir}/sp_*
%{_libdir}/%{name}.a
%{_mandir}/man1/*.1.gz
%{_includedir}/%{name}/*.h
%doc COPYING 


%changelog
* Wed Jan 13 2010 Eero Tamminen <eero.tamminen@nokia.com> 0.2.2
  * Rebuild for Harmattan and relax version missmatcg check.
     NB#151528

* Wed Mar 04 2009 Eero Tamminen <eero.tamminen@nokia.com> 0.2.1
  * Various issues reported by Coverity have been fixed. Fixes:
    NB#103722

* Tue May 13 2008 Eero Tamminen <eero.tamminen@nokia.com> 0.2.0
  * As jhash.h is no longer included in linux-kernel-headers, a copy has
    been explicitly added to this package (its license is loose enough
    to permit this). 

* Thu Sep 13 2007 Eero Tamminen <eero.tamminen@nokia.com> 0.1.4
  * Use libc double conversion function, the ones in cvs_float.c
    break with Glibc 2.5 for some reason
  * 

* Wed Aug 29 2007 Eero Tamminen <eero.tamminen@nokia.com> 0.1.3
  * Applied some updates to contact information. 

* Tue Jul 31 2007 Eero Tamminen <eero.tamminen@nokia.com> 0.1.2
  * Checked/fixed missing licencing information
  * 

* Thu Jan 18 2007 Eero Tamminen <eero.tamminen@nokia.com> 0.1.1
  * fix: csv_load sets input source name
  * sp_fix_tool_vers: now checks release.h vs. debian/changelog
  * bugfix: csv output float normalization fixed

* Wed Oct 04 2006 Eero Tamminen <eero.tamminen@nokia.com> 0.1.0
  * csv api modified
  * csv calculator:
    - supports c-style: <expr>?<expr>:<expr>
    - supports conditional: <if-expr>#<then-expr>
    - short-cut evaluation for '&&' and '||'
  * /proc parsers moved to libsysperf

* Thu Jan 26 2006 Eero Tamminen <eero.tamminen@nokia.com> 0.0.8
  * sp_textconv: now supports include via '::INC path'
  * sp_gen_manfile: fixed double warning issues with section shuffling

* Thu Sep 22 2005 Eero Tamminen <eero.tamminen@nokia.com> 0.0.7
  * new query methods for csv_t and array_t

* Wed Sep 14 2005 Eero Tamminen <eero.tamminen@nokia.com> 0.0.6
  * csv_rowcalc method added for csv_t objects

* Fri Sep 09 2005 Eero Tamminen <eero.tamminen@nokia.com> 0.0.5
  * msg api defaults to verbosity >= warnings

* Fri Jul 22 2005 Eero Tamminen <eero.tamminen@nokia.com> 0.0.4
  * csv api cleaned up

* Thu Jul 07 2005 Eero Tamminen <eero.tamminen@nokia.com> 0.0.3
  * generates man files for binaries
  * added: sp_fix_tool_vers
  * added: sp_gen_manfile
  * added: sp_textconv

* Tue Jul 05 2005 Eero Tamminen <eero.tamminen@nokia.com> 0.0.2
  * Added: sp_gen_changelog

* Tue Jul 05 2005 Eero Tamminen <eero.tamminen@nokia.com> 0.0.1
  * Initial release.
