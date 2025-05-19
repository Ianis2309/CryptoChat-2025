QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    backgroundscrollarea.cpp \
    chatwindow.cpp \
    crypto_utils.cpp \
    csocket.cpp \
    main.cpp \
    mainwindow.cpp \
    splashscreen.cpp

HEADERS += \
    backgroundscrollarea.h \
    chatwindow.h \
    crypto_utils.h \
    csocket.h \
    mainwindow.h \
    splashscreen.h

FORMS += \
    chatwindow.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

LIBS += -lws2_32
LIBS += -ladvapi32
