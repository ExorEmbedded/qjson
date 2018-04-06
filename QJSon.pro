TEMPLATE = lib
TARGET = qjson

INCLUDEPATH += . src
LIBS +=
DEPENDPATH += . src
LIBDIR = /usr/lib
INCDIR = /usr/include/qjson

exists (../../../../linuxbuild/lin-common.pri) {
  include(../../../../linuxbuild/lin-common.pri)
}

DEFINES += QJSONBACKPORT_LIBRARY Q_JSONRPC_DLL Q_BUILD_JSONRPC

SOURCES += src/qjsonwriter.cpp \
    src/qjsonvalue.cpp \
    src/qjsonparser.cpp \
    src/qjsonobject.cpp \
    src/qjsondocument.cpp \
    src/qjsonarray.cpp \
    src/qjson.cpp src/parser.cpp src/serializer.cpp

HEADERS += src/qjson-backport_global.h src/qjsonwriter_p.h \
    src/qjsonvalue.h \
    src/qjsonparser_p.h \
    src/qjson_p.h \
    src/qjsonobject.h \
    src/qjsonexport.h \
    src/qjsondocument.h \
    src/qjsonarray.h src/parser.h src/serializer.h

target.path = $${LIBDIR}
INSTALLS += target

headers.path = $${INCDIR}
headers.files = $${HEADERS}
INSTALLS += headers
