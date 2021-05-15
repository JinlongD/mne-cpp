//=============================================================================================================
/**
 * @file     dummytoolbox.h
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
 * @brief    Contains the declaration of the DummyToolbox class.
 *
 */

#ifndef TMSCONTROLLER_H
#define TMSCONTROLLER_H

//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "tmscontroller_global.h"
#include "MagstimDevices/magstim.h"
#include "MagstimDevices/rapid.h"

//#include <magserial/magstimdevices/magstim.h>
//#include <magserial/magstimdevices/rapid.h>

#include <scShared/Plugins/abstractalgorithm.h>
#include <utils/generics/circularbuffer.h>

//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

//=============================================================================================================
// EIGEN INCLUDES
//=============================================================================================================

//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================

namespace SCMEASLIB {
    class RealTimeMultiSampleArray;
}

namespace FIFFLIB {
    class FiffInfo;
}

//=============================================================================================================
// DEFINE NAMESPACE TMSCONTROLLERPLUGIN
//=============================================================================================================

namespace TMSCONTROLLERPLUGIN
{

//=============================================================================================================
// TMSCONTROLLERPLUGIN FORWARD DECLARATIONS
//=============================================================================================================

//class TmsControllerWidget;

//=============================================================================================================
/**
 * DECLARE CLASS tmscontroller
 *
 * @brief The tmscontroller class provides a tmscontroller algorithm structure.
 */
class TMSCONTROLLERSHARED_EXPORT TmsController : public SCSHAREDLIB::AbstractAlgorithm
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "scsharedlib/1.0" FILE "tmscontroller.json") //New Qt5 Plugin system replaces Q_EXPORT_PLUGIN2 macro
    // Use the Q_INTERFACES() macro to tell Qt's meta-object system about the interfaces
    Q_INTERFACES(SCSHAREDLIB::AbstractAlgorithm)

public:
    //=========================================================================================================
    /**
     * Constructs a DummyToolbox.
     */
    TmsController();

    //=========================================================================================================
    /**
     * Destroys the DummyToolbox.
     */
    ~TmsController();

    //=========================================================================================================
    /**
     * Initialise input and output connectors.
     */
    virtual void init();

    //=========================================================================================================
    /**
     * Clone the plugin.
     */
    virtual QSharedPointer<SCSHAREDLIB::AbstractPlugin> clone() const;

    //=========================================================================================================
    /**
     * Returns the plugin name.
     */
    virtual QString getName() const;

    //=========================================================================================================
    /**
     * Returns the plugin type (Algorithm or Sensor).
     */
    virtual SCSHAREDLIB::AbstractPlugin::PluginType getType() const; 

    //=========================================================================================================
    /**
     * Returns the setup widget for the plugin configuration in the mainwindow.
     */
    virtual QWidget* setupWidget();

    //=========================================================================================================
    /**
     * Starts the QThread of this plugin.
     */
    virtual bool start();

    //=========================================================================================================
    /**
     * Stops the QThread of this plugin.
     */
    virtual bool stop();

    //=========================================================================================================
    /**
     * Is called when plugin is detached of the stage. Can be used to safe settings.
     */
    virtual void unload();

    //=========================================================================================================
    /**
     * Udates the pugin with new (incoming) data.
     *
     * @param[in] pMeasurement    The incoming data in form of a generalized Measurement.
     */
    void update(SCMEASLIB::Measurement::SPtr pMeasurement);

    //=========================================================================================================
    /**
     * Settings for the setupWidget
     * Get serial port name from the setupwidget GUI.
     */
    void updatePortName(QString portName);
    void updateDeviceName(QString deviceName);
    void updateCheckStatus(bool bIsChecked, QString sCheckInfo, std::tuple<int, int, int> baseVersion);
    QString currentPortName();
    QString currentDeviceName();
    std::tuple<bool, QString, std::tuple<int, int, int>> currentCheckStatus();

protected:
    //=========================================================================================================
    /**
     * Inits quick control widgets which are used to control this plugin, then emits them in form of a QList.
     */
    virtual void initPluginControlWidgets();

    //=========================================================================================================
    /**
     * AbstractAlgorithm function
     */
    virtual void run();

    //=========================================================================================================
    /*
     *
     */
    void onClickFire();
    void onClickDefault();

    void onChangeStaticPower(int newPower);
    void onChangeSuperRapid(int newSuperRapid);
    void onChangeUnlockCode(QString newUnlockCode);
    void onChangeVoltage(int newVoltage);

private:
    //FIFFLIB::FiffInfo::SPtr         m_pFiffInfo;            /**< Fiff measurement info.*/

    SCSHAREDLIB::PluginInputData<SCMEASLIB::RealTimeMultiSampleArray>::SPtr      m_pInput;      /**< The incoming data.*/
    //SCSHAREDLIB::PluginOutputData<SCMEASLIB::RealTimeMultiSampleArray>::SPtr     m_pOutput;     /**< The outgoing data.*/

    QSharedPointer<UTILSLIB::CircularBuffer_Matrix_double>  m_pCircularBuffer;      /**< Holds incoming raw data. */

    QSharedPointer<FIFFLIB::FiffInfo>     m_pFiffInfo;            /**< Fiff measurement info.*/
    //QSharedPointer<TmsControllerWidget>   m_pTmsControllerWidget; /**< The widget used to control this plugin by the user.*/

    bool        m_bIsRunning = false;               /**< tmscontroller thread is running after called start().*/
    bool        m_bIsConnectable = false;           /**< TMS device is connectable via current serial port settings.*/
    bool        m_bIsToFire = false;                /**< TMS device is ready to make a shot.*/

    int         m_iStaticPower;
    int         m_iSuperRapid;
    int         m_iVoltage;

    QString     m_sCheckInfo;
    QString     m_sPortName = "";
    QString     m_sDeviceName = "";
    QString     m_sUnlockCode;

    QMutex      m_mutex;

    std::tuple<int, int, int>   m_tBaseVersion;

signals:
    //=========================================================================================================
    /**
     * Emitted when fiffInfo is available
     */
    void fiffInfoAvailable();

    //=========================================================================================================
    /**
     * Emitted when current device status is changed.
     */
    void sig_updateDeviceStatus(bool isReadyToFire, QString currentStatus);
};
} // NAMESPACE

#endif // TMSCONTROLLER_H
