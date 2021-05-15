//=============================================================================================================
/**
 * @file     dummysetupwidget.cpp
 * @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
 *           Lorenz Esch <lesch@mgh.harvard.edu>;
 *           Viktor Klueber <Viktor.Klueber@tu-ilmenau.de>
 * @since    0.1.0
 * @date     February, 2013
 *
 * @section  LICENSE
 *
 * Copyright (C) 2013, Christoph Dinh, Lorenz Esch, Viktor Klueber. All rights reserved.
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
 * @brief    Definition of the TmsControllerSetupWidget class.
 *
 */

//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "../tmscontroller.h"
#include "tmscontrollersetupwidget.h"

//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QDebug>
#include <QSerialPortInfo>

//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace TMSCONTROLLERPLUGIN;
using namespace Ui;

//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

TmsControllerSetupWidget::TmsControllerSetupWidget(TmsController* toolbox, QWidget *parent)
    : QWidget(parent)
    , m_pTmsController(toolbox)
    , ui(new TmsControllerSetupWidgetClass)
{
    ui->setupUi(this);

    // Load all available serial port names
    const auto qSerialPortList = QSerialPortInfo::availablePorts();
    ui->m_qComboBox_serialPort->setPlaceholderText("--Select Port--");
    for (const QSerialPortInfo &info: qSerialPortList)
        ui->m_qComboBox_serialPort->addItem(info.portName());

    ui->m_qComboBox_magstimDevices->setPlaceholderText("--Select Device--");
    ui->m_qComboBox_magstimDevices->addItem("Rapid");
    ui->m_qComboBox_magstimDevices->addItem("Bistim");
    ui->m_qComboBox_magstimDevices->addItem("200^2");

    ui->m_qComboBox_magstimDevices->setCurrentIndex(0);
    ui->m_qComboBox_magstimDevices->setEnabled(false);

    ui->m_qLabel_magstimVersion->setVisible(false);

    this->init();

    //Always connect GUI elemts after ui->setpUi has been called
    QObject::connect(ui->m_qPushButton_checkConnection, &QPushButton::released,
                     this, &TmsControllerSetupWidget::pressedCheckConnection);
    QObject::connect(ui->m_qComboBox_serialPort, &QComboBox::currentTextChanged,
                     this, &TmsControllerSetupWidget::portNameChanged);
    QObject::connect(ui->m_qComboBox_magstimDevices, &QComboBox::currentTextChanged,
                     this, &TmsControllerSetupWidget::deviceNameChanged);
}

//=============================================================================================================

TmsControllerSetupWidget::~TmsControllerSetupWidget()
{
}

//=============================================================================================================

void TmsControllerSetupWidget::init()
{
    bool isConnectable = std::get<0>(m_pTmsController->currentCheckStatus());
    QString checkInfo = std::get<1>(m_pTmsController->currentCheckStatus());
    std::tuple<int, int, int> baseVersion = std::get<2>(m_pTmsController->currentCheckStatus());

    if (!m_pTmsController->currentPortName().isEmpty())
    {
        ui->m_qComboBox_serialPort->setCurrentText(m_pTmsController->currentPortName());
    }

    if (!m_pTmsController->currentDeviceName().isEmpty())
    {
        ui->m_qComboBox_magstimDevices->setCurrentText(m_pTmsController->currentDeviceName());
    }

    if (isConnectable)
    {
        ui->m_qLabel_magstimVersion->setVisible(true);
        ui->m_qLabel_magstimVersion->setText(tr("(NV %1.%2.%3)")
                                             .arg(std::get<0>(baseVersion))
                                             .arg(std::get<1>(baseVersion))
                                             .arg(std::get<2>(baseVersion)));
    }
    ui->m_qLabel_connectionStatus->setText(checkInfo);
}

//=============================================================================================================

void TmsControllerSetupWidget::pressedCheckConnection()
{
    QString portName = ui->m_qComboBox_serialPort->currentText();
    QString deviceName = ui->m_qComboBox_magstimDevices->currentText();
    QString checkInfo;
    std::tuple<int, int, int> version = std::make_tuple(0,0,0);
    int error = 0, error2 = 0, error3 = 0;
    if (!portName.isEmpty())
    {

        // QString portName, int superRapid, QString unlock, int voltage, version
        Rapid pMyRapid(portName, 0, "", 240, version);
        pMyRapid.connect(error);
        if (!error)
        {
            version = pMyRapid.getVersion(error2);
            if (error2)
            {
                ui->m_qLabel_connectionStatus->setText(tr("Software version unsolved. Disconnecting with code 1%.").arg(error2));
                checkInfo = ui->m_qLabel_connectionStatus->text();
                m_pTmsController->updateCheckStatus(false, checkInfo, version);
                pMyRapid.disconnect(error3);
            }
            else
            {
                pMyRapid.disconnect(error3);
                if (!error3)
                {
                    ui->m_qLabel_connectionStatus->setText(tr("Connection is good, enjoy!"));
                    ui->m_qLabel_magstimVersion->setVisible(true);
                    ui->m_qLabel_magstimVersion->setText(tr("(NV %1.%2.%3)")
                                                         .arg(std::get<0>(version))
                                                         .arg(std::get<1>(version))
                                                         .arg(std::get<2>(version)));
                    checkInfo = ui->m_qLabel_connectionStatus->text();
                    m_pTmsController->updateCheckStatus(true, checkInfo, version);
                }
                else
                {
                    ui->m_qLabel_connectionStatus->setText(tr("Disconnect error with code %1.").arg(error3));
                    checkInfo = ui->m_qLabel_connectionStatus->text();
                    m_pTmsController->updateCheckStatus(false, checkInfo, version);
                }
            }
        }
        else
        {
            ui->m_qLabel_connectionStatus->setText(tr("Connect error with code %1.").arg(error));
            checkInfo = ui->m_qLabel_connectionStatus->text();
            m_pTmsController->updateCheckStatus(false, checkInfo, version);
        }
    }
    else
    {
        //QMessageBox msgBox;
        //msgBox.setText((tr("Please selecte a serial port!")));
        //msgBox.exec();
        ui->m_qLabel_connectionStatus->setText(tr("Nonvalid port name. Please selecte a serial port!"));
        checkInfo = ui->m_qLabel_connectionStatus->text();
        m_pTmsController->updateCheckStatus(false, checkInfo, version);
    }

}

//=============================================================================================================

void TmsControllerSetupWidget::portNameChanged()
{
    qDebug() << "TmsControllerSetupWidget::deviceNameChanged.";

    QString currentPort = ui->m_qComboBox_serialPort->currentText();
    ui->m_qLabel_connectionStatus->setText(tr("Connection not specified."));
    QString checkInfo = ui->m_qLabel_connectionStatus->text();
    m_pTmsController->updateCheckStatus(false, checkInfo, std::make_tuple(0,0,0));
    m_pTmsController->updatePortName(currentPort);
}

//=============================================================================================================

void TmsControllerSetupWidget::deviceNameChanged()
{
    qDebug() << "TmsControllerSetupWidget::deviceNameChanged.";

    QString currentDevice = ui->m_qComboBox_serialPort->currentText();
    ui->m_qLabel_connectionStatus->setText(tr("Connection not specified."));
    QString checkInfo = ui->m_qLabel_connectionStatus->text();
    m_pTmsController->updateCheckStatus(false, checkInfo, std::make_tuple(0,0,0));
    m_pTmsController->updateDeviceName(currentDevice);
}
