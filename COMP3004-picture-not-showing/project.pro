QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    aed.cpp \
    main.cpp \
    mainwindow.cpp \
    patient.cpp \
    qcustomplot.cpp

HEADERS += \
    patient.h \
    aed.h \
    mainwindow.h \
    qcustomplot.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    project.qrc

DISTFILES += \
    pic/electrodeAdultPadOn.svg \
    pic/electrodeChildPadOn.svg \
    pic/electrodePadOff.svg \
    pic/powerOff.svg \
    pic/powerOn.svg \
    pic/shockButton.svg \
    pic/shockButton_on.svg \
    pic/step1.svg \
    pic/step1_light.svg \
    pic/step2.svg \
    pic/step2_light.svg \
    pic/step3.svg \
    pic/step3_light.svg \
    pic/step4.svg \
    pic/step4_light.svg \
    pic/step5.svg \
    pic/step5_light.svg
