QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    ./ffmpeg/include/

LIBS += -L$$PWD/ffmpeg/lib/ -lavformat -lavcodec -lavdevice -lavfilter -lavutil -lpostproc -lswresample -lswscale

QT += multimedia

SOURCES += \
    log.cpp \
    main.cpp \
    mainwindow.cpp \
    player/player.cpp

HEADERS += \
    log.h \
    mainwindow.h \
    player/player.h


FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/ffmpeg/lib/ -lavcodec
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/ffmpeg/lib/ -lavcodecd
#else:unix: LIBS += -L$$PWD/ffmpeg/lib/ -lavcodec

#INCLUDEPATH += $$PWD/ffmpeg/include
#DEPENDPATH += $$PWD/ffmpeg/include

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/ffmpeg/lib/ -lavdevice
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/ffmpeg/lib/ -lavdeviced
#else:unix: LIBS += -L$$PWD/ffmpeg/lib/ -lavdevice

#INCLUDEPATH += $$PWD/ffmpeg/include
#DEPENDPATH += $$PWD/ffmpeg/include

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/ffmpeg/lib/ -lavfilter
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/ffmpeg/lib/ -lavfilterd
#else:unix: LIBS += -L$$PWD/ffmpeg/lib/ -lavfilter

#INCLUDEPATH += $$PWD/ffmpeg/include
#DEPENDPATH += $$PWD/ffmpeg/include

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/ffmpeg/lib/ -lavformat
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/ffmpeg/lib/ -lavformatd
#else:unix: LIBS += -L$$PWD/ffmpeg/lib/ -lavformat

#INCLUDEPATH += $$PWD/ffmpeg/include
#DEPENDPATH += $$PWD/ffmpeg/include

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/ffmpeg/lib/ -lavutil
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/ffmpeg/lib/ -lavutild
#else:unix: LIBS += -L$$PWD/ffmpeg/lib/ -lavutil

#INCLUDEPATH += $$PWD/ffmpeg/include
#DEPENDPATH += $$PWD/ffmpeg/include

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/ffmpeg/lib/ -lpostproc
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/ffmpeg/lib/ -lpostprocd
#else:unix: LIBS += -L$$PWD/ffmpeg/lib/ -lpostproc

#INCLUDEPATH += $$PWD/ffmpeg/include
#DEPENDPATH += $$PWD/ffmpeg/include

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/ffmpeg/lib/ -lswresample
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/ffmpeg/lib/ -lswresampled
#else:unix: LIBS += -L$$PWD/ffmpeg/lib/ -lswresample

#INCLUDEPATH += $$PWD/ffmpeg/include
#DEPENDPATH += $$PWD/ffmpeg/include

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/ffmpeg/lib/ -lswscale
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/ffmpeg/lib/ -lswscaled
#else:unix: LIBS += -L$$PWD/ffmpeg/lib/ -lswscale

#INCLUDEPATH += $$PWD/ffmpeg/include
#DEPENDPATH += $$PWD/ffmpeg/include
