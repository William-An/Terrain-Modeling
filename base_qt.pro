SOURCES += \
	src/*.cpp \
	src/*.cc \
	src/gl_core_3_3.c

HEADERS += \
	src/*.hpp \
	src/*.hh \
	src/gl_core_3_3.h

INCLUDEPATH += \
	$$PWD/include

MOC_DIR     = build/moc
OBJECTS_DIR = build/obj
RCC_DIR     = build/qrc
UI_DIR      = build/uic

CONFIG += qt c++17 debug
QT += widgets
