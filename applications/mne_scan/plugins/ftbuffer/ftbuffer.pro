#--------------------------------------------------------------------------------------------------------------
#
# @file     ftbuffer.pro
# @author   Gabriel Motta <gbmotta@mgh.harvard.edu>
#           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
# @version  0.1
# @date     January, 2020
#
# @section  LICENSE
#
# Copyright (C) 2020, Gabriel Motta and Matti Hamalainen. All rights reserved.
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
# @brief    This project file generates the makefile for the ftbuffer plug-in.
#
#--------------------------------------------------------------------------------------------------------------

include(../../../../mne-cpp.pri)

TEMPLATE = lib

CONFIG += plugin

DEFINES += FTBUFFER_LIBRARY

QT += core widgets network testlib


TARGET = ftbuffer
CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,d)
}

DESTDIR = $${MNE_BINARY_DIR}/mne_scan_plugins

contains(MNECPP_CONFIG, static) {
    CONFIG += staticlib
    DEFINES += STATICLIB
} else {
    CONFIG += shared
}


LIBS += -L$${MNE_LIBRARY_DIR}
CONFIG(debug, debug|release) {
    LIBS += -lMNE$${MNE_LIB_VERSION}Utilsd \
            -lscMeasd \
            -lscDispd \
            -lscSharedd
}
else {
    LIBS += -lMNE$${MNE_LIB_VERSION}Utils \
            -lscMeas \
            -lscDisp \
            -lscShared
}

SOURCES += \
    ftbuffer.cpp \
    FormFiles/ftbuffersetupwidget.cpp \
    FormFiles/ftbufferaboutwidget.cpp \
    FormFiles/ftbufferyourwidget.cpp \
    ftbuffproducer.cpp \
        ftsrc/cpp/FtConnection.cc \
        ftsrc/src/cleanup.c \
        ftsrc/src/clientrequest.c \
        ftsrc/src/compat_clock_gettime.c \
        ftsrc/src/compat_fsync.c \
        ftsrc/src/compat_gettimeofday.c \
        ftsrc/src/dmarequest.c \
        ftsrc/src/endianutil.c \
        ftsrc/src/extern.c \
        ftsrc/src/interface.c \
        ftsrc/src/printstruct.c \
        ftsrc/src/rdaserver.c \
        ftsrc/src/socketserver.c \
        ftsrc/src/swapbytes.c \
        ftsrc/src/tcprequest.c \
        ftsrc/src/tcpsocket.c \
        ftsrc/src/tcpserver.c \
        ftsrc/src/timestamp.c \
        ftsrc/src/util.c \
        ftsrc/ftbuffclient.cpp \

HEADERS += \
    ftbuffer_global.h \
    ftbuffer.h \
    FormFiles/ftbuffersetupwidget.h \
    FormFiles/ftbufferaboutwidget.h \
    FormFiles/ftbufferyourwidget.h \
    ftbuffproducer.h

FORMS += \
    FormFiles/ftbuffersetup.ui \
    FormFiles/ftbufferabout.ui \
    FormFiles/ftbufferyourtoolbarwidget.ui

INCLUDEPATH += $${EIGEN_INCLUDE_DIR}
INCLUDEPATH += $${MNE_INCLUDE_DIR}
INCLUDEPATH += $${MNE_SCAN_INCLUDE_DIR}
INCLUDEPATH += $$MNE_BINARY_DIR/../applications/mne_scan/plugins/ftbuffer/ftsrc/cpp
INCLUDEPATH += $$MNE_BINARY_DIR/../applications/mne_scan/plugins/ftbuffer/ftsrc/src
INCLUDEPATH += $$MNE_BINARY_DIR/../applications/mne_scan/plugins/ftbuffer/ftsrc/

OTHER_FILES += ftbuffer.json

UI_DIR = $$PWD

unix:!macx {
    # === Unix ===
    QMAKE_RPATHDIR += $ORIGIN/../../lib
}

# Put generated form headers into the origin --> cause other src is pointing at them
UI_DIR = $${PWD}

unix: QMAKE_CXXFLAGS += -isystem $$EIGEN_INCLUDE_DIR

# suppress visibility warnings
unix: QMAKE_CXXFLAGS += -Wno-attributes

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