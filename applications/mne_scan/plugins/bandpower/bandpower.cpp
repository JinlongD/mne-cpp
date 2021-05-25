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
#include <Eigen/Core>

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
{
}

//=====================================================================================================================
Bandpower::~Bandpower()
{
    if(this->isRunning()) {
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
    m_outputConnectors.append(m_pBandpowerOutput);

    // Setting parameters
    m_dMinFreq              = 8.0;
    m_dMaxFreq              = 30.0;
    m_dSegmentLength        = 400;
    m_dSegmentStep          = 50;
    m_iNumBins              = 11;
    m_iNumPickedChannels    = 1;
    m_sSpectrumMethod       = "AR";
    m_sDetrendMethod        = "None";
    m_iAROrder              = 16;
    m_iAREvaluationPoints   = 1;
    m_sPickedChNames.append(QString::number(0));
    m_bNumPickedChUpdated   = false;
    m_bNumBinsUpdated       = false;
    m_bSegmentInit          = false;
    m_bBandpowerOutputInit  = false;

    m_sSettingsPath = QString("MNESCAN/%1").arg(this->getName());
    QSettings settings("MNECPP");
    settings.clear();
    settings.setValue(m_sSettingsPath + QString("/minFreq"), m_dMinFreq);
    settings.setValue(m_sSettingsPath + QString("/maxFreq"), m_dMaxFreq);
    settings.setValue(m_sSettingsPath + QString("/segmentLength"), m_dSegmentLength);
    settings.setValue(m_sSettingsPath + QString("/segmentStep"), m_dSegmentStep);
    settings.setValue(m_sSettingsPath + QString("/numBins"), m_iNumBins);
    settings.setValue(m_sSettingsPath + QString("/numPickedChannels"), m_iNumPickedChannels);
    settings.setValue(m_sSettingsPath + QString("/spectrumMethod"), m_sSpectrumMethod);
    settings.setValue(m_sSettingsPath + QString("/detrendMethod"), m_sDetrendMethod);
    settings.setValue(m_sSettingsPath + QString("/arOrder"), m_iAROrder);
    settings.setValue(m_sSettingsPath + QString("/evaluationsPoints"), m_iAREvaluationPoints);
    //settings.setValue(m_sSettingsPath + QString("/pickedChNames"), {});
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
    BandpowerSetupWidget* pSetupWidget = new BandpowerSetupWidget(this);
    return pSetupWidget;
}

//=====================================================================================================================
void Bandpower::update(SCMEASLIB::Measurement::SPtr pMeasurement)
{
    if(QSharedPointer<RealTimeMultiSampleArray> pRTMSA = pMeasurement.dynamicCast<RealTimeMultiSampleArray>()) {

        //QMutexLocker locker(&m_qMutex);
        //Fiff information
        if(!m_pFiffInfoInput) {
            //=========================================================================================================
            m_pFiffInfoInput = pRTMSA->info();

            //m_pOutput->measurementData()->initFromFiffInfo(m_pFiffInfo);
            //m_pOutput->measurementData()->setMultiArraySize(1);

            //=========================================================================================================
            // selecte the fiffinfo of EEG channels.
            QStringList includeChs;
            includeChs << ""; //"STI 014";
            QStringList excludeChs;
            excludeChs << ""; //m_pFiffInfoInput->bads;
            bool want_meg = false;
            bool want_eeg = true;
            bool want_stim = false;
            m_vecEEGChPicks = m_pFiffInfoInput->pick_types(want_meg, want_eeg, want_stim, includeChs, excludeChs);

            //=========================================================================================================
            // get the parameters of the original EEG input data.
            FIFFLIB::FiffInfo pickedInfo = m_pFiffInfoInput->pick_info(m_vecEEGChPicks);
            //m_iDataNumChannels = pRTMSA->getMultiSampleArray().first().rows(); // m_pFiffInfoInput->nchan
            m_iDataBufferSize = pRTMSA->getMultiSampleArray().first().cols();
            m_dDataSampFreq = pickedInfo.sfreq;
            m_iEEGNumChannels = pickedInfo.nchan;
            m_sEEGChNames = pickedInfo.ch_names;
            //m_sSelectedChNames = m_sEEGChNames.at(m_iSelectedNumChannels);
            QSettings settings("MNECPP");
            settings.setValue(m_sSettingsPath + QString("/samplingFrequency"), m_dDataSampFreq);
            settings.setValue(m_sSettingsPath + QString("/eegChNames"), m_sEEGChNames);
        }

        /** 1: m_pFiffInfoOutput is not initialized;
             *  2: m_pFiffInfoOutput is initialized from EEG output type.
             *  3: m_iNumPickedChannels is changed.
             *  4: m_iNumBins is changed.
             */
        //if (!m_pFiffInfoOutput) {
        //if (!m_pFiffInfoOutput || m_bNumPickedChUpdated || m_bNumBinsUpdated) {
        //if (!m_pFiffInfoOutput || !m_bBandpowerOutputInit) {
        if (!m_pFiffInfoOutput) {
            //=====================================================================================================
            //m_qMutex.lock();
            // Rearrange the output fiffinfo here.
            m_iBandpowerNumChannels = m_iNumPickedChannels*m_iNumBins;

            QList<FIFFLIB::FiffChInfo> fiffChInfoList;
            FIFFLIB::FiffChInfo fiffChInfo;
            QStringList chNameList;
            //fiffChInfo.ch_name = "BP";
            fiffChInfo.kind     = 502; //pickedInfo.chs.at(0).kind;    // 502, misc: miscellaneous analog channels.
            fiffChInfo.range    = -1; //pickedInfo.chs.at(0).range;   // -1
            fiffChInfo.unit     = -1; //pickedInfo.chs.at(0).unit;    // -1
            for (int i = 0; i < m_iNumPickedChannels; ++i) {
                for (int j = 0; j < m_iNumBins; ++j) {
                    //QString tempStr = QString("BP%1").arg(i);
                    fiffChInfo.ch_name = m_sEEGChNames.at(m_sPickedChNames.at(i).toInt()) + QString("-BP%1").arg(j);
                    chNameList.append(fiffChInfo.ch_name);
                    fiffChInfoList.append(fiffChInfo);
                }
            }

            //m_pFiffInfoOutput = QSharedPointer<FIFFLIB::FiffInfo>(new FIFFLIB::FiffInfo(pickedInfo));
            m_pFiffInfoOutput = FIFFLIB::FiffInfo::SPtr(new FIFFLIB::FiffInfo(*pRTMSA->info().data()));
            m_pFiffInfoOutput->filename = "Bandpower";
            m_pFiffInfoOutput->bads.clear();
            m_pFiffInfoOutput->nchan = m_iBandpowerNumChannels;
            m_pFiffInfoOutput->ch_names = chNameList;
            m_pFiffInfoOutput->chs = fiffChInfoList;
            m_pBandpowerOutput->measurementData()->initFromFiffInfo(m_pFiffInfoOutput);
            m_pBandpowerOutput->measurementData()->setMultiArraySize(1);
            m_bNumPickedChUpdated = false;
            m_bNumBinsUpdated = false;
            m_bBandpowerOutputInit = true;

            //m_qMutex.unlock();
        }

        if(!m_bPluginControlWidgetsInit) {
            initPluginControlWidgets();
        }

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
void Bandpower::onChangeBandpowerMinFreq(double dMinFreq)
{
    QMutexLocker locker(&m_qMutex);

    m_dMinFreq = dMinFreq;
}

//=====================================================================================================================
void Bandpower::onChangeBandpowerMaxFreq(double dMaxFreq)
{
    QMutexLocker locker(&m_qMutex);

    m_dMaxFreq = dMaxFreq;
}

//=====================================================================================================================
void Bandpower::onChangeBandpowerPickedChannels(QStringList sPickedChIndex)
{
    QMutexLocker locker(&m_qMutex);

    if (!sPickedChIndex.isEmpty()) {
        m_iNumPickedChannels = sPickedChIndex.size();
        m_sPickedChNames = sPickedChIndex;
    } else {
        m_iNumPickedChannels = 1;
        m_sPickedChNames.clear(); // empty, reset to default.
        m_sPickedChNames.append(QString::number(0));
    }

    m_bNumPickedChUpdated = true;
    m_bBandpowerOutputInit = false;
}

//=====================================================================================================================
void Bandpower::onChangeBandpowerNumBins(int iNumBins)
{
    QMutexLocker locker(&m_qMutex);

    m_iNumBins = iNumBins;
    m_bNumBinsUpdated = true;
    m_bBandpowerOutputInit = false;
}

//=====================================================================================================================
void Bandpower::onChangeBandpowerSegmentLength(double dSegmentLength)
{
    QMutexLocker locker(&m_qMutex);

    m_dSegmentLength = dSegmentLength;
    m_bSegmentInit = false;
}

//=====================================================================================================================
void Bandpower::onChangeBandpowerSegmentStep(double dSegmentStep)
{
    QMutexLocker locker(&m_qMutex);

    m_dSegmentStep = dSegmentStep;
    m_bSegmentInit = false;
}

//=====================================================================================================================
void Bandpower::onChangeBandpowerSpectrumMethod(const QString &sSpectrumMethod)
{
    QMutexLocker locker(&m_qMutex);

    m_sSpectrumMethod = sSpectrumMethod;
}

//=====================================================================================================================
void Bandpower::onChangeBandpowerDetrendMethod(const QString &sDetrendMethod)
{
    QMutexLocker locker(&m_qMutex);

    m_sDetrendMethod = sDetrendMethod;
}

//=====================================================================================================================
void Bandpower::onChangeBandpowerAROrder(int arOrder)
{
    QMutexLocker locker(&m_qMutex);

    m_iAROrder = arOrder;
}

//=====================================================================================================================
void Bandpower::onChangeBandpowerAREvaluationPoints(int iAREvaluationPoints)
{
    QMutexLocker locker(&m_qMutex);

    m_iAREvaluationPoints = iAREvaluationPoints;
}

//=====================================================================================================================
void Bandpower::initPluginControlWidgets()
{
    if(m_pFiffInfoInput) {
        QList<QWidget*> plControlWidgets;

        // The plugin's control widgets
        // BandpowerSettingsView
        BandpowerSettingsView* pBandpowerSettingsView = new BandpowerSettingsView(m_sSettingsPath);
        pBandpowerSettingsView->setObjectName("group_tab_Settings_Bandpower Settings");
        connect(pBandpowerSettingsView, &BandpowerSettingsView::sig_updateMinFreq,
                this, &Bandpower::onChangeBandpowerMinFreq);
        connect(pBandpowerSettingsView, &BandpowerSettingsView::sig_updateMaxFreq,
                this, &Bandpower::onChangeBandpowerMaxFreq);
        connect(pBandpowerSettingsView, &BandpowerSettingsView::sig_updatePickedChannels,
                this, &Bandpower::onChangeBandpowerPickedChannels);
        connect(pBandpowerSettingsView, &BandpowerSettingsView::sig_updateNumBins,
                this, &Bandpower::onChangeBandpowerNumBins);
        connect(pBandpowerSettingsView, &BandpowerSettingsView::sig_updateSegmentLength,
                this, &Bandpower::onChangeBandpowerSegmentLength);
        connect(pBandpowerSettingsView, &BandpowerSettingsView::sig_updateSegmentStep,
                this, &Bandpower::onChangeBandpowerSegmentStep);
        connect(pBandpowerSettingsView, &BandpowerSettingsView::sig_updateSpectrumMethod,
                this, &Bandpower::onChangeBandpowerSpectrumMethod);
        connect(pBandpowerSettingsView, &BandpowerSettingsView::sig_updateDetrendMethod,
                this, &Bandpower::onChangeBandpowerDetrendMethod);
        plControlWidgets.append(pBandpowerSettingsView);

        // ARSettingsView
        BandpowerARSettingsView* pARSettingsView = new BandpowerARSettingsView(m_sSettingsPath);
        pARSettingsView->setObjectName("group_tab_Settings_AR Settings");
        connect(pARSettingsView, &BandpowerARSettingsView::sig_updateAROrder,
                this, &Bandpower::onChangeBandpowerAROrder);
        connect(pARSettingsView, &BandpowerARSettingsView::sig_updateAREvaluationPoints,
                this, &Bandpower::onChangeBandpowerAREvaluationPoints);
        plControlWidgets.append(pARSettingsView);

        // emit plugin's control widgets
        emit pluginControlWidgetsChanged(plControlWidgets, this->getName());

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

        m_bPluginControlWidgetsInit = true;
    }
}

//=====================================================================================================================
void Bandpower::initSegment()
{

}

//=====================================================================================================================
void Bandpower::run()
{
    // Wait for Fiff Info
    while(!m_pFiffInfoInput) {
        msleep(10);
    }

    //=================================================================================================================
    // init segment data for the first data segment.
    int iSegLen  = static_cast<int>(m_dSegmentLength * m_dDataSampFreq / 1000.0); // length of data block/segment to be processed.
    int iSegStep = static_cast<int>(m_dSegmentStep * m_dDataSampFreq / 1000.0); // step length for the block/segment sliding/moving/updating.
    //MatrixXd    matSegment(m_iNumPickedChannels, iSegLen); // segment data matrix to be processed.
    MatrixXd    matTempSeg(m_iNumPickedChannels, (iSegLen/m_iDataBufferSize + 1)*m_iDataBufferSize); // temp considering input buffersize.
    MatrixXd    matSegment;
    MatrixXd    matTemp;
    MatrixXd    matDataInput;

    // detremine spectrum resolution - calculation only for FFT, but we keep it for both methods here to keep the resolution.
    VectorXd    vecFFTResFreqs = Spectral::calculateFFTFreqs(iSegLen, m_dDataSampFreq);

    // prepare storage for the first data segment.
    for (int i = 0; i <= iSegLen/m_iDataBufferSize; ++i) {
        while (!m_pBandpowerBuffer->pop(matTemp));
        for (int j = 0; j < m_iNumPickedChannels; ++j) {
            matTempSeg.block(j, i*m_iDataBufferSize, 1, m_iDataBufferSize) = matTemp.row(m_vecEEGChPicks(m_sPickedChNames.at(j).toInt()));
        }
        //matTempSegment.middleCols(i*m_iDataBufferSize, m_iDataBufferSize) = matTemp;
    }
    matSegment = matTempSeg.leftCols(iSegLen);
    matTemp = matTempSeg.rightCols(matTempSeg.cols() - iSegLen);

    m_bSegmentInit = true;

    //=================================================================================================================
    // compute bandpower features.
    while(!isInterruptionRequested()) {

        //QMutexLocker locker(&m_qMutex);
        m_qMutex.lock();

        // detrend
        MatrixXd matSegmentDetrend = BPSpectral::detrendData(matSegment, m_sDetrendMethod);

        // calculate bandpowers
        QVector<VectorXd> matSpectrum;
        MatrixXd matBandpower(m_iBandpowerNumChannels, 1);

        if (m_sSpectrumMethod == "AR") {
            MatrixXcd ARSpectraWeights = BPSpectral::generateARSpectraWeights(m_dMinFreq/m_dDataSampFreq, m_dMaxFreq/m_dDataSampFreq,
                                                                              m_iNumBins, m_iAREvaluationPoints, true);
            QVector<QPair<VectorXd, double>> ARCoeffs = BPSpectral::calculateARWeightsMEMMatrix(matSegmentDetrend, m_iAROrder, true);
            matSpectrum = BPSpectral::psdFromARSpectra(ARCoeffs, ARSpectraWeights, m_dDataSampFreq, true);

            //if (m_iNumPickedChannels <= matSpectrum.length()) {
            //    qDebug() << "[BandPower::run]More channels selected than pre-defined! Only the first "
            //                 << m_iNumPickedChannels << " are displayed!";
            //}

            for (int i = 0; i < std::min(m_iNumPickedChannels, matSpectrum.length()); ++i) {
                matBandpower.block(i*m_iNumBins, 0, m_iNumBins, 1) = matSpectrum.at(i);
            }
            matSpectrum.clear();
        } else if (m_sSpectrumMethod == "FFT") {
            // generate hanning window
            QPair<MatrixXd, VectorXd> tapers = Spectral::generateTapers(iSegLen, "hanning");
            MatrixXd matTapers = tapers.first;
            VectorXd vecTapersWeights = tapers.second;

            // compute spectrum
            QVector<MatrixXcd> matTaperedSpectrum;
            matTaperedSpectrum = Spectral::computeTaperedSpectraMatrix(matSegmentDetrend, matTapers, iSegLen, true);
            matSpectrum = BPSpectral::psdFromTaperedSpectra_BP(matTaperedSpectrum, vecTapersWeights, iSegLen, m_dDataSampFreq, false);

            // select frequencies that fall within the band
            //if (m_iNumPickedChannels <=matSpectrum.length()) {
            //    qDebug() << "[BandPower::run] More channels selected than pre-defined! Only the first "
            //             << m_iNumPickedChannels << " are displayed!";
            //}
            m_dBinWidth = (m_dMaxFreq - m_dMinFreq) / static_cast<double>(m_iNumBins);
            if (m_dBinWidth < vecFFTResFreqs[1] - vecFFTResFreqs[0]) {
                qDebug() << "[BandPower::run] Selected bin width is smaller than FFT resolution";
            }

            for (int i = 0; i < std::min(m_iNumPickedChannels, matSpectrum.length()); ++i) {
                for (int j = 0; j < m_iNumPickedChannels; ++j) {
                    matBandpower(i*m_iNumBins + j, 1) = BPSpectral::bandpowerFromSpectrumEntriesOffset(vecFFTResFreqs,
                                                                                                       matSpectrum.at(i),
                                                                                                       m_dMinFreq + j*m_dBinWidth,
                                                                                                       m_dMinFreq + (j+1)*m_dBinWidth);
                }
            }
            matSpectrum.clear();
        }

        m_pBandpowerOutput->measurementData()->setValue(matBandpower);

        m_qMutex.unlock();

        //=============================================================================================================
        if(!isInterruptionRequested()) {
            if (m_bSegmentInit && m_bBandpowerOutputInit) {
                // send the data to the connected plugins and the online display
                // unocmment this if you also uncommented the m_pOutput in the constructor above
                //m_pBandpowerOutput->measurementData()->setValue(matBandpower);

                // move forward the segment matrix entries (i.e., matSegment and matTemp) by one iSegmentStep.
                //m_qMutex.lock();
                if (matTemp.cols() >= iSegStep) {
                    MatrixXd tempSeg(m_iNumPickedChannels, iSegLen);
                    tempSeg << matSegment.rightCols(iSegLen - iSegStep), matTemp.leftCols(iSegStep);
                    matSegment = tempSeg;
                    //MatrixXd temp;
                    //temp = matTemp.rightCols(matTemp.cols() - iSegStep);
                    //matTemp = temp;
                    matTemp = matTemp.rightCols(matTemp.cols() - iSegStep);
                } else {
                    //MatrixXd matBufferInput;
                    while (matTemp.cols() < iSegStep) {
                        MatrixXd temp(m_iNumPickedChannels, matTemp.cols() + m_iDataBufferSize);
                        MatrixXd tempDataInput(m_iNumPickedChannels, m_iDataBufferSize);
                        while(!m_pBandpowerBuffer->pop(matDataInput));
                        for (int i = 0; i < m_iNumPickedChannels; ++i) {
                            tempDataInput.row(i) = matDataInput.row(m_vecEEGChPicks(m_sPickedChNames.at(i).toInt()));
                        }
                        temp << matTemp, tempDataInput;
                        matTemp = temp;
                    }
                    MatrixXd tempSeg(m_iNumPickedChannels, iSegLen);
                    tempSeg << matSegment.rightCols(iSegLen - iSegStep), matTemp.leftCols(iSegStep);
                    matSegment = tempSeg;
                    matTemp = matTemp.rightCols(matTemp.cols() - iSegStep);
                }
                //m_qMutex.unlock();
            } else {
                //=====================================================================================================
                // changes bandpowerOutput
                if (!m_bBandpowerOutputInit) {
                    //m_qMutex.lock();
                    // Rearrange the output fiffinfo here.
                    m_iBandpowerNumChannels = m_iNumPickedChannels*m_iNumBins;

                    QList<FIFFLIB::FiffChInfo> fiffChInfoList;
                    FIFFLIB::FiffChInfo fiffChInfo;
                    QStringList chNameList;
                    //fiffChInfo.ch_name = "BP";
                    fiffChInfo.kind     = 502; //pickedInfo.chs.at(0).kind;    // 502, misc: miscellaneous analog channels.
                    fiffChInfo.range    = -1; //pickedInfo.chs.at(0).range;   // -1
                    fiffChInfo.unit     = -1; //pickedInfo.chs.at(0).unit;    // -1
                    for (int i = 0; i < m_iNumPickedChannels; ++i) {
                        for (int j = 0; j < m_iNumBins; ++j) {
                            //QString tempStr = QString("BP%1").arg(i);
                            fiffChInfo.ch_name = m_sEEGChNames.at(m_sPickedChNames.at(i).toInt()) + QString("-BP%1").arg(j);
                            chNameList.append(fiffChInfo.ch_name);
                            fiffChInfoList.append(fiffChInfo);
                        }
                    }

                    //m_pFiffInfoOutput = FIFFLIB::FiffInfo::SPtr(new FIFFLIB::FiffInfo(*pRTMSA->info().data()));
                    m_pFiffInfoOutput->filename = "Bandpower";
                    m_pFiffInfoOutput->bads.clear();
                    m_pFiffInfoOutput->nchan = m_iBandpowerNumChannels;
                    m_pFiffInfoOutput->ch_names = chNameList;
                    m_pFiffInfoOutput->chs = fiffChInfoList;
                    m_pBandpowerOutput->measurementData()->initFromFiffInfo(m_pFiffInfoOutput);
                    m_pBandpowerOutput->measurementData()->setMultiArraySize(1);
                    m_bBandpowerOutputInit = true;

                    m_pBandpowerOutput = PluginOutputData<RealTimeMultiSampleArray>::create(this, "BandpowerOut", "Bandpower output data");
                    m_pBandpowerOutput->measurementData()->setName(this->getName());
                    m_outputConnectors.append(m_pBandpowerOutput);
                    //m_qMutex.unlock();
                }

                //=====================================================================================================
                // init segment data for the first data segment.
                if (!m_bSegmentInit) {
                    //m_qMutex.lock();
                    iSegLen  = static_cast<int>(m_dSegmentLength * m_dDataSampFreq / 1000.0);
                    iSegStep = static_cast<int>(m_dSegmentStep * m_dDataSampFreq / 1000.0);
                    MatrixXd    matTempSeg(m_iNumPickedChannels, (iSegLen/m_iDataBufferSize + 1)*m_iDataBufferSize);

                    // detremine spectrum resolution - calculation only for FFT, but we keep it for both methods here to keep the resolution.
                    vecFFTResFreqs = Spectral::calculateFFTFreqs(iSegLen, m_dDataSampFreq);

                    // prepare storage for the first data segment.
                    for (int i = 0; i <= iSegLen/m_iDataBufferSize; ++i) {
                        while (!m_pBandpowerBuffer->pop(matTemp));
                        for (int j = 0; j < m_iNumPickedChannels; ++j) {
                            matTempSeg.block(j, i*m_iDataBufferSize, 1, m_iDataBufferSize) = matTemp.row(m_vecEEGChPicks(m_sPickedChNames.at(j).toInt()));
                        }
                        //matTempSegment.middleCols(i*m_iDataBufferSize, m_iDataBufferSize) = matTemp;
                    }
                    matSegment = matTempSeg.leftCols(iSegLen);
                    matTemp = matTempSeg.rightCols(matTempSeg.cols() - iSegLen);

                    m_bSegmentInit = true;
                    //m_qMutex.unlock();
                }
            }
        } //if(!isInterruptionRequested())
    } // while (!isInterruptionRequested())
}
