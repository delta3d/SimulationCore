# Qt project file for the Stealth Viewer.

TEMPLATE = app
CONFIG += qt debug thread
TARGET = Stealth Viewer

UI_FILES_DIR = ../../source/StealthViewer/ui
UI_HEADERS_DIR = ../../include/StealthViewer/Qt
UI_SOURCES_DIR = ../../source/StealthViewer/ui

UI_SRC_GLOB = $$system(python doglob.py $$quote('$$UI_FILES_DIR/*.ui'))

FORMS += $$UI_SRC_GLOB

SRC_DIR = ../../source/StealthViewer/Qt
INC_DIR = ../../include/StealthViewer/Qt

SRC_GLOB = $$system(python doglob.py $$quote('$$SRC_DIR/*.cpp'))
INC_GLOB = $$system(python doglob.py $$quote('$$INC_DIR/*.h'))

win32 {
   MOC_DIR = $$quote($$SRC_DIR/moc)
   DESTDIR = ../../bin
}

HEADERS += $$INC_GLOB 
SOURCES += $$SRC_GLOB

INCLUDEPATH = ../../include
INCLUDEPATH += ../../include/StealthViewer/Qt


