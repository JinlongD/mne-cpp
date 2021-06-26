//=====================================================================================================================
/**
 * @file     bandpower.cpp
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
 * @brief    Definition of the bandpower class.
 *
 */
//=====================================================================================================================

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================
#include "bandpower.h"
#include "bpspectral.h"

#include "FormFiles/bandpowersetupwidget.h"
#include "FormFiles/bandpowersettingsview.h"
#include "FormFiles/bandpowerarsettingsview.h"

#include <fiff/fiff.h>
#include <fiff/fiff_constants.h>
//#include <disp/viewers/channelselectionview.h>
//#include <disp/viewers/helpers/channelinfomodel.h>

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
using namespace BANDPOWERPLUGIN;
using namespace SCSHAREDLIB;
using namespace SCMEASLIB;
using namespace UTILSLIB;
using namespace Eigen;

//=====================================================================================================================
// DEFINE MEMBER METHODS
//=====================================================================================================================
Bandpower::Bandpower()
    : m_pBandpowerBuffer(QSharedPointer<CircularBuffer_Matrix_double>(new CircularBuffer_Matrix_double(40)))
    , m_bPluginControlWidgetsInit(false)
    , m_dMaxFreq(30.0)
    , m_dMinFreq(8.0)
    , m_iSegmentLength(400)
    , m_iSegmentStep(50)
    , m_iNumBins(11)
    , m_iNumPickedChannels(1)
    , m_sDetrendMethod("None")
    , m_sSpectrumMethod("AR")
    , m_iAROrder(16)
    , m_iAREvaluationPoints(1)
    , m_bBandpowerChNumReset(false)
    , m_bBandpowerSegmentReset(false)
    , m_sPickedChNames({"0"})
{
    m_sSettingsPath = QString("MNESCAN/%1").arg(this->getName());
}

//=====================================================================================================================
Bandpower::~Bandpower()
{
    if(this->isRunning()) {
        qDebug() << "[Bandpower::~Bandpower] if this thread is running.";
        stop();
    }
}

//=====================================================================================================================
QSharedPointer<AbstractPlugin> Bandpower::clone() const
{
    QSharedPointer<Bandpower> pClone(new Bandpower);
    return pClone;
}

//=====================================================================================================================
void Bandpower::init()
{
    // Input
    m_pBandpowerInput = PluginInputData<RealTimeMultiSampleArray>::create(this, "BandpowerIn", "Bandpower input data");
    connect(m_pBandpowerInput.data(), &PluginInputConnector::notify,
            this, &Bandpower::update, Qt::DirectConnection);
    m_inputConnectors.append(m_pBandpowerInput);


    // Output - Uncomment this if you don't want to send processed data (in form of a matrix) to other plugins.
    // Also, this output stream will generate an online display in your plugin
    m_pBandpowerOutput = PluginOutputData<RealTimeMultiSampleArray>::create(this, "BandpowerOut", "Bandpower output data");
    m_pBandpowerOutput->measurementData()->setName(this->getName());
    connect(this, &Bandpower::sig_resetRTMSAChNum,
            m_pBandpowerOutput->measurementData().data(), &RealTimeMultiSampleArray::resetChannelNum, Qt::QueuedConnection);
    m_outputConnectors.append(m_pBandpowerOutput);

    /*// Initialization of the settings parameters on the GUIs.
    m_dMinFreq              = 8.0;
    m_dMaxFreq              = 30.0;
    m_iSegmentLength        = 400;
    m_iSegmentStep          = 50;
    m_iNumBins              = 11;
    m_iNumPickedChannels    = 1;
    m_sSpectrumMethod       = "AR";
    m_sDetrendMethod        = "None";
    m_iAROrder              = 16;
    m_iAREvaluationPoints   = 1;
    m_sPickedChNames.append(QString::number(0));
    m_bBandpowerChNumReset          = false;
    m_bBandpowerSegmentReset        = false;*/

    m_sSettingsPath = QString("MNESCAN/%1").arg(this->getName());
    QSettings settings("MNECPP");
    //settings.clear();
    settings.setValue(m_sSettingsPath + QString("/minFreq"), m_dMinFreq);
    settings.setValue(m_sSettingsPath + QString("/maxFreq"), m_dMaxFreq);
    settings.setValue(m_sSettingsPath + QString("/numBins"), m_iNumBins);
    settings.setValue(m_sSettingsPath + QString("/segmentLength"), static_cast<double>(m_iSegmentLength));
    settings.setValue(m_sSettingsPath + QString("/segmentStep"), static_cast<double>(m_iSegmentStep));
    //settings.setValue(m_sSettingsPath + QString("/numPickedChannels"), m_iNumPickedChannels);
    settings.setValue(m_sSettingsPath + QString("/spectrumMethod"), m_sSpectrumMethod);
    settings.setValue(m_sSettingsPath + QString("/detrendMethod"), m_sDetrendMethod);
    settings.setValue(m_sSettingsPath + QString("/arOrder"), m_iAROrder);
    settings.setValue(m_sSettingsPath + QString("/evaluationsPoints"), m_iAREvaluationPoints);
}

//=====================================================================================================================
void Bandpower::unload()
{
}

//=====================================================================================================================
bool Bandpower::start()
{
    //Start thread
    QThread::start();

    return true;
}

//=====================================================================================================================
bool Bandpower::stop()
{
    requestInterruption();
    //this->quit();
    wait(500);

    // Clear all data in the buffer connected to displays and other plugins
    m_pBandpowerOutput->measurementData()->clear();
    m_pBandpowerBuffer->clear();

    m_bPluginControlWidgetsInit = false;
    return true;
}

//=====================================================================================================================
AbstractPlugin::PluginType Bandpower::getType() const
{
    return _IAlgorithm;
}

//=====================================================================================================================
QString Bandpower::getName() const
{
    return "Bandpower";
}

//=====================================================================================================================
QWidget* Bandpower::setupWidget()
{
    qDebug() << "[Bandpower::setupWidget]";
    BandpowerSetupWidget* pSetupWidget = new BandpowerSetupWidget(this);
    return pSetupWidget;
}

//=====================================================================================================================
void Bandpower::update(SCMEASLIB::Measurement::SPtr pMeasurement)
{
    if(QSharedPointer<RealTimeMultiSampleArray> pRTMSA = pMeasurement.dynamicCast<RealTimeMultiSampleArray>()) {
        if(!m_pFiffInfoOutput) {
            m_pFiffInfoInput = pRTMSA->info(); // get the input Fiff information.

            // selecte the fiffinfo of all EEG channels out of the input fiffinfo.
            QStringList includeChs;
            includeChs << ""; //"STI 014";
            QStringList excludeChs;
            excludeChs << ""; //m_pFiffInfoInput->bads;
            bool want_meg = false;
            bool want_eeg = true;
            bool want_stim = false;
            m_vecEEGChPicks = m_pFiffInfoInput->pick_types(want_meg, want_eeg, want_stim, includeChs, excludeChs);

            // get the parameters of the original EEG input data.
            FIFFLIB::FiffInfo pickedInfo = m_pFiffInfoInput->pick_info(m_vecEEGChPicks);
            //m_iDataNumChannels = pRTMSA->getMultiSampleArray().first().rows(); // m_pFiffInfoInput->nchan
            m_iDataBufferSize   = pRTMSA->getMultiSampleArray().first().cols();
            m_dDataSampFreq     = pickedInfo.sfreq;
            m_iDataNumEEGChs    = pickedInfo.nchan;
            m_sDataEEGChNames   = pickedInfo.ch_names;
            m_iSegmentLength    = static_cast<int>(0.4 * m_dDataSampFreq); // init: 400 ms.
            m_iSegmentStep      = static_cast<int>(0.05 * m_dDataSampFreq); // init: 50 ms.

            m_iNumPickedChannels = m_sPickedChNames.size();
            m_iBandpowerNumChannels = m_iNumPickedChannels*m_iNumBins;

            QList<FIFFLIB::FiffChInfo> fiffChInfoList;
            FIFFLIB::FiffChInfo fiffChInfo;
            QStringList chNameList;
            fiffChInfo.kind     = 502; //pickedInfo.chs.at(0).kind;    // 2: eeg; 502: misc (miscellaneous analog channels).
            //fiffChInfo.range    = -1; //pickedInfo.chs.at(0).range;   // -1
            fiffChInfo.unit     = 0; //pickedInfo.chs.at(0).unit;    // 107: uV, -1: no unit; 0: unitless

            for (int i = 0; i < m_iNumPickedChannels; ++i) {
                for (int j = 0; j < m_iNumBins; ++j) {
                    fiffChInfo.ch_name = m_sDataEEGChNames.at(m_sPickedChNames.at(i).toInt()) + QString("-BP%1").arg(j);
                    chNameList.append(fiffChInfo.ch_name);
                    fiffChInfoList.append(fiffChInfo);
                }
            }

            //m_pFiffInfoOutput = FIFFLIB::FiffInfo::SPtr(new FIFFLIB::FiffInfo(*pRTMSA->info().data()));
            m_pFiffInfoOutput = FIFFLIB::FiffInfo::SPtr::create();  // Init the output Fiff information.

            m_pFiffInfoOutput->filename = "";
            m_pFiffInfoOutput->bads.clear();
            m_pFiffInfoOutput->nchan = m_iBandpowerNumChannels;
            m_pFiffInfoOutput->ch_names = chNameList;
            m_pFiffInfoOutput->chs = fiffChInfoList;
            m_pFiffInfoOutput->sfreq = m_dDataSampFreq / m_iSegmentStep; // 0

            m_pBandpowerOutput->measurementData()->initFromFiffInfo(m_pFiffInfoOutput);
            m_pBandpowerOutput->measurementData()->setMultiArraySize(1);
        }

        //=============================================================================================================
        // Init the plugins' quick control widgets if not yet.
        if(!m_bPluginControlWidgetsInit) {
            initPluginControlWidgets();
        }

        //=============================================================================================================
        // Append the input data matrix into the bandpower circular buffer.
        for(unsigned char i = 0; i < pRTMSA->getMultiArraySize(); ++i) {
            // Please note that we do not need a copy here since this function will block until
            // the buffer accepts new data again. Hence, the data is not deleted in the actual
            // Mesaurement function after it emitted the notify signal.
            while(!m_pBandpowerBuffer->push(pRTMSA->getMultiSampleArray()[i])) {
                //Do nothing until the circular buffer is ready to accept new data again
            }
        }
    }
}

//=====================================================================================================================
void Bandpower::initPluginControlWidgets()
{
    if(m_pFiffInfoOutput) {
        QList<QWidget*> plControlWidgets;

        // The plugin's control widgets
        // BandpowerSettingsView
        BandpowerSettingsView* pBandpowerSettingsView = new BandpowerSettingsView(m_sSettingsPath, m_sDataEEGChNames, m_sPickedChNames, m_dDataSampFreq);
        //m_pBandpowerSettingsView = new BandpowerSettingsView(m_sSettingsPath, m_sDataEEGChNames, m_sPickedChNames, m_dDataSampFreq);
        pBandpowerSettingsView->setObjectName("group_tab_Settings_Bandpower Settings");
        connect(pBandpowerSettingsView, &BandpowerSettingsView::sig_updateMinFreq,
                this, &Bandpower::onUpdateBandpowerMinFreq);
        connect(pBandpowerSettingsView, &BandpowerSettingsView::sig_updateMaxFreq,
                this, &Bandpower::onUpdateBandpowerMaxFreq);
        connect(pBandpowerSettingsView, &BandpowerSettingsView::sig_updatePickedChannels,
                this, &Bandpower::onUpdateBandpowerPickedChannels);
        connect(pBandpowerSettingsView, &BandpowerSettingsView::sig_updateNumBins,
                this, &Bandpower::onUpdateBandpowerNumBins);
        connect(pBandpowerSettingsView, &BandpowerSettingsView::sig_updateSegmentLength,
                this, &Bandpower::onUpdateBandpowerSegmentLength);
        connect(pBandpowerSettingsView, &BandpowerSettingsView::sig_updateSegmentStep,
                this, &Bandpower::onUpdateBandpowerSegmentStep);
        connect(pBandpowerSettingsView, &BandpowerSettingsView::sig_updateSpectrumMethod,
                this, &Bandpower::onUpdateBandpowerSpectrumMethod);
        connect(pBandpowerSettingsView, &BandpowerSettingsView::sig_updateDetrendMethod,
                this, &Bandpower::onUpdateBandpowerDetrendMethod);
        plControlWidgets.append(pBandpowerSettingsView);

        // ARSettingsView
        BandpowerARSettingsView* pBandpowerARSettingsView = new BandpowerARSettingsView(m_sSettingsPath);
        //m_pBandpowerARSettingsView = new BandpowerARSettingsView(m_sSettingsPath);
        pBandpowerARSettingsView->setObjectName("group_tab_Settings_AR Settings");
        connect(pBandpowerARSettingsView, &BandpowerARSettingsView::sig_updateAROrder,
                this, &Bandpower::onUpdateBandpowerAROrder);
        connect(pBandpowerARSettingsView, &BandpowerARSettingsView::sig_updateAREvaluationPoints,
                this, &Bandpower::onUpdateBandpowerAREvaluationPoints);
        plControlWidgets.append(pBandpowerARSettingsView);

        // emit plugin's control widgets
        emit pluginControlWidgetsChanged(plControlWidgets, this->getName());

        m_bPluginControlWidgetsInit = true;
    }

    /*// channel selection view
        m_pChannelInfoModel = DISPLIB::ChannelInfoModel::SPtr::create(m_pFiffInfoInput, this);
        m_pChannelSelectionView = DISPLIB::ChannelSelectionView::SPtr::create(m_sSettingsPath, this,
                                                                              m_pChannelInfoModel, Qt::Window);
        m_pChannelSelectionView->setWindowTitle(tr(QString("%1: Channel Selection Window").arg(this->getName()).toUtf8()));
        connect(m_pChannelSelectionView.data(), &DISPLIB::ChannelSelectionView::loadedLayoutMap,
                m_pChannelInfoModel.data(), &DISPLIB::ChannelInfoModel::layoutChanged);
        connect(m_pChannelInfoModel.data(), &DISPLIB::ChannelInfoModel::channelsMappedToLayout,
                m_pChannelSelectionView.data(), &DISPLIB::ChannelSelectionView::setCurrentlyMappedFiffChannels);
        m_pChannelInfoModel->layoutChanged(m_pChannelSelectionView->getLayoutMap());

        connect(m_pChannelSelectionView.data(), &DISPLIB::ChannelSelectionView::showSelectedChannelsOnly,
                m_pChannelDataView.data(), &DISPLIB::RtFiffRawView::showSelectedChannelsOnly);
        connect(m_pChannelDataView.data(), &DISPLIB::RtFiffRawView::channelMarkingChanged,
                m_pChannelSelectionView.data(), &DISPLIB::ChannelSelectionView::updateBadChannels);
        connect(m_pChannelDataView.data(), &DISPLIB::RtFiffRawView::selectedChannelsChanged,
                m_pChannelSelectionView.data(), &DISPLIB::ChannelSelectionView::setUserSelection);
        connect(m_pChannelDataView.data(), &DISPLIB::RtFiffRawView::selectedChannelsResetted,
                m_pChannelSelectionView.data(), &DISPLIB::ChannelSelectionView::resetUserSelection);*/
}

//=====================================================================================================================
void Bandpower::run()
{
    // Wait for Fiff Info
    while(!m_pFiffInfoOutput) {
        msleep(10);
    }

    //=================================================================================================================
    // init segment data for the first data block.
    //MatrixXd    matSegment(m_iNumPickedChannels, m_iSegmentLength); // segment data matrix to be processed.
    MatrixXd    matSegment;
    MatrixXd    matTemp;
    MatrixXd    matDataInput;

    // detremine spectrum resolution - calculation only for FFT, but we keep it for both methods here to keep the resolution.
    VectorXd    vecFFTResFreqs = Spectral::calculateFFTFreqs(m_iSegmentLength, m_dDataSampFreq);

    // prepare storage for the first data segment.
    MatrixXd matTempSeg(m_iNumPickedChannels, (m_iSegmentLength/m_iDataBufferSize + 1)*m_iDataBufferSize); // temp considering input buffersize.
    for (int i = 0; i <= m_iSegmentLength/m_iDataBufferSize; ++i) {
        while (!m_pBandpowerBuffer->pop(matDataInput));
        for (int j = 0; j < m_iNumPickedChannels; ++j) {
            matDataInput *= 1.0e6;
            matTempSeg.block(j, i*m_iDataBufferSize, 1, m_iDataBufferSize) = matDataInput.row(m_vecEEGChPicks(m_sPickedChNames.at(j).toInt()));
        }
    }
    matSegment = matTempSeg.leftCols(m_iSegmentLength);
    matTemp = matTempSeg.rightCols(matTempSeg.cols() - m_iSegmentLength);

    while(!isInterruptionRequested()) {
        // compute bandpower features.

        //QMutexLocker locker(&m_qMutex);
        m_qMutex.lock();

        // detrend
        MatrixXd matSegmentDetrend = BPSpectral::detrendData(matSegment, m_sDetrendMethod);

        // calculate bandpowers
        QVector<VectorXd> matSpectrum;
        MatrixXd matBandpower(m_iBandpowerNumChannels, 1);

        if (m_sSpectrumMethod == "AR") {
            MatrixXcd ARSpectraWeights = BPSpectral::generateARSpectraWeights(m_dMinFreq/m_dDataSampFreq, m_dMaxFreq/m_dDataSampFreq, m_iNumBins, m_iAREvaluationPoints, true);
            QVector<QPair<VectorXd, double>> ARCoeffs = BPSpectral::calculateARWeightsMEMMatrix(matSegmentDetrend, m_iAROrder, true);
            matSpectrum = BPSpectral::psdFromARSpectra(ARCoeffs, ARSpectraWeights, m_dDataSampFreq, true);

            for (int i = 0; i < std::min(m_iNumPickedChannels, matSpectrum.length()); ++i) {
                matBandpower.block(i*m_iNumBins, 0, m_iNumBins, 1) = matSpectrum.at(i);
            }
            matSpectrum.clear();
        } else if (m_sSpectrumMethod == "FFT") {
            // generate hanning window
            QPair<MatrixXd, VectorXd> tapers = Spectral::generateTapers(m_iSegmentLength, "hanning");
            MatrixXd matTapers = tapers.first;
            VectorXd vecTapersWeights = tapers.second;

            // compute spectrum
            QVector<MatrixXcd> matTaperedSpectrum;
            matTaperedSpectrum = Spectral::computeTaperedSpectraMatrix(matSegmentDetrend, matTapers, m_iSegmentLength, true);
            matSpectrum = BPSpectral::psdFromTaperedSpectra_BP(matTaperedSpectrum, vecTapersWeights, m_iSegmentLength, m_dDataSampFreq, false);

            // select frequencies that fall within the band
            m_dBinWidth = (m_dMaxFreq - m_dMinFreq) / static_cast<double>(m_iNumBins);
            if (m_dBinWidth < vecFFTResFreqs[1] - vecFFTResFreqs[0]) {
                qDebug() << "[BandPower::run] Selected bin width is smaller than FFT resolution";
            }

            for (int i = 0; i < std::min(m_iNumPickedChannels, matSpectrum.length()); ++i) {
                for (int j = 0; j < m_iNumPickedChannels; ++j) {
                    matBandpower(i*m_iNumBins + j, 1) = BPSpectral::bandpowerFromSpectrumEntriesOffset(vecFFTResFreqs, matSpectrum.at(i), m_dMinFreq + j*m_dBinWidth, m_dMinFreq + (j+1)*m_dBinWidth);
                }
            }
            matSpectrum.clear();
        }

        m_qMutex.unlock();

        if(!isInterruptionRequested()) {
            // send the data to the connected plugins and the online display
            // unocmment this if you also uncommented the m_pOutput in the constructor above
            m_pBandpowerOutput->measurementData()->setValue(matBandpower);
        }

        //if(!isInterruptionRequested()) {
        // move forward the segment matrix entries (i.e., matSegment and matTemp) by one iSegmentStep.
        if (!m_bBandpowerChNumReset && !m_bBandpowerSegmentReset) {
            //m_qMutex.lock();
            if (matTemp.cols() >= m_iSegmentStep) {
                MatrixXd tempSeg(m_iNumPickedChannels, m_iSegmentLength);
                tempSeg << matSegment.rightCols(m_iSegmentLength - m_iSegmentStep), matTemp.leftCols(m_iSegmentStep);
                matSegment = tempSeg;
                matTemp = matTemp.rightCols(matTemp.cols() - m_iSegmentStep);
            } else {
                while (matTemp.cols() < m_iSegmentStep) {
                    MatrixXd temp(m_iNumPickedChannels, matTemp.cols() + m_iDataBufferSize);
                    MatrixXd tempDataInput(m_iNumPickedChannels, m_iDataBufferSize);
                    while(!m_pBandpowerBuffer->pop(matDataInput));
                    for (int i = 0; i < m_iNumPickedChannels; ++i) {
                        matDataInput *= 1.0e6;
                        tempDataInput.row(i) = matDataInput.row(m_vecEEGChPicks(m_sPickedChNames.at(i).toInt()));
                    }
                    temp << matTemp, tempDataInput;
                    matTemp = temp;
                }
                MatrixXd tempSeg(m_iNumPickedChannels, m_iSegmentLength);
                tempSeg << matSegment.rightCols(m_iSegmentLength - m_iSegmentStep), matTemp.leftCols(m_iSegmentStep);
                matSegment = tempSeg;
                matTemp = matTemp.rightCols(matTemp.cols() - m_iSegmentStep);
            }
            //m_qMutex.unlock();
        } else {
            // reset bandpower channels when picked channels (channel number or/and channel names) and/or number of bins is changed.
            if (m_bBandpowerChNumReset) {
                //m_qMutex.lock();
                // Rearrange the output fiffinfo here.
                m_iNumPickedChannels = m_sPickedChNames.size();
                m_iBandpowerNumChannels = m_iNumPickedChannels*m_iNumBins;

                QList<FIFFLIB::FiffChInfo> fiffChInfoList;
                FIFFLIB::FiffChInfo fiffChInfo;
                QStringList chNameList;
                fiffChInfo.kind     = 502; //pickedInfo.chs.at(0).kind;    // 502, misc: miscellaneous analog channels.
                //fiffChInfo.range    = -1; //pickedInfo.chs.at(0).range;   // -1
                fiffChInfo.unit     = 0; //pickedInfo.chs.at(0).unit;    // -1
                for (int i = 0; i < m_iNumPickedChannels; ++i) {
                    for (int j = 0; j < m_iNumBins; ++j) {
                        fiffChInfo.ch_name = m_sDataEEGChNames.at(m_sPickedChNames.at(i).toInt()) + QString("-BP%1").arg(j);
                        chNameList.append(fiffChInfo.ch_name);
                        fiffChInfoList.append(fiffChInfo);
                    }
                }

                //m_pFiffInfoOutput->filename = "Bandpower";    // not necessary to change.
                //m_pFiffInfoOutput->bads.clear();              // not necessary to change.
                m_pFiffInfoOutput->nchan = m_iBandpowerNumChannels;
                m_pFiffInfoOutput->ch_names = chNameList;
                m_pFiffInfoOutput->chs = fiffChInfoList;

                m_pBandpowerOutput->measurementData()->resetChannelNum(true);
                m_pBandpowerOutput->measurementData()->initFromFiffInfo(m_pFiffInfoOutput);
                m_pBandpowerOutput->measurementData()->setMultiArraySize(1);

                m_bBandpowerChNumReset = false;
                //m_qMutex.unlock();
            }

            // reset data segment for the first data block when segment length and/or step is changed.
            if (m_bBandpowerSegmentReset) {
                //m_qMutex.lock();

                // detremine spectrum resolution - calculation only for FFT, but we keep it for both methods here to keep the resolution.
                vecFFTResFreqs = Spectral::calculateFFTFreqs(m_iSegmentLength, m_dDataSampFreq);

                // prepare storage for the first data segment.
                MatrixXd    matTempSeg(m_iNumPickedChannels, (m_iSegmentLength/m_iDataBufferSize + 1)*m_iDataBufferSize);
                for (int i = 0; i <= m_iSegmentLength/m_iDataBufferSize; ++i) {
                    while (!m_pBandpowerBuffer->pop(matDataInput));
                    matDataInput *= 1.0e6;
                    for (int j = 0; j < m_iNumPickedChannels; ++j) {
                        matTempSeg.block(j, i*m_iDataBufferSize, 1, m_iDataBufferSize) = matDataInput.row(m_vecEEGChPicks(m_sPickedChNames.at(j).toInt()));
                    }
                }
                matSegment = matTempSeg.leftCols(m_iSegmentLength);
                matTemp = matTempSeg.rightCols(matTempSeg.cols() - m_iSegmentLength);

                m_bBandpowerSegmentReset = false;
                //m_qMutex.unlock();
            }
        }
        //} //if(!isInterruptionRequested())
    } // while (!isInterruptionRequested())
}

//=====================================================================================================================
void Bandpower::onUpdateBandpowerMinFreq(double dMinFreq)
{
    QMutexLocker locker(&m_qMutex);

    m_dMinFreq = dMinFreq;
}

//=====================================================================================================================
void Bandpower::onUpdateBandpowerMaxFreq(double dMaxFreq)
{
    QMutexLocker locker(&m_qMutex);

    m_dMaxFreq = dMaxFreq;
}

//=====================================================================================================================
void Bandpower::onUpdateBandpowerPickedChannels(QStringList sPickedChIndex)
{
    QMutexLocker locker(&m_qMutex);

    m_sPickedChNames = sPickedChIndex;
    m_iNumPickedChannels = sPickedChIndex.size();

    m_bBandpowerChNumReset = true; // changed picked channels (names and/or number), bandpower channels (rows of RTMSA) should be reset.
    m_bBandpowerSegmentReset = true; // segment data (columns of RTMSA) should also be reset according picked EEG channels.
}

//=====================================================================================================================
void Bandpower::onUpdateBandpowerNumBins(int iNumBins)
{
    QMutexLocker locker(&m_qMutex);

    m_iNumBins = iNumBins;
    m_bBandpowerChNumReset = true; // only need to reset bandpower channels (rows of RTMSA), nothing to do with segment data.
}

//=====================================================================================================================
void Bandpower::onUpdateBandpowerSegmentLength(int iSegmentLength)
{
    QMutexLocker locker(&m_qMutex);

    m_iSegmentLength = iSegmentLength;
    m_bBandpowerSegmentReset = true; // only need to reset segment data (columns of RTMSA), nothing to do with bandpower channels.
}

//=====================================================================================================================
void Bandpower::onUpdateBandpowerSegmentStep(int iSegmentStep)
{
    QMutexLocker locker(&m_qMutex);

    m_iSegmentStep = iSegmentStep;
    //m_bBandpowerSegmentReset = true; // only need to reset segment data (columns of RTMSA), nothing to do with bandpower channels.
}

//=====================================================================================================================
void Bandpower::onUpdateBandpowerSpectrumMethod(const QString &sSpectrumMethod)
{
    QMutexLocker locker(&m_qMutex);

    m_sSpectrumMethod = sSpectrumMethod;
}

//=====================================================================================================================
void Bandpower::onUpdateBandpowerDetrendMethod(const QString &sDetrendMethod)
{
    QMutexLocker locker(&m_qMutex);

    m_sDetrendMethod = sDetrendMethod;
}

//=====================================================================================================================
void Bandpower::onUpdateBandpowerAROrder(int arOrder)
{
    QMutexLocker locker(&m_qMutex);

    m_iAROrder = arOrder;
}

//=====================================================================================================================
void Bandpower::onUpdateBandpowerAREvaluationPoints(int iAREvaluationPoints)
{
    QMutexLocker locker(&m_qMutex);

    m_iAREvaluationPoints = iAREvaluationPoints;
}
