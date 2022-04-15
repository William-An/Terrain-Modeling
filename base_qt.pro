SOURCES += \
	src/app.cpp \
	src/glview.cpp \
	src/glstate.cpp \
	src/mesh.cpp \
	src/light.cpp \
	src/util.cpp \
	src/gl_core_3_3.c

HEADERS += \
	src/app.hpp \
	src/glview.hpp \
	src/glstate.hpp \
	src/mesh.hpp \
	src/light.hpp \
	src/util.hpp \
	src/gl_core_3_3.h

INCLUDEPATH += \
	$$PWD/include

MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/qrc
UI_DIR      = build/uic

CONFIG += qt c++17
QT += widgets
