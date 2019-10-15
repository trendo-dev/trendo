
Debian
====================
This directory contains files used to package trendod/trendo-qt
for Debian-based Linux systems. If you compile trendod/trendo-qt yourself, there are some useful files here.

## trendo: URI support ##


trendo-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install trendo-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your trendoqt binary to `/usr/bin`
and the `../../share/pixmaps/trendo128.png` to `/usr/share/pixmaps`

trendo-qt.protocol (KDE)
