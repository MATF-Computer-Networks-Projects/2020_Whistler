QT       -= core gui
QT -=gui
QT +=network
QT+=sql

#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG+=console
#CONFIG += c++11 console
CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

VPATH += \
    src/ \
    headers/ \
    ui/

SOURCES += \
    main.cpp \
    server.cpp \
    client.cpp \
    simplecrypt.cpp

HEADERS += \
    server.h \
    client.h \
    simplecrypt.h

FORMS += \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
