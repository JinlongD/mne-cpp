//=============================================================================================================
/**
 * @file     dummytoolbox.cpp
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
 * @brief    Definition of the TmsController class.
 *
 */

//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "tmscontroller.h"

#include "FormFiles/tmscontrollersetupwidget.h"
#include "FormFiles/tmscontrollerwidget.h"

#include <scMeas/realtimemultisamplearray.h>
#include <fiff/fiff_info.h>

//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QtWidgets>
#include <QtCore/QtPlugin>
#include <QDebug>
//#include <QMutexLocker>
#include <QMessageBox>

//=============================================================================================================
// EIGEN INCLUDES
//=============================================================================================================

#include <Eigen/Core>

//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace TMSCONTROLLERPLUGIN;
using namespace SCSHAREDLIB;
using namespace SCMEASLIB;
using namespace UTILSLIB;
using namespace FIFFLIB;
using namespace Eigen;

//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

TmsController::TmsController()
    : m_pCircularBuffer(QSharedPointer<CircularBuffer_Matrix_double>(new CircularBuffer_Matrix_double(40)))
{
}

//=============================================================================================================

TmsController::~TmsController()
{
    if(this->isRunning())
    {
        stop();
    }
}

//============================================================================================================= init

void TmsController::init()
{
    // Input
    m_pInput = PluginInputData<RealTimeMultiSampleArray>::create(this, "TmsControllerIn", "TmsController input data");
    connect(m_pInput.data(), &PluginInputConnector::notify,
            this, &TmsController::update, Qt::DirectConnection);
    m_inputConnectors.append(m_pInput);

    /*
    // Output - Uncomment this if you don't want to send processed data (in form of a matrix) to other plugins.
    // Also, this output stream will generate an online display in your plugin
    m_pOutput = PluginOutputData<RealTimeMultiSampleArray>::create(this, "TmsControllerOut", "TmsController output data");
    m_pOutput->measurementData()->setName(this->getName());
    m_outputConnectors.append(m_pOutput);
    */
}

//============================================================================================================= initPluginControlWidgets

void TmsController::initPluginControlWidgets()
{
    if(m_pFiffInfo)
    {
        QList<QWidget*> plControlWidgets;

        // The plugin's quick control widget
        TmsControllerWidget* pTmsControllerWidget = new TmsControllerWidget(QString("MNESCAN/%1").arg(this->getName()));
        pTmsControllerWidget->setObjectName("group_tab_Settings");

        pTmsControllerWidget->updateGuiPort(m_sPortName);
        pTmsControllerWidget->updateDeviceStatus(false, "Disconnected.");

        QObject::connect(pTmsControllerWidget, &TmsControllerWidget::sig_changeStaticPower,
                         this, &TmsController::onChangeStaticPower, Qt::ConnectionType::DirectConnection);
        QObject::connect(pTmsControllerWidget, &TmsControllerWidget::sig_changeSuperRapid,
                         this, &TmsController::onChangeSuperRapid, Qt::ConnectionType::DirectConnection);
        QObject::connect(pTmsControllerWidget, &TmsControllerWidget::sig_changeUnlockCode,
                         this, &TmsController::onChangeUnlockCode, Qt::ConnectionType::DirectConnection);
        QObject::connect(pTmsControllerWidget, &TmsControllerWidget::sig_changeVoltage,
                         this, &TmsController::onChangeVoltage, Qt::ConnectionType::DirectConnection);

        QObject::connect(pTmsControllerWidget, &TmsControllerWidget::sig_clickFire,
                         this, &TmsController::onClickFire, Qt::ConnectionType::DirectConnection);
        QObject::connect(pTmsControllerWidget, &TmsControllerWidget::sig_clickDefault,
                         this, &TmsController::onClickDefault, Qt::ConnectionType::DirectConnection);

        QObject::connect(this, &TmsController::sig_updateDeviceStatus,
                         pTmsControllerWidget, &TmsControllerWidget::updateDeviceStatus, Qt::ConnectionType::DirectConnection);

        plControlWidgets.append(pTmsControllerWidget);
        emit pluginControlWidgetsChanged(plControlWidgets, this->getName());

        m_bPluginControlWidgetsInit = true;
    }
}

//============================================================================================================= clone

QSharedPointer<AbstractPlugin> TmsController::clone() const
{
    QSharedPointer<TmsController> pClone(new TmsController);
    return pClone;
}

//============================================================================================================= getName

QString TmsController::getName() const
{
    return "TmsController";
}

//============================================================================================================= getType

AbstractPlugin::PluginType TmsController::getType() const
{
    return _IAlgorithm;
}

//============================================================================================================= setWidget

QWidget* TmsController::setupWidget()
{
    // SetupWidgets docked in the mainwindow is later destroyed by CentralWidget (MainWindow)
    // - So it has to be created everytime new.
    TmsControllerSetupWidget* pSetupWidget = new TmsControllerSetupWidget(this);
    return pSetupWidget;
}

//============================================================================================================= start

bool TmsController::start()
{
    if (m_bIsConnectable)
    {
        m_bIsRunning = true;
        QThread::start();   //Start thread
        //this->start();
        return true;
    }
    else
    {
        //QMessageBox msgBox;
        //msgBox.setText("Please check the TMS Controller connection!");
        //msgBox.exec();
        return false;
    }
}

//============================================================================================================= stop

bool TmsController::stop()
{
    m_bIsRunning = false;
    m_bPluginControlWidgetsInit = false;

    requestInterruption();
    wait(500);

    // Clear all data in the buffer connected to displays and other plugins
    //m_pOutput->measurementData()->clear();
    m_pCircularBuffer->clear();

    return true;
}

//============================================================================================================= unload

void TmsController::unload()
{
}

//============================================================================================================= update

void TmsController::update(SCMEASLIB::Measurement::SPtr pMeasurement)
{
    if(QSharedPointer<RealTimeMultiSampleArray> pRTMSA = pMeasurement.dynamicCast<RealTimeMultiSampleArray>()) {
        //Fiff information
        if(!m_pFiffInfo)
        {
            m_pFiffInfo = pRTMSA->info();

            //m_pOutput->measurementData()->initFromFiffInfo(m_pFiffInfo);
            //m_pOutput->measurementData()->setMultiArraySize(1);
        }

        if(!m_bPluginControlWidgetsInit)
        {
            initPluginControlWidgets();
        }

        for(unsigned char i = 0; i < pRTMSA->getMultiArraySize(); ++i)
        {
            // Please note that we do not need a copy here since this function will block until
            // the buffer accepts new data again. Hence, the data is not deleted in the actual
            // Mesaurement function after it emitted the notify signal.
            while(!m_pCircularBuffer->push(pRTMSA->getMultiSampleArray()[i]))
            {
                //Do nothing until the circular buffer is ready to accept new data again
            }
        }
    }
}

//============================================================================================================= run

void TmsController::run()
{
    MatrixXd matData;

    // Wait for Fiff Info
    while(!m_pFiffInfo)
    {
        msleep(10);
    }

    // Prepare Rapid
    //double timeNextShotPossible = clock();
    int iError = 0;
    int currentStaticPower      = m_iStaticPower;
    int currentSuperRapid       = m_iSuperRapid;
    int currentVoltage          = m_iVoltage;
    QString currentUnlockCode   = m_sUnlockCode;
    std::map<QString, std::map<QString, double>> currentDeviceParameters;

    Rapid* pMyRapid = new Rapid(m_sPortName, currentSuperRapid, currentUnlockCode, currentVoltage, m_tBaseVersion);
    pMyRapid->connect(iError);
    currentDeviceParameters = pMyRapid->getParameters(iError);
    pMyRapid->setPower(currentStaticPower, false, currentDeviceParameters, iError);
    pMyRapid->arm(false, currentDeviceParameters, iError);
    pMyRapid->ignoreCoilSafetySwitch(iError);
    QThread::sleep(1);
    emit sig_updateDeviceStatus(true, "Device armed. Ready to Fire");

    while(!isInterruptionRequested())
    {
        // Get the current fiff data
        if(m_pCircularBuffer->pop(matData))
        {
            //ToDo: Implement your algorithm here

            //Send the data to the connected plugins and the online display
            //Unocmment this if you also uncommented the m_pOutput in the constructor above
            //if(!isInterruptionRequested()) {
            //m_pOutput->measurementData()->setValue(matData);
            //}
        }


        // check if still running
        if (!m_bIsRunning)
        {
            break;
        }

        // To triggers a shot
        if (m_bIsToFire)
        {
            iError = 0;
            m_bIsToFire = false;
            //pMyRapid->quickFire(iError);
            pMyRapid->fire(iError);
            pMyRapid->resetQuikeFire();
            //QThread::msleep(100);

            qDebug() << "Fire with return code: " << iError;

            /*
            // Fire only if not in DeadTime after the last shot
            if (timeNextShotPossible < clock())
            {
                timeNextShotPossible = clock() + m_iDeadTime*CLOCKS_PER_SEC;

                pMyRapid->quickFire(iError);
                pMyRapid->resetQuikeFire();
                QThread::msleep(50);
            }
            */
        }

        // check if need to update parameters (parameters changed from GUI)
        if (currentStaticPower != m_iStaticPower)
        {
            iError = 0;
            currentStaticPower = m_iStaticPower;    // update static power.
            pMyRapid->setPower(currentStaticPower, false, currentDeviceParameters, iError);
            QThread::msleep(50);
            qDebug() << "Power updated with return code: " << iError;
        }

        if (currentSuperRapid != m_iSuperRapid)
        {
            // update superRapid.
        }

        if (currentVoltage != m_iVoltage)
        {
            // update voltage.
        }

        if (currentUnlockCode != m_sUnlockCode)
        {
            // update unlock code.
        }
    } // while

    pMyRapid->disconnect(iError);
}

//=============================================================================================================

//=============================================================================================================

void TmsController::updatePortName(QString portName)
{
    m_mutex.lock();
    m_sPortName = portName;
    m_mutex.unlock();
}

//=============================================================================================================

void TmsController::updateDeviceName(QString deviceName)
{
    m_mutex.lock();
    m_sPortName = deviceName;
    m_mutex.unlock();
}

//=============================================================================================================

void TmsController::updateCheckStatus(bool bIsChecked, QString sCheckInfo, std::tuple<int, int, int> baseVersion)
{
    m_mutex.lock();
    m_bIsConnectable = bIsChecked;
    m_sCheckInfo = sCheckInfo;
    m_tBaseVersion  = baseVersion;
    m_mutex.unlock();
}

//=============================================================================================================

QString TmsController::currentPortName()
{
    return m_sPortName;
}

//=============================================================================================================

QString TmsController::currentDeviceName()
{
    return m_sDeviceName;
}

//=============================================================================================================

std::tuple<bool, QString, std::tuple<int, int, int>> TmsController::currentCheckStatus()
{
    return std::make_tuple(m_bIsConnectable, m_sCheckInfo, m_tBaseVersion);

}

//=============================================================================================================

//=============================================================================================================

void TmsController::onClickFire()
{
    m_mutex.lock();
    //m_bIsParametersChanged = true;
    m_bIsToFire = true;
    m_mutex.unlock();
}

//=============================================================================================================

void TmsController::onClickDefault()
{
    m_mutex.lock();
    m_iSuperRapid = 0;
    m_sUnlockCode = "";
    m_iVoltage = 240;
    m_mutex.unlock();
}

//=============================================================================================================

void TmsController::onChangeStaticPower(int newPower)
{
    m_mutex.lock();
    m_iStaticPower = newPower;
    m_mutex.unlock();
}

//=============================================================================================================

void TmsController::onChangeSuperRapid(int newSuperRapid)
{
    m_mutex.lock();
    m_iSuperRapid = newSuperRapid;
    m_mutex.unlock();
}

//=============================================================================================================

void TmsController::onChangeUnlockCode(QString newUnlockCode)
{
    m_mutex.lock();
    m_sUnlockCode = newUnlockCode;
    m_mutex.unlock();
}

//=============================================================================================================

void TmsController::onChangeVoltage(int newVoltage)
{
    m_mutex.lock();
    m_iVoltage = newVoltage;
    m_mutex.unlock();
}
