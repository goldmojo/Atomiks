#
# spec file for package atomiks
#
# Copyright (c) 2013, 2014, 2015 Mateusz Viste
#

Name: atomiks
Version: 1.0.4.1
Release: 1%{?dist}
Summary: A faithful remake of, and a tribute to, Atomix, a classic puzzle game

Group: Amusements/Games/Logic

License: GPL-3.0+
URL: http://atomiks.sourceforge.net/
Source0: %{name}-%{version}.tar.gz

BuildRequires: SDL2-devel
BuildRequires: SDL2_mixer-devel

%description
Atomiks is a faithful remake of, and a tribute to, Atomix, a classic puzzle game created by Softtouch & RoSt and published in 1990 by the Thalion Software company. Atomiks is free software, and shares no code with the original Atomix game.

%prep
%setup

%build
make

%check

%install
install -D atomiks %buildroot/%{_bindir}/atomiks
mkdir -p %buildroot/usr/share/icons/hicolor/64x64/apps/
install -D atomiks.png %buildroot/usr/share/icons/hicolor/64x64/apps/

%files
%dir /usr/
%dir /usr/share/
%dir /usr/share/icons/
%dir /usr/share/icons/hicolor/
%dir /usr/share/icons/hicolor/64x64/
%dir /usr/share/icons/hicolor/64x64/apps
%attr(644, root, root) %doc readme.txt license.txt history.txt
%attr(755, root, root) %{_bindir}/atomiks
%attr(644, root, root) /usr/share/icons/hicolor/64x64/apps/atomiks.png

%changelog
