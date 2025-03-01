QT += widgets
QT += core gui
QT += multimedia

TEMPLATE = app

SOURCES = \
    objects.cpp \
    engine.cpp \
    objStructure.cpp \
    parser.cpp \
    editormainwindow.cpp \
    edit_main.cpp \
    graphicsvieweditor.cpp

HEADERS += \
    objects.h \
    engine.h \
    objStructure.h \
    parser.h \
    editormainwindow.h \
    graphicsvieweditor.h \
    definitions.h

TARGET = MixMaster

FORMS += \
    editormainwindow.ui \
	
	RC_FILE = bakingquest.rc
