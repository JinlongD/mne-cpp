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
#include <Eigen/SparseCore>

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
    , m_bPluginControlWidgetsInit(false)
    , m_iCurrentClassifier(0)
    , m_iTriggerThreshold(3)
    , m_iTriggerClass(1)
{
}

//=====================================================================================================================
Classifiers::~Classifiers()
{
    if (m_pParserThread->isRunning()) {
        m_pParserThread->quit();
        m_pParserThread->wait();
    }
    delete m_pParserThread;
    delete m_pMatParser;

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

    m_pParserThread = new QThread;
    m_pMatParser = new MatParser;
    m_pMatParser->moveToThread(m_pParserThread);
    connect(m_pMatParser, &MatParser::sig_isParsingFinished,
            this, &Classifiers::onIsParsingFinished);
    connect(this, &Classifiers::sig_getClassifiersFromMat,
            m_pMatParser, &MatParser::getClassifiers);
    //connect(m_pParserThread, &QThread::finished, m_pParserThread, &QThread::deleteLater);
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
            QList<FIFFLIB::FiffChInfo> fiffChInfoList;
            FIFFLIB::FiffChInfo fiffChInfo;
            QStringList chNameList;
            fiffChInfo.kind     = 2; // 2: eeg; 502: misc (miscellaneous analog channels).
            //fiffChInfo.range    = 0.000001; // 0.0005: eeg //pickedInfo.chs.at(0).range;
            fiffChInfo.unit     = 107; // 107: uV, -1: no unit; 0: unitless

            fiffChInfo.ch_name = "Trig";
            chNameList.append(fiffChInfo.ch_name);
            fiffChInfoList.append(fiffChInfo);

            //m_pFiffInfo = FIFFLIB::FiffInfo::SPtr(new FIFFLIB::FiffInfo(*pRTMSA->info().data()));
            m_pFiffInfo = FIFFLIB::FiffInfo::SPtr::create();
            //m_pFiffInfo = pRTMSA->info();

            m_pFiffInfo->filename = "";
            m_pFiffInfo->bads.clear();
            m_pFiffInfo->nchan = 1;
            m_pFiffInfo->ch_names = chNameList;
            m_pFiffInfo->chs = fiffChInfoList;
            m_pFiffInfo->sfreq = 20;

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
        ClassifiersSettingsView* pClassifiersSettingsView = new ClassifiersSettingsView(m_pMatParser->m_sClassifierNames,
                                                                                        m_pMatParser->m_sClassNames,
                                                                                        m_iTriggerThreshold,
                                                                                        QString("MNESCAN/%1").arg(this->getName()));
        pClassifiersSettingsView->setObjectName("group_tab_Settings_Classifiers");
        connect(pClassifiersSettingsView, &ClassifiersSettingsView::sig_updateClassifiers,
                this, &Classifiers::onClassifiersChanged);
        connect(pClassifiersSettingsView, &ClassifiersSettingsView::sig_updateTriggerThreshold,
                this, &Classifiers::onTriggerThresholdChanged);
        connect(pClassifiersSettingsView, &ClassifiersSettingsView::sig_updateTriggerClass,
                this, &Classifiers::onTriggerClassChanged);

        plControlWidgets.append(pClassifiersSettingsView);

        emit pluginControlWidgetsChanged(plControlWidgets, this->getName());

        m_bPluginControlWidgetsInit = true;
    }
}

//=====================================================================================================================
void Classifiers::run()
{
    Eigen::MatrixXd matDataInput;
    Eigen::MatrixXd matDataOutput(1, 4);
    //Eigen::Matrix<double, 1, 10> matDataOutput;

    Eigen::MatrixXd matWeight;
    Eigen::MatrixXd matMean;
    Eigen::VectorXd vecBias;

    /*if (!m_bIsClassifiersInit) {
        qWarning() << "[Classifiers::run] No classifiers loaded yet!";
        this->requestInterruption();
    }

    if (m_iCurrentClassifier == 0) {
        matWeight = m_pMatParser->m_classifierLDA.matWeight.transpose();
        vecBias = m_pMatParser->m_classifierLDA.vecBias;
    } else {
        matWeight = m_pMatParser->m_classifierFDA.matWeight.transpose();
        matMean = m_pMatParser->m_classifierFDA.vecMeanProj;
    }*/

    // Wait for Fiff Info
    while(!m_pFiffInfo) {
        msleep(10);
    }

    qint8 iTriggerCounter = 0;
    qDebug() << m_pMatParser->m_iFeatureNum << "=========";
    while(!isInterruptionRequested()) {
        // Get the input data
       if(m_pCircularBuffer->pop(matDataInput)) {
           //matDataOutput = matDataInput.row(1).leftCols(20);
           //matDataOutput.fill(1);
           matDataOutput.leftCols(2).fill(0);
           matDataOutput.rightCols(2).fill(1);
       }

        /*m_qMutex.lock();
        if (matDataInput.rows() == m_pMatParser->m_iFeatureNum) {
            if (!predictLDA(matDataInput, matWeight, vecBias)) {
                iTriggerCounter = 0;
            } else {
                iTriggerCounter += 1;
            }

            if (iTriggerCounter != m_iTriggerThreshold) {
                //matDataOutput << 0;
                matDataOutput.fill(0);
            } else {
                //matDataOutput << 1;
                matDataOutput.fill(1);
                iTriggerCounter -= 1;
            }
        } else {
            //matDataOutput << -1;
            matDataOutput.fill(-1);
            qWarning() << "[Classifiers::run] Feature dimension is not valid, please reset the Bandpower plugin.";
        }
        m_qMutex.unlock();*/


        //Send the data to the connected plugins and the online display
        //Unocmment this if you also uncommented the m_pOutput in the constructor above
        if(!isInterruptionRequested()) {
            m_pOutput->measurementData()->setValue(matDataOutput);
        }
    }
}

//=====================================================================================================================
void Classifiers::setMatByteArray(const QByteArray &byteArray)
{
    m_pMatParser->setMatFile(byteArray);
    m_bIsClassifiersInit = false;
}

//=====================================================================================================================
void Classifiers::parsingMat()
{
    m_pParserThread->start();
    emit sig_getClassifiersFromMat();
}

//=====================================================================================================================
void Classifiers::onIsParsingFinished()
{
    if (m_pParserThread->isRunning()) {
        m_pParserThread->quit();
        //m_pParserThread->wait(500);
    }
    m_sClassifiersInfo = "Classifiers:\n"
                    + tr(">>>>%1:\nClass Number = %2, Feature Number = %3, Cross-Validation Accuracy: %4%.\n")
                    .arg(m_pMatParser->m_sClassifierNames.at(0))
                    .arg(m_pMatParser->m_classifierLDA.iClassNum)
                    .arg(m_pMatParser->m_classifierLDA.iFeatureNum)
                    .arg(m_pMatParser->m_classifierLDA.dCVAccuracy)
                    + tr(">>>>%1:\nClass Number = %2, Feature Number = %3, Cross-Validation Accuracy: %4%.\n")
                    .arg(m_pMatParser->m_sClassifierNames.at(1))
                    .arg(m_pMatParser->m_classifierFDA.iClassNum)
                    .arg(m_pMatParser->m_classifierFDA.iFeatureNum)
                    .arg(m_pMatParser->m_classifierFDA.dCVAccuracy);
    emit sig_updateClassifiersInfo(m_sClassifiersInfo);

    m_bIsClassifiersInit = true;
}

//=====================================================================================================================
void Classifiers::onClassifiersChanged(int classifierIndex)
{
    m_qMutex.lock();
    m_iCurrentClassifier = classifierIndex;
    m_qMutex.unlock();
}

//=====================================================================================================================

void Classifiers::onTriggerThresholdChanged(int triggerThreshold)
{
    m_qMutex.lock();
    m_iTriggerThreshold = triggerThreshold;
    m_qMutex.unlock();
}

//=====================================================================================================================

void Classifiers::onTriggerClassChanged(int triggerClass)
{
    m_qMutex.lock();
    m_iTriggerClass = triggerClass;
    m_qMutex.unlock();
}
