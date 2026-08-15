QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    component.cpp \
    componenteditdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    schematicpage.cpp \
    shematicClass.cpp \
    wire.cpp \
    wirerouter.cpp \
    wizard.cpp\
    pin.cpp\
    pickdevicesdialog.cpp\
    placeddeviceitem.cpp\




HEADERS += \
    component.h \
    componenteditdialog.h \
    junction.h \
    mainwindow.h \
    schematicpage.h \
    shematicClass.h \
    wire.h \
    wirerouter.h \
    wizard.h\
    geometrytypes.h\
    pin.h\
    component.h\
    pickdevicesdialog.h\
    placeddeviceitem.h\
    junction.h


FORMS += \
    mainwindow.ui \
    schematicpage.ui \
    wizard.ui

TRANSLATIONS += \
    proteus_eo_001.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    .gitignore
