QT += serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = terminal
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    console.cpp \
    circle.cpp \
    linkitem.cpp \
    msgtree.cpp \
    linkscene.cpp

HEADERS += \
    mainwindow.h \
    settingsdialog.h \
    console.h \
    circle.h \
    linkitem.h \
    msgtree.h \
    define.h \
    linkscene.h

FORMS += \
    mainwindow.ui \
    settingsdialog.ui

RESOURCES += \
    terminal.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/serialport/terminal
INSTALLS += target
