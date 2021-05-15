//=============================================================================================================
/**
 * @file     dummyyourwidget.cpp
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
 * @brief    Definition of the TmsControllerWidget class.
 *
 */

//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "tmscontrollerwidget.h"
#include "ui_tmscontrollerwidget.h"

//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QSettings>

//=============================================================================================================
// EIGEN INCLUDES
//=============================================================================================================

//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace TMSCONTROLLERPLUGIN;

//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

TmsControllerWidget::TmsControllerWidget(const QString& sSettingsPath,
                                         QWidget *parent)
    : QWidget(parent)
    , m_pUi(new Ui::TmsControllerWidgetGui)
{
    m_sSettingsPath = sSettingsPath;
    m_pUi->setupUi(this);

    m_pUi->m_qLabel_Frequency->setEnabled(false);
    m_pUi->m_qSpinBox_Frequency->setEnabled(false);
    m_pUi->m_qLabel_Duration->setEnabled(false);
    m_pUi->m_qDoubleSpinBox_Duration->setEnabled(false);
    //m_pUi->m_qPushButton_quickFire->setEnabled(false);

    loadSettings();

    //Always connect GUI elemts after m_pUi->setpUi has been called
    QObject::connect(m_pUi->m_qPushButton_Default, &QPushButton::released,
            this, &TmsControllerWidget::onClickDefault);
    QObject::connect(m_pUi->m_qPushButton_quickFire, &QPushButton::released,
            this, &TmsControllerWidget::onClickFire);

    QObject::connect(m_pUi->m_qSpinBox_Power, static_cast<void (QSpinBox::*)()>(&QSpinBox::editingFinished),
            this, &TmsControllerWidget::onChangeStaticPower);
    QObject::connect(m_pUi->m_qLineEdit_unlockCode, static_cast<void (QLineEdit::*)()>(&QLineEdit::editingFinished),
            this, &TmsControllerWidget::onChangeUnlockCode);
    QObject::connect(m_pUi->m_qComboBox_operatingVoltage, &QComboBox::currentTextChanged,
            this, &TmsControllerWidget::onChangeVoltage);
    QObject::connect(m_pUi->m_qComboBox_superRapid, &QComboBox::currentTextChanged,
            this, &TmsControllerWidget::onChangeSuperRapid);

    /*
    QObject::connect(m_pUi->m_qComboBox_Device, &QComboBox::currentTextChanged,
            this, &TmsControllerWidget::onChangeDevice);
    QObject::connect(m_pUi->m_qComboBox_Mode, &QComboBox::currentTextChanged,
            this, &TmsControllerWidget::onChangePulseMode);
    QObject::connect(m_pUi->m_qComboBox_Power, &QComboBox::currentTextChanged,
            this, &TmsControllerWidget::onChangePowerMode);
    QObject::connect(m_pUi->m_qSpinBox_Power, static_cast<void (QSpinBox::*)()>(&QSpinBox::editingFinished),
            this, &TmsControllerWidget::onChangeStaticPower);
    QObject::connect(m_pUi->m_qSpinBox_Pulses, static_cast<void (QSpinBox::*)()>(&QSpinBox::editingFinished),
            this, &TmsControllerWidget::onChangePulseNum);
    QObject::connect(m_pUi->m_qSpinBox_deadTime, static_cast<void (QSpinBox::*)()>(&QSpinBox::editingFinished),
            this, &TmsControllerWidget::onChangeDeadTime);
    QObject::connect(m_pUi->m_qDoubleSpinBox_Wait, static_cast<void (QDoubleSpinBox::*)()>(&QDoubleSpinBox::editingFinished),
            this, &TmsControllerWidget::onChangeWait);
    QObject::connect(m_pUi->m_qSpinBox_Frequency, static_cast<void (QSpinBox::*)()>(&QSpinBox::editingFinished),
            this, &TmsControllerWidget::onChangeFrequency);
    QObject::connect(m_pUi->m_qDoubleSpinBox_Duration, static_cast<void (QDoubleSpinBox::*)()>(&QDoubleSpinBox::editingFinished),
            this, &TmsControllerWidget::onChangeDuration);
    */


}

//=============================================================================================================

TmsControllerWidget::~TmsControllerWidget()
{
    saveSettings();

    delete m_pUi;
}

//=============================================================================================================

void TmsControllerWidget::saveSettings()
{
    if(m_sSettingsPath.isEmpty()) {
        return;
    }

    QSettings settings("MNECPP");

    //settings.setValue(m_sSettingsPath + QString("/valueName"), m_pUi->m_pDoubleSpinBox_dummy->value());
}

//=============================================================================================================

void TmsControllerWidget::loadSettings()
{
    if(m_sSettingsPath.isEmpty()) {
        return;
    }

    QSettings settings("MNECPP");

    //m_pUi->m_pDoubleSpinBox_dummy->setValue(settings.value(m_sSettingsPath + QString("/valueName"), 10).toInt());
}

//=============================================================================================================

void TmsControllerWidget::updateGuiPort(QString currentPort)
{
    m_sPortName = currentPort;
    m_pUi->m_qLabel_portName_show->setText(currentPort);
}

void TmsControllerWidget::updateDeviceStatus(bool isReadyToFire, QString currentStatus)
{
    m_pUi->m_qLabel_deviceStatus_show->setText(currentStatus);
    m_pUi->m_qPushButton_quickFire->setEnabled(isReadyToFire);
}
//=============================================================================================================

QString TmsControllerWidget::getDevice()
{
    return m_pUi->m_qComboBox_Device->currentText();
}

QString TmsControllerWidget::getPortName()
{
    return m_pUi->m_qLabel_portName_show->text();
}

QString TmsControllerWidget::getUnlockCode()
{
    return m_pUi->m_qLineEdit_unlockCode->text();
}

int TmsControllerWidget::getVoltage()
{
    int currentIndex = m_pUi->m_qComboBox_operatingVoltage->currentIndex();
    if (currentIndex == 0)
        return 240; // 240 Volt
    else
        return 115; // 115 Volt
}

int TmsControllerWidget::getSuperRapid()
{
    return m_pUi->m_qComboBox_superRapid->currentIndex();
}

int TmsControllerWidget::getPulseMode()
{
    return m_pUi->m_qComboBox_Mode->currentIndex();
}

int TmsControllerWidget::getPowerMode()
{
    return m_pUi->m_qComboBox_Power->currentIndex();
}

int TmsControllerWidget::getStaticPower()
{
    return m_pUi->m_qSpinBox_Power->value();
}

int TmsControllerWidget::getPulseNum()
{
    return m_pUi->m_qSpinBox_Pulses->value();
}

double TmsControllerWidget::getDeadTime()
{
    return m_pUi->m_qSpinBox_deadTime->value();
}

double TmsControllerWidget::getWait()
{
    return m_pUi->m_qDoubleSpinBox_Wait->value();
}

int TmsControllerWidget::getFrequency()
{
    return m_pUi->m_qSpinBox_Frequency->value();
}

double TmsControllerWidget::getDuration()
{
    return m_pUi->m_qDoubleSpinBox_Duration->value();
}

//=============================================================================================================

void TmsControllerWidget::onChangeDevice()
{}

void TmsControllerWidget::onChangePortName()
{}

void TmsControllerWidget::onChangeUnlockCode()
{
    m_sUnlockCode = getUnlockCode();
    emit sig_changeUnlockCode(m_sUnlockCode);
}

void TmsControllerWidget::onChangeVoltage()
{
    m_iVoltage = getVoltage();
    emit sig_changeVoltage(m_iVoltage);
}

void TmsControllerWidget::onChangeSuperRapid()
{
    m_iSuperRapid = getSuperRapid();
    emit sig_changeSuperRapid(m_iSuperRapid);
}

void TmsControllerWidget::onChangePulseMode()
{}

void TmsControllerWidget::onChangePowerMode()
{}

void TmsControllerWidget::onChangeStaticPower()
{
    m_iStaticPower = getStaticPower();
    //m_iStaticPower = m_pUi->m_qSpinBox_Power->value();
    emit sig_changeStaticPower(m_iStaticPower);
}

void TmsControllerWidget::onChangePulseNum()
{}

void TmsControllerWidget::onChangeDeadTime()
{}

void TmsControllerWidget::onChangeWait()
{}

void TmsControllerWidget::onChangeFrequency()
{}

void TmsControllerWidget::onChangeDuration()
{}

void TmsControllerWidget::onClickDefault()
{
    m_pUi->m_qLabel_portName_show->setText(m_sPortName);
    m_pUi->m_qLineEdit_unlockCode->setText("");
    m_pUi->m_qComboBox_operatingVoltage->setCurrentIndex(0);
    m_pUi->m_qComboBox_superRapid->setCurrentIndex(0);
    m_pUi->m_qComboBox_Mode->setCurrentIndex(0);
    m_pUi->m_qComboBox_Power->setCurrentIndex(0);
    m_pUi->m_qSpinBox_Power->setValue(20);
    m_pUi->m_qSpinBox_Pulses->setValue(1);
    m_pUi->m_qSpinBox_deadTime->setValue(3);
    m_pUi->m_qDoubleSpinBox_Wait->setValue(0);
    m_pUi->m_qSpinBox_Frequency->setValue(10);
    m_pUi->m_qDoubleSpinBox_Duration->setValue(0);

    emit sig_clickDefault();
}

void TmsControllerWidget::onClickFire()
{
    emit sig_clickFire();
}
