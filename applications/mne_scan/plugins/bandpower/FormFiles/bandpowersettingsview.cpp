﻿//=====================================================================================================================
/**
 * @file     bandpowersettingsview.cpp
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
 * @brief    Definition of the bandpowersettingsview class.
 *
 */
//=====================================================================================================================

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================
#include "bandpowersettingsview.h"
#include "ui_bandpowersettingsview.h"

#include "bandpowerchannelselectionview.h"

#include <disp/viewers/channelselectionview.h>
#include <disp/viewers/helpers/channelinfomodel.h>
#include <disp/viewers/rtfiffrawview.h>

//=====================================================================================================================
// QT INCLUDES
//=====================================================================================================================
#include <QSettings>
#include <QDebug>

//=====================================================================================================================
// EIGEN INCLUDES
//=====================================================================================================================

//=====================================================================================================================
// USED NAMESPACES
//=====================================================================================================================
using namespace BANDPOWERPLUGIN;

//=====================================================================================================================
// DEFINE MEMBER METHODS
//=====================================================================================================================
BandpowerSettingsView::BandpowerSettingsView(const QString& sSettingsPath,
                                             QWidget *parent)
    : QWidget(parent)
    , m_pUi(new Ui::BandpowerSettingsView)
{
    m_sSettingsPath = sSettingsPath;
    m_pUi->setupUi(this);

    loadSettings();

    m_pUi->m_qDoubleSpinBox_MinFreq->setMinimum(1.0);
    m_pUi->m_qDoubleSpinBox_MinFreq->setMaximum(m_dDataSampFreq/2.0 - 1.0);
    m_pUi->m_qDoubleSpinBox_MaxFreq->setMinimum(2.0);
    m_pUi->m_qDoubleSpinBox_MaxFreq->setMaximum(m_dDataSampFreq/2.0); // static_cast<int>(m_dDataSampFreq/2.0)

    m_dBinWidth = (m_dMaxFreq - m_dMinFreq) / (static_cast<double>(m_iNumBins));
    m_pUi->m_qLabel_binWidth->setText(QString("(Binwidth: %1 Hz.)").arg(m_dBinWidth));
    //m_pUi->m_qSpinBox_nBins->setMinimum(1);

    m_pUi->m_qDoubleSpinBox_MinFreq->setValue(m_dMinFreq);
    //connect(m_pUi->m_qDoubleSpinBox_MinFreq, &QDoubleSpinBox::editingFinished,
    //        this, &BandpowerSettingsView::onChangeDoubleSpinBoxMinFreq);
    connect(m_pUi->m_qDoubleSpinBox_MinFreq, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &BandpowerSettingsView::onChangeDoubleSpinBoxMinFreq);

    m_pUi->m_qDoubleSpinBox_MaxFreq->setValue(m_dMaxFreq);
    connect(m_pUi->m_qDoubleSpinBox_MaxFreq, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &BandpowerSettingsView::onChangeDoubleSpinBoxMaxFreq);

    m_pUi->m_qSpinBox_nBins->setValue(m_iNumBins);
    //connect(m_pUi->m_qSpinBox_nBins, &QSpinBox::editingFinished,
    //        this, &BandpowerSettingsView::onChangeSpinBoxNumBins);
    connect(m_pUi->m_qSpinBox_nBins, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &BandpowerSettingsView::onChangeSpinBoxNumBins);

    if (m_sPickedChIndex.isEmpty()) {
        m_pUi->m_qLabel_nChannels_show->setText(QString("*1")); //* indicates default: picked 1st EEG channel.
    } else {
        m_pUi->m_qLabel_nChannels_show->setText(QString("%1").arg(m_iNumPickedChannels));
    }
    connect(m_pUi->m_qPushButton_PickChannels, &QPushButton::clicked,
            this, &BandpowerSettingsView::onChangePushButtonSelectChannels);

    m_pUi->m_qDoubleSpinBox_SegmentLength->setValue(m_dSegmentLength);
    connect(m_pUi->m_qDoubleSpinBox_SegmentLength, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &BandpowerSettingsView::onChangeDoubleSpinBoxSegmentLength);

    m_pUi->m_qDoubleSpinBox_SegmentStep->setValue(m_dSegmentStep);
    connect(m_pUi->m_qDoubleSpinBox_SegmentStep, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &BandpowerSettingsView::onChangeDoubleSpinBoxSegmentStep);

    m_pUi->m_qComboBox_Method->setCurrentText(m_sSpectrumMethod);
    connect(m_pUi->m_qComboBox_Method, &QComboBox::currentTextChanged,
            this, &BandpowerSettingsView::onChangeComboBoxSpectrumMethod);

    m_pUi->m_qComboBox_Detrend->setCurrentText(m_sDetrendMethod);
    connect(m_pUi->m_qComboBox_Detrend, &QComboBox::currentTextChanged,
            this, &BandpowerSettingsView::onChangeComboBoxDetrendMethod);
}

//=====================================================================================================================
BandpowerSettingsView::~BandpowerSettingsView()
{
    saveSettings();
    delete m_pUi;
}

//=============================================================================================================
void BandpowerSettingsView::onChangeDoubleSpinBoxMinFreq(double currentValue)
{
    //double currentValue = m_pUi->m_qDoubleSpinBox_MinFreq->value();

    if(currentValue >= m_dMaxFreq) {
        currentValue = m_dMaxFreq - 1;
        m_pUi->m_qDoubleSpinBox_MinFreq->setValue(currentValue);
        qDebug() << "[BandpowerSettingsView:: onChangeSpinBoxMinFreq]: "
                 << "Minimum frequency should not be larger than maximum frequency and has been reset to "
                 << currentValue;
    } else if (currentValue < m_pUi->m_qDoubleSpinBox_MinFreq->minimum()) {
        currentValue = m_pUi->m_qDoubleSpinBox_MinFreq->minimum();
        qDebug() << "[BandpowerSettingsView:: onChangeSpinBoxMinFreq]: "
                 << "Minimum frequency should not be smaller than 1 Hz and has been reset to 1 Hz.";
    }

    if(currentValue == m_dMinFreq) {
        return;
    }

    m_dMinFreq = currentValue;
    m_dBinWidth = (m_dMaxFreq - m_dMinFreq) / (static_cast<double>(m_iNumBins));
    m_pUi->m_qLabel_binWidth->setText(QString("(Binwidth: %1 Hz.)").arg(m_dBinWidth));
    saveSettings();
    emit sig_updateMinFreq(m_dMinFreq);
}

//=============================================================================================================
void BandpowerSettingsView::onChangeDoubleSpinBoxMaxFreq(double currentValue)
{
    //double currentValue = m_pUi->m_qDoubleSpinBox_MaxFreq->value();

    if(currentValue <= m_dMinFreq) {
        currentValue = m_dMinFreq + 1;
        m_pUi->m_qDoubleSpinBox_MaxFreq->setValue(currentValue);
        qDebug() << "[BandpowerSettingsView:: onChangeSpinBoxMaxFreq]: "
                 << "Maximum frequency should not be smaller than minimum frequency and has been reset to "
                 << currentValue;
    } else if (currentValue > m_pUi->m_qDoubleSpinBox_MaxFreq->maximum()) {
        currentValue = m_pUi->m_qDoubleSpinBox_MaxFreq->maximum();
        qDebug() << "[BandpowerSettingsView:: onChangeSpinBoxMaxFreq]: "
                 << "Maximum frequency should not be larger than " << currentValue << "and has been reset to "
                 << currentValue;
    }

    if(currentValue == m_dMaxFreq) {
        return;
    }

    m_dMaxFreq = currentValue;
    m_dBinWidth = (m_dMaxFreq - m_dMinFreq) / (static_cast<double>(m_iNumBins));
    m_pUi->m_qLabel_binWidth->setText(QString("(Binwidth: %1 Hz.)").arg(m_dBinWidth));
    saveSettings();
    emit sig_updateMaxFreq(m_dMaxFreq);
}

//=============================================================================================================
void BandpowerSettingsView::onChangeSpinBoxNumBins(int currentValue)
{
    //int currentValue = m_pUi->m_qSpinBox_nBins->value();

    if(currentValue == m_iNumBins) {
        return;
    }

    m_iNumBins = currentValue;
    m_dBinWidth = (m_dMaxFreq - m_dMinFreq) / (static_cast<double>(m_iNumBins));
    m_pUi->m_qLabel_binWidth->setText(QString("(Binwidth: %1 Hz.)").arg(m_dBinWidth));
    saveSettings();
    emit sig_updateNumBins(m_iNumBins);
}

//=============================================================================================================
void BandpowerSettingsView::onChangePushButtonSelectChannels()
{
    //BandpowerChannelSelectionView *pChannelSelectionView = new BandpowerChannelSelectionView(m_sSettingsPath, this);
    if (m_sEEGChNames.isEmpty()) {
        qWarning() << "[BandpowerSettingsView:: onChangePushButtonSelectChannels]: "
                 << "No available EEG channels, please check the connection to the measurements.";
        return;
    }

    // init channel selection view
    m_pChannelSelectionView = BandpowerChannelSelectionView::SPtr::create(m_sSettingsPath);
    //BandpowerChannelSelectionView::SPtr pChannelSelectionView = new BandpowerChannelSelectionView::SPtr->create(m_sSettingsPath);
    connect(m_pChannelSelectionView.data(), &BandpowerChannelSelectionView::sig_updatePickedChNames,
            this, &BandpowerSettingsView::onChangePickedChannelNames);
    m_pChannelSelectionView->show();
}

//=============================================================================================================
void BandpowerSettingsView::onChangeDoubleSpinBoxSegmentLength(double currentValue)
{
    //int currentValue = m_pUi->m_qDoubleSpinBox_SegmentLength->value();
    if(currentValue == m_dSegmentLength)
        return;

    m_dSegmentLength = currentValue;
    saveSettings();
    emit sig_updateSegmentLength(m_dSegmentLength);
}

//=============================================================================================================
void BandpowerSettingsView::onChangeDoubleSpinBoxSegmentStep(double currentValue)
{
    //int currentValue = m_pUi->m_qDoubleSpinBox_SegmentStep->value();

    if(currentValue == m_dSegmentStep) {
        return;
    } else if (currentValue > m_dSegmentLength) {
        qWarning() << QString("[BandpowerSettingsView::onChangeSpinBoxSegmentStep]: update stemp for segment cannot be ")
                      + QString("larger than segment length and has been reset to be the same as segment length.");
        currentValue = m_dSegmentLength;
    }

    m_dSegmentStep = currentValue;
    saveSettings();
    emit sig_updateSegmentStep(m_dSegmentStep);
}

//=====================================================================================================================
void BandpowerSettingsView::onChangeComboBoxSpectrumMethod(const QString &sSpectrumMethod)
{
    m_sSpectrumMethod = sSpectrumMethod;
    saveSettings();
    emit sig_updateSpectrumMethod(m_sSpectrumMethod);
}

//=====================================================================================================================
void BandpowerSettingsView::onChangeComboBoxDetrendMethod(const QString &sDetrendMethod)
{
    m_sDetrendMethod = sDetrendMethod;
    saveSettings();
    emit sig_updateDetrendMethod(m_sDetrendMethod);
}

//=====================================================================================================================
void BandpowerSettingsView::onChangePickedChannelNames(QStringList sPickedChNames)
{
    sPickedChNames.sort(Qt::CaseSensitive);

    if (m_sPickedChIndex == sPickedChNames) {
        return;
    } else {
        if (!sPickedChNames.isEmpty()) {
            m_iNumPickedChannels = sPickedChNames.size();
            m_sPickedChIndex = sPickedChNames;
            //m_sPickedChIndex.sort(Qt::CaseSensitive);
            m_pUi->m_qLabel_nChannels_show->setText(QString::number(m_iNumPickedChannels));
        } else {
            m_iNumPickedChannels = 1;
            //m_sPickedChIndex.clear();
            m_sPickedChIndex = sPickedChNames;
            m_pUi->m_qLabel_nChannels_show->setText(QString("*%1").arg(m_iNumPickedChannels));
            qWarning() << "[BandpowerSettingsView::onChangePickedChannelNames]: No channel is selected. "
                       << "Picked channels are set to default: selected the first available EEG channel.";
        }
        saveSettings();
        emit sig_updatePickedChannels(m_sPickedChIndex);
        qDebug() << "[BandpowerSettingsView::onChangePickedChannelNames]: Picked channels: " << m_sPickedChIndex;
    }
}

//=====================================================================================================================
void BandpowerSettingsView::saveSettings()
{
    if(m_sSettingsPath.isEmpty()) {
        return;
    }

    QSettings settings("MNECPP");
    settings.setValue(m_sSettingsPath + QString("/minFreq"), m_dMinFreq);
    settings.setValue(m_sSettingsPath + QString("/maxFreq"), m_dMaxFreq);
    settings.setValue(m_sSettingsPath + QString("/numBins"), m_iNumBins);
    settings.setValue(m_sSettingsPath + QString("/numPickedChannels"), m_iNumPickedChannels);
    settings.setValue(m_sSettingsPath + QString("/segmentLength"), m_dSegmentLength);
    settings.setValue(m_sSettingsPath + QString("/segmentStep"), m_dSegmentStep);
    settings.setValue(m_sSettingsPath + QString("/spectrumMethod"), m_sSpectrumMethod);
    settings.setValue(m_sSettingsPath + QString("/detrendMethod"), m_sDetrendMethod);
    settings.setValue(m_sSettingsPath + QString("/pickedChNames"), m_sPickedChIndex);
}

//=====================================================================================================================
void BandpowerSettingsView::loadSettings()
{
    if(m_sSettingsPath.isEmpty()) {
        return;
    }

    QSettings settings("MNECPP");
    m_dMinFreq              = settings.value(m_sSettingsPath + QString("/minFreq"), 8.0).toDouble(); // default 8.0 Hz.
    m_dMaxFreq              = settings.value(m_sSettingsPath + QString("/maxFreq"), 30.0).toDouble(); // default 30.0 Hz.
    m_iNumBins              = settings.value(m_sSettingsPath + QString("/numBins"), 11).toInt(); // default 11 bins.
    m_iNumPickedChannels    = settings.value(m_sSettingsPath + QString("/numPickedChannels"), 1).toInt(); // default 1 channel.
    m_dSegmentLength        = settings.value(m_sSettingsPath + QString("/segmentLength"), 400).toDouble(); // default 400 ms.
    m_dSegmentStep          = settings.value(m_sSettingsPath + QString("/segmentStep"), 50).toDouble(); // default 50 ms.
    m_sSpectrumMethod       = settings.value(m_sSettingsPath + QString("/spectrumMethod"), "AR").toString(); // default AR.
    m_sDetrendMethod        = settings.value(m_sSettingsPath + QString("/detrendMethod"), "None").toString(); // default no detrend.

    m_dDataSampFreq         = settings.value(m_sSettingsPath + QString("/samplingFrequency"), 100.0).toDouble(); // default 100.0 Hz.
    m_sEEGChNames           = settings.value(m_sSettingsPath + QString("/eegChNames"), {}).toStringList(); // default empty.
    m_sPickedChIndex        = settings.value(m_sSettingsPath + QString("/pickedChNames"), {}).toStringList(); // default empty.
}
