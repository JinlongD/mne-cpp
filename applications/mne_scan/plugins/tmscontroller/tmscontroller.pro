#==============================================================================================================
#
# @file     tmscontroller.pro
# @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
#           Lorenz Esch <lesch@mgh.harvard.edu>
# @since    0.1.0
# @date     July, 2012
#
# @section  LICENSE
#
# Copyright (C) 2012, Christoph Dinh, Lorenz Esch. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification, are permitted provided that
# the following conditions are met:
#     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
#       following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
#       the following disclaimer in the documentation and/or other materials provided with the distribution.
#     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
#       to endorse or promote products derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
# PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
#
# @brief    This project file generates the makefile for the tmscontroller plug-in.
#
#==============================================================================================================

include(../../../../mne-cpp.pri)

TEMPLATE = lib

QT += core widgets serialport

CONFIG += skip_target_version_ext plugin c++11

DESTDIR = $${MNE_BINARY_DIR}/mne_scan_plugins

contains(MNECPP_CONFIG, static) {
    CONFIG += staticlib
    DEFINES += STATICBUILD
} else {
    CONFIG += shared
}

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += TMSCONTROLLER_PLUGIN

TARGET = tmscontroller
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,d)
}

LIBS += -L$${MNE_LIBRARY_DIR}
CONFIG(debug, debug|release) {
    LIBS += -lscSharedd \
            -lscDispd \
            -lscMeasd \
            -lmnecppFiffd \
            -lmnecppUtilsd \
#            -lmnecppMagseriald \
} else {
    LIBS += -lscShared \
            -lscDisp \
            -lscMeas \
            -lmnecppFiff \
            -lmnecppUtils \
#            -lmnecppMagserial \
}

SOURCES += \
        tmscontroller.cpp \
        FormFiles/tmscontrollersetupwidget.cpp \
        FormFiles/tmscontrollerwidget.cpp \
        SerialConnection/connectionrobot.cpp \
        SerialConnection/serialportcontroller.cpp \
        MagstimDevices/magstim.cpp \
        MagstimDevices/rapid.cpp \


HEADERS += \
        tmscontroller.h\
        tmscontroller_global.h \
        FormFiles/tmscontrollersetupwidget.h \
        FormFiles/tmscontrollerwidget.h \
        SerialConnection/connectionrobot.h \
        SerialConnection/serialportcontroller.h \
        MagstimDevices/magstim.h \
        MagstimDevices/rapid.h \

FORMS += \
        FormFiles/tmscontrollersetup.ui \
        FormFiles/tmscontrollerwidget.ui \

RESOURCES += \
        resources.qrc

OTHER_FILES += \
        tmscontroller.json \
        configuration/rapid_config.json \
        configuration/rapid_system_info.json \
DISTFILES += \
        tmscontroller.json \

INCLUDEPATH += $${EIGEN_INCLUDE_DIR}
INCLUDEPATH += $${MNE_INCLUDE_DIR}
INCLUDEPATH += $${MNE_SCAN_INCLUDE_DIR}

# Put generated form headers into the origin --> cause other src is pointing at them
#UI_DIR = $$PWD

# suppress visibility warnings
unix: QMAKE_CXXFLAGS += -Wno-attributes

unix:!macx {
    QMAKE_RPATHDIR += $ORIGIN/../../lib
}

# Activate FFTW backend in Eigen for non-static builds only
contains(MNECPP_CONFIG, useFFTW):!contains(MNECPP_CONFIG, static) {
    DEFINES += EIGEN_FFTW_DEFAULT
    INCLUDEPATH += $$shell_path($${FFTW_DIR_INCLUDE})
    LIBS += -L$$shell_path($${FFTW_DIR_LIBS})

    win32 {
        # On Windows
        LIBS += -llibfftw3-3 \
                -llibfftw3f-3 \
                -llibfftw3l-3 \
    }

    unix:!macx {
        # On Linux
        LIBS += -lfftw3 \
                -lfftw3_threads \
    }
}
