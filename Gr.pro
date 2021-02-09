QT += widgets serialport
QT += axcontainer
requires(qtConfig(combobox))

CONFIG += axcontainer #导出excel

TARGET = terminal
TEMPLATE = app

SOURCES += \
    MainWindow.cpp \
    MyQSerial.cpp \
    ParseMessage.cpp \
    SettingsDialog.cpp \
    main.cpp

HEADERS += \
    MainWindow.h \
    MyQSerial.h \
    ParseMessage.h \
    SettingsDialog.h

FORMS += \
    MainWindow.ui \
    SettingsDialog.ui

RESOURCES += \
    terminal.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/serialport/terminal
INSTALLS += target

DISTFILES += \
    Diagnostic20210121.bin
