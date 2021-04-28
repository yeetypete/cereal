QT += widgets serialport charts
requires(qtConfig(combobox))

TARGET = cereal
TEMPLATE = app

HEADERS += \
    mainwindow.h \
    settingsdialog.h \
    console.h \
    chart.h

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp \
    console.cpp \
    chart.cpp

FORMS += \
    mainwindow.ui \
    settingsdialog.ui

RESOURCES += \
    terminal.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/serialport/terminal
INSTALLS += target
