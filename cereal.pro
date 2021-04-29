QT += widgets serialport charts
requires(qtConfig(combobox))

TARGET = cereal
TEMPLATE = app

HEADERS += \
    src/mainwindow.h \
    src/settingsdialog.h \
    src/console.h \
    src/chart.h

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/settingsdialog.cpp \
    src/console.cpp \
    src/chart.cpp

FORMS += \
    src/mainwindow.ui \
    src/settingsdialog.ui

RESOURCES += \
    cereal.qrc


target.path = build/
INSTALLS += target
