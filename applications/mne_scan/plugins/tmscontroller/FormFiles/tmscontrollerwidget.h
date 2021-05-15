//=============================================================================================================
/**
 * @file     dummyyourwidget.h
 * @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
 *           Viktor Klueber <Viktor.Klueber@tu-ilmenau.de>;
 *           Lorenz Esch <lesch@mgh.harvard.edu>
 * @since    0.1.0
 * @date     January, 2016
 *
 * @section  LICENSE
 *
 * Copyright (C) 2016, Christoph Dinh, Viktor Klueber, Lorenz Esch. All rights reserved.
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
 * @brief    Contains the declaration of the DummyYourWidget class.
 *
 */

#ifndef TMSCONTROLLERWIDGET_H
#define TMSCONTROLLERWIDGET_H

//=============================================================================================================
// INCLUDES
//=============================================================================================================

//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QWidget>

//=============================================================================================================
// EIGEN INCLUDES
//=============================================================================================================

//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================

namespace Ui{
    class TmsControllerWidgetGui;
}

//=============================================================================================================
// DEFINE NAMESPACE TMSCONTROLLERPLUGIN
//=============================================================================================================

namespace TMSCONTROLLERPLUGIN
{

//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================

//=============================================================================================================
/**
 * DECLARE CLASS TmsControllerWidget
 *
 * @brief The TmsController class provides a tmscontroller widget.
 */
class TmsControllerWidget : public QWidget
{
    Q_OBJECT

public:
    typedef QSharedPointer<TmsControllerWidget> SPtr;         /**< Shared pointer type for TmsControllerWidget. */
    typedef QSharedPointer<TmsControllerWidget> ConstSPtr;    /**< Const shared pointer type for TmsControllerWidget. */

    //=========================================================================================================
    /**
     * Constructs a TmsController.
     */
    explicit TmsControllerWidget(const QString& sSettingsPath = "",
                             QWidget *parent = 0);

    //=========================================================================================================
    /**
     * Destroys the TmsController.
     */
    ~TmsControllerWidget();

    //=========================================================================================================
    /**
     * Updates parameters in the control widget Gui.
     */
    void updateGuiPort(QString currentPort);
    void updateDeviceStatus(bool isReadyToFire, QString currentStatus);

    QString getDevice();
    QString getPortName();
    QString getUnlockCode();
    int     getVoltage();
    int     getSuperRapid();
    int     getPulseMode();
    int     getPowerMode();
    int     getStaticPower();
    int     getPulseNum();
    double  getDeadTime();
    double  getWait();
    int     getFrequency();
    double  getDuration();

    void onChangeDevice();
    void onChangePortName();
    void onChangeUnlockCode();
    void onChangeVoltage();
    void onChangeSuperRapid();
    void onChangePulseMode();
    void onChangePowerMode();
    void onChangeStaticPower();
    void onChangePulseNum();
    void onChangeDeadTime();
    void onChangeWait();
    void onChangeFrequency();
    void onChangeDuration();

    void onClickDefault();
    void onClickFire();

private:
    //=========================================================================================================
    /**
     * Saves all important settings of this view via QSettings.
     */
    void saveSettings();

    //=========================================================================================================
    /**
     * Loads and inits all important settings of this view via QSettings.
     */
    void loadSettings();

    //=========================================================================================================
    /**
     * @brief m_pUi
     */
    Ui::TmsControllerWidgetGui*     m_pUi;              /**< The UI class specified in the designer. */
    QString                         m_sSettingsPath;    /**< The settings path to store the GUI settings to. */

    //=========================================================================================================
    /**
     * GUI parameters
     */
    bool        m_bIsParametersChanged = false;     // parameters in GUI is changed.
    double      m_dWait;
    double      m_dDuration;

    int         m_iDevice;      // 0-Rapid2; 1-Bistim2.
    int         m_iVoltage;     // 0-240 V; 1-115 V.
    int         m_iSuperRapid;
    int         m_iPulseMode;   // 0-Single pulse; 1-rTMS.
    int         m_iPowerMode;   // 0-Static; 1-Dynamic.
    int         m_iStaticPower;
    int         m_iPulses;
    int         m_iDeadTime;
    int         m_iFrequency;

    QString     m_sDevice = "Rapid";
    QString     m_sSoftWareVersion = "NV 7.2.0";
    QString     m_sPortName;
    QString     m_sUnlockCode = nullptr;

signals:
    void sig_clickDefault();
    void sig_clickFire();

    void sig_changeStaticPower(int newPower);
    void sig_changeSuperRapid(int newSuperRapid);
    void sig_changeUnlockCode(QString newUnlockCode);
    void sig_changeVoltage(int newVoltage);

};
}   //namespace

#endif // TMSCONTROLLERWIDGET_H
