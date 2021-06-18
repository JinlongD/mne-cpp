//=====================================================================================================================
/**
 * @file     classifiers.cpp
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
 * @brief    Definition of the classifiers class.
 *
 */
//=====================================================================================================================

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================
#include "classifiers.h"
#include "matparser.h"

#include "FormFiles/classifierssetupwidget.h"
#include "FormFiles/classifierssettingsview.h"

#include <fiff/fiff.h>
#include <fiff/fiff_constants.h>

//=====================================================================================================================
// QT INCLUDES
//=====================================================================================================================
#include <QDebug>
#include <QSettings>

//=====================================================================================================================
// EIGEN INCLUDES
//=====================================================================================================================
#include <Eigen/Core>

//=====================================================================================================================
// USED NAMESPACES
//=====================================================================================================================
using namespace CLASSIFIERSPLUGIN;
using namespace SCSHAREDLIB;
using namespace SCMEASLIB;
using namespace UTILSLIB;
using namespace Eigen;

//=====================================================================================================================
// DEFINE MEMBER METHODS
//=====================================================================================================================
Classifiers::Classifiers()
    : m_pCircularBuffer(QSharedPointer<CircularBuffer_Matrix_double>(new CircularBuffer_Matrix_double(40)))
    , m_bChNumReset(false)
    , m_iNumPickedCh(1)
    , m_sPickedChNames({"0"})
    , m_pMatParser(new MatParser(this))
{
}

//=====================================================================================================================
Classifiers::~Classifiers()
{
    if(this->isRunning()) {
        stop();
    }
}

//=====================================================================================================================
QSharedPointer<AbstractPlugin> Classifiers::clone() const
{
    QSharedPointer<Classifiers> pClone(new Classifiers);
    return pClone;
}

//=====================================================================================================================
void Classifiers::init()
{
    // Input
    m_pInput = PluginInputData<RealTimeMultiSampleArray>::create(this, "ClassifiersIn", "Classifiers input data");
    connect(m_pInput.data(), &PluginInputConnector::notify,
            this, &Classifiers::update, Qt::DirectConnection);
    m_inputConnectors.append(m_pInput);

    // Output - Uncomment this if you don't want to send processed data (in form of a matrix) to other plugins.
    // Also, this output stream will generate an online display in your plugin
    m_pOutput = PluginOutputData<RealTimeMultiSampleArray>::create(this, "ClassifiersOut", "Classifiers output data");
    m_pOutput->measurementData()->setName(this->getName());
    m_outputConnectors.append(m_pOutput);
}

//=====================================================================================================================
void Classifiers::unload()
{
}

//=====================================================================================================================
bool Classifiers::start()
{
    //Start thread
    QThread::start();

    return true;
}

//=====================================================================================================================
bool Classifiers::stop()
{
    requestInterruption();
    wait(500);

    // Clear all data in the buffer connected to displays and other plugins
    m_pOutput->measurementData()->clear();
    m_pCircularBuffer->clear();

    m_bPluginControlWidgetsInit = false;

    return true;
}

//=====================================================================================================================
AbstractPlugin::PluginType Classifiers::getType() const
{
    return _IAlgorithm;
}

//=====================================================================================================================
QString Classifiers::getName() const
{
    return "Classifiers";
}

//=====================================================================================================================
QWidget* Classifiers::setupWidget()
{
    ClassifiersSetupWidget* pSetupWidget = new ClassifiersSetupWidget(this);
    return pSetupWidget;
}

//=====================================================================================================================
void Classifiers::update(SCMEASLIB::Measurement::SPtr pMeasurement)
{
    if(QSharedPointer<RealTimeMultiSampleArray> pRTMSA = pMeasurement.dynamicCast<RealTimeMultiSampleArray>()) {
        //Fiff information
        if(!m_pFiffInfo) {
            m_pFiffInfo = FIFFLIB::FiffInfo::SPtr::create();
            //m_pFiffInfo = pRTMSA->info();

            QStringList includeChs = QList<QString>() << ""; //"STI 014";
            QStringList excludeChs = {""};
            bool want_meg = false;
            bool want_eeg = true;
            bool want_stim = false;
            m_vecEEGChPicks = pRTMSA->info()->pick_types(want_meg, want_eeg, want_stim, includeChs, excludeChs);
            FIFFLIB::FiffInfo pickedInfo =pRTMSA->info()->pick_info(m_vecEEGChPicks);

            m_iDataBufferSize = pRTMSA->getMultiSampleArray().first().cols();
            m_sEEGChNames = pickedInfo.ch_names;
            m_dDataSampFreq = pickedInfo.sfreq;

            QList<FIFFLIB::FiffChInfo> fiffChInfoList;
            FIFFLIB::FiffChInfo fiffChInfo;
            QStringList chNameList;
            fiffChInfo.kind     = 2; //pickedInfo.chs.at(0).kind;    // 502, misc: miscellaneous analog channels.
            //fiffChInfo.range    = 0.0005; //pickedInfo.chs.at(0).range;   // -1
            fiffChInfo.unit     = 107; //pickedInfo.chs.at(0).unit;    // -1

            for (int i = 0; i < m_iNumPickedCh; ++i) {
                //QString tempStr = QString("BP%1").arg(i);
                fiffChInfo.ch_name = m_sEEGChNames.at(m_sPickedChNames.at(i).toInt()); // + QString("-BP");
                chNameList.append(fiffChInfo.ch_name);
                fiffChInfoList.append(fiffChInfo);
            }
            m_pFiffInfo->filename = "";
            m_pFiffInfo->bads.clear();
            m_pFiffInfo->nchan = m_iNumPickedCh;
            m_pFiffInfo->ch_names = chNameList;
            m_pFiffInfo->chs = fiffChInfoList;
            m_pFiffInfo->sfreq = m_dDataSampFreq;

            m_pOutput->measurementData()->initFromFiffInfo(m_pFiffInfo);
            m_pOutput->measurementData()->setMultiArraySize(1);
        }

        if(!m_bPluginControlWidgetsInit) {
            initPluginControlWidgets();
        }

        for(unsigned char i = 0; i < pRTMSA->getMultiArraySize(); ++i) {
            // Please note that we do not need a copy here since this function will block until
            // the buffer accepts new data again. Hence, the data is not deleted in the actual
            // Mesaurement function after it emitted the notify signal.
            while(!m_pCircularBuffer->push(pRTMSA->getMultiSampleArray()[i])) {
                //Do nothing until the circular buffer is ready to accept new data again
            }
        }
    }
}

//=====================================================================================================================
void Classifiers::initPluginControlWidgets()
{
    if(m_pFiffInfo) {
        QList<QWidget*> plControlWidgets;

        // The plugin's control widget
        ClassifiersSettingsView* pClassifiersSettingsView = new ClassifiersSettingsView(QString("MNESCAN/%1").arg(this->getName()));
        pClassifiersSettingsView->setObjectName("group_tab_Settings_Your Widget");

        plControlWidgets.append(pClassifiersSettingsView);

        emit pluginControlWidgetsChanged(plControlWidgets, this->getName());

        m_bPluginControlWidgetsInit = true;
    }
}

//=====================================================================================================================
void Classifiers::run()
{
    MatrixXd matDataInput;
    MatrixXd matDataOutput; //(m_iNumPickedCh, m_iDataBufferSize);

    // Wait for Fiff Info
    while(!m_pFiffInfo) {
        msleep(10);
    }

    //=================================================================================================================
    while(!isInterruptionRequested()) {
        //=============================================================================================================
        // Get the current data
        if(m_pCircularBuffer->pop(matDataInput)) {
            //ToDo: Implement your algorithm here

            m_qMutex.lock();
            MatrixXd matTemp(m_iNumPickedCh, m_iDataBufferSize);
            for (int i = 0; i < m_iNumPickedCh; ++i) {
                matTemp.row(i) = matDataInput.row(m_vecEEGChPicks(m_sPickedChNames.at(i).toInt()));
            }

            matDataOutput = matTemp;

            m_qMutex.unlock();

            //Send the data to the connected plugins and the online display
            //Unocmment this if you also uncommented the m_pOutput in the constructor above
            if(!isInterruptionRequested()) {
                m_pOutput->measurementData()->setValue(matDataOutput);
            }
        }

        //=============================================================================================================
        if (m_bChNumReset) {
            // reset picked channel number
            //m_qMutex.lock();

            QList<FIFFLIB::FiffChInfo> fiffChInfoList;
            FIFFLIB::FiffChInfo fiffChInfo;
            QStringList chNameList;
            fiffChInfo.kind     = 2; //pickedInfo.chs.at(0).kind;    // 502, misc: miscellaneous analog channels.
            //fiffChInfo.range    = 0.0005; //pickedInfo.chs.at(0).range;   // -1
            fiffChInfo.unit     = 107; //pickedInfo.chs.at(0).unit;    // -1

            m_iNumPickedCh = m_sPickedChNames.size();

            for (int i = 0; i < m_iNumPickedCh; ++i) {
                //QString tempStr = QString("BP%1").arg(i);
                fiffChInfo.ch_name = m_sEEGChNames.at(m_sPickedChNames.at(i).toInt()); // + QString("-BP");
                chNameList.append(fiffChInfo.ch_name);
                fiffChInfoList.append(fiffChInfo);
            }

            m_pFiffInfo->nchan = m_iNumPickedCh;
            m_pFiffInfo->ch_names = chNameList;
            m_pFiffInfo->chs = fiffChInfoList;

            m_pOutput->measurementData()->initFromFiffInfo(m_pFiffInfo);
            m_pOutput->measurementData()->setMultiArraySize(1);
            m_pOutput->measurementData()->resetChannelNum(true); // true

            m_bChNumReset = false;
            //m_qMutex.unlock();
        }
        //=============================================================================================================
    }
    //=================================================================================================================
}

//=====================================================================================================================
MatParser* Classifiers::getMatParser()
{
    return m_pMatParser;
}
