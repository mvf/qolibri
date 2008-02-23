BuildRequires: libqt4-devel >= 4.1.0
Requires: libqt4 >= 4.1.0

Name:         qolibri
License:      GPL
Group:        Productivity/Office/Dictionary
Summary:      EPWING Dictionary/Book Viewer
Version:      1.0.0
Release:      1
URL:          http://sourceforge.jp/projects/qolibri/
BuildRoot:    %{_tmppath}/%{name}-%{version}-build
Source0:      %name-%{version}.tar.gz

%description
Viewer of EPWING Dictionary/Book.
EPWING is JIS(Japanese Industrial Standard) format of electronic dictionary.
Some dictionaries are created for free.

%prep
# extract the source and go into the kvpnc-0.7.2 directory
%setup -q

%build
qmake -config release
make %{?jobs:-j%jobs}

%install
make install
rm -rf /usr/share/qt4/translations/qolibri
rm -f /usr/share/qt4/translations/qolibri_*
%suse_update_desktop_file %name Office Dictionary

%clean
rm -f $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
#%doc README VERSION
%dir /opt/kde3/share/icons/hicolor/16x16/apps
%dir /opt/kde3/share/icons/hicolor/32x32/apps
/usr/bin/qolibri
