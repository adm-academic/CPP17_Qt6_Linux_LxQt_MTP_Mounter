QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    common.cpp \
    main.cpp \
    mainwindow.cpp \
    settings.cpp \
    states.cpp

HEADERS += \
    common.h \
    mainwindow.h \
    settings.h \
    states.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

TRANSLATIONS += \
    CPP17_Qt6_Linux_LxQt_MTP_Mounter_ru_RU.ts

DISTFILES += \
    CPP17_Qt6_Linux_LxQt_MTP_Mounter_ru_RU.qm \
    MTP_Mounter.conf \
    TODO.txt \
    build_distro.sh \
    install.sh \
    show_metrics.sh
