# w2rViewer

## Story
Project as a starting point for developing log viewers and real-time data viewers.

Visualizing data on the GUI helps me do my job more efficiently, but I wanted to do it without having to write code from scratch every time.
The code was mostly written in the blank month when I changed jobs in 2021, but I have been following Qt version upgrades and other changes since then.

The code used to be maintained in a private SVN, but I decided to make it public on GitHub starting in March 2024.
The reason for this is that I want to use it for other purposes besides my work, and I want to make it clear that the parts I created myself and the parts I created for work do not get mixed up.

The code I wrote is MIT licensed, so I can use it for any future use, but there is GPL code mixed in where I use third party libraries. It can be easily separated if needed.

## Functionality
This project is intended as a starting point and implements only a minimal visualization example.
I envision using this repository by forking it. Deleting unnecessary functionality and implementing necessary functionality.

### 3D View
View 3D models, point clouds, etc. using OpenGL
If you are not using 3D View, you must edit mainwindow.(cpp,h,ui)
Define USE_3D_VIEW in w2rViewer.pro

### Plot View
Use QCustomPlot to plot dynamic and static data
Define USE_PLOT_VIEW in w2rViewer.pro

### Image View
Display image files. Scaling and panning are implemented.
Define USE_IMAGE_VIEW in w2rViewer.pro

### Map View
Use QGeoView to view a map and the data on it.
Define USE_MAP_VIEW in w2rViewer.pro

## Platform
Windows (Qt 5.12.12 Mingw64)

Linux (Qt 5.12.8, Ubuntu 20.04 package "qt5-default")
- Linux version needs to install "libqt5serialport5-dev" to use serialport

## Third party libraries
1. QCustomPlot (GPL)
    - to build this library, run thirdParty/QCustomPlot/build_****.sh
2. QGeoView (LGPL3)
    - to build this library, run thirdParty/QGeoView/build_****.sh
3. EDL module from Cloud compare (GPL)

build_mingw.sh uses mingw toolchain from Qt, script must run on MSYS2.
You need to install several packages using pacman (cmake, make, git etc)

## Sample data
1. Stanford bunny (reduced polygon and converted to Metasequoia format)


