# Version is injected by packaging/rpm/Makefile via `zfr version`.
# RPM Version cannot contain '-'; use `zfr version -r` (hyphens → '_').
# srcversion is the unsanitized Meson/git version and names the tarball.
%{!?version:%global version 0.0.0}
%{!?srcversion:%global srcversion %{version}}

Name:           poutils
Version:        %{version}
Release:        1%{?dist}
Summary:        Meson-based CLI project template with example app

License:        AGPL-3.0-or-later
URL:            https://github.com/lenik/poutils
Packager:       Lenik <poutils@bodz.net>
Source0:        %{name}-%{srcversion}.tar.xz

BuildRequires:  meson
BuildRequires:  ninja-build
BuildRequires:  pkgconf
BuildRequires:  check
BuildRequires:  libbas-c-dev
BuildRequires:  asciidoctor

%description
poutils is a template repository for small C/C++ command-line utilities.
It currently ships the poedit example application and Debian packaging
metadata, and includes Check-based unit-test integration.

%prep
%setup -q -n %{name}-%{srcversion}

%build
export CC="${CC:-gcc}"
export CXX="${CXX:-g++}"
export TMPDIR="${TMPDIR:-/tmp}"
mkdir -p "$TMPDIR"
meson setup build \
    --prefix=%{_prefix} \
    --bindir=%{_bindir} \
    --datadir=%{_datadir} \
    --mandir=%{_mandir} \
    --sysconfdir=%{_sysconfdir} \
    --localstatedir=%{_localstatedir} \
    --buildtype=plain
meson compile -C build

%install
meson install -C build --destdir=%{buildroot}
# pkgdatadir is reserved for project data; ensure the directory exists for %files.
mkdir -p %{buildroot}%{_datadir}/poutils

%files
%{_bindir}/poedit
%{_datadir}/bash-completion/completions/poedit
%{_mandir}/man1/poedit.1*
%dir %{_datadir}/poutils/
%{_datadir}/poutils/
%{_datadir}/locale/*/LC_MESSAGES/poutils.mo
%{_mandir}/*/man1/poedit.1*
%{_datadir}/doc/poutils/

%changelog
* Thu Aug 20 2026 Lenik <poutils@bodz.net>
- Align spec with debian/control (Meson, AGPL-3.0-or-later).
- Version comes from `zfr version`, the same method meson.build uses.
