﻿//=============================================================================================================
/**
 * @file     dummysetupwidget.h
 * @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
 *           Viktor Klueber <Viktor.Klueber@tu-ilmenau.de>
 * @since    0.1.0
 * @date     February, 2013
 *
 * @section  LICENSE
 *
 * Copyright (C) 2013, Christoph Dinh, Viktor Klueber. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 * the following conditions are met:
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
 *       following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
 *       the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of MNE-CPP authors nor the names of its contributors may be used
 *       to endorse or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * @brief    Contains the declaration of the DummySetupWidget class.
 *
 */

#ifndef TMSCONTROLLERSETUPWIDGET_H
#define TMSCONTROLLERSETUPWIDGET_H

//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "ui_tmscontrollersetup.h"

//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QtWidgets>

//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================

namespace Ui {
    class TmsControllerSetupWidgetClass;
}

//=============================================================================================================
// DEFINE NAMESPACE TMSCONTROLLERPLUGIN
//=============================================================================================================

namespace TMSCONTROLLERPLUGIN
{

//=============================================================================================================
// TMSCONTROLLERPLUGIN FORWARD DECLARATIONS
//=============================================================================================================

class TmsController;

//=============================================================================================================
/**
 * DECLARE CLASS TmsControllerSetupWidget
 *
 * @brief The TmsControllerSetupWidget class provides the TmsController configuration window.
 */
class TmsControllerSetupWidget : public QWidget
{
    Q_OBJECT

public:

    //=========================================================================================================
    /**
     * Constructs a TmsControllerSetupWidget which is a child of parent.
     *
     * @param [in] toolbox a pointer to the corresponding TmsController.
     * @param [in] parent pointer to parent widget; If parent is 0, the new TmsControllerSetupWidget becomes a window.
     *                  If parent is another widget, TmsControllerSetupWidget becomes a child window inside parent.
     *                  TmsControllerSetupWidget is deleted when its parent is deleted.
     */
    TmsControllerSetupWidget(TmsController* toolbox, QWidget *parent = 0);

    //=========================================================================================================
    /**
     * Destroys the TmsControllerSetupWidget.
     * All TmsControllerSetupWidget's children are deleted first.
     * The application exits if TmsControllerSetupWidget is the main widget.
     */
    ~TmsControllerSetupWidget();

    //=========================================================================================================
    /**
     * Triggered a checking of the serial port connection status.
     */
    void pressedCheckConnection();

    //=========================================================================================================
    /**
     * update serial port name from the setupGui.
     */
    void portNameChanged();

    //=========================================================================================================
    /**
     * update magstim device name from the setupGui.
     */
    void deviceNameChanged();

    //=========================================================================================================
    /**
     * Destroys the TmsControllerSetupWidget.
     * All TmsControllerSetupWidget's children are deleted first.
     * The application exits if TmsControllerSetupWidget is the main widget.
     */
    void init();

private:
    TmsController*               m_pTmsController;	/**< Holds a pointer to corresponding TmsController.*/

    Ui::TmsControllerSetupWidgetClass*  ui;              /**< Holds the user interface for the TmsControllerSetupWidget.*/

};
} // NAMESPACE

#endif // TMSCONTROLLERSETUPWIDGET_H
