Qt template code for CS334


LINUX =========================

1. Make sure you have all dependencies installed.

	Debian-based systems (e.g. Ubuntu):
	$ sudo apt install build-essential libglm-dev qt5-default

	Arch-based systems (e.g. Manjaro):
	$ sudo pacman -Sy base-devel glm qt5-base

2. Generate makefile
	$ qmake .

2. Compile
	$ make

3. Run
	$ ./base_qt




WINDOWS =======================

1. Install Qt5

	https://www.qt.io/download-open-source
	Recently the Qt Company has decided that you need a Qt Account to
	download Qt binaries, even for open source development. There may
	be ways to circumvent this.

2. Install Qt Visual Studio Extension

	In Visual Studio, go to Extensions > Manage Extensions, click on
	Visual Studio Marketplace, search for "qt", and install the
	Qt Visual Studio Tools. Restart Visual Studio.

3. Open the base_qt.pro file

	In Visual Studio, go to Extensions > Qt VS Tools > Open Qt Project
	File (.pro). Navigate to base_qt.pro and open it. This will generate
	a visual studio solution and project.

4. Build & run


Note: it's recommended that any changes you make to the project (such as
adding new source files) be done directly in the .pro file, followed by
re-generating the visual studio project.
