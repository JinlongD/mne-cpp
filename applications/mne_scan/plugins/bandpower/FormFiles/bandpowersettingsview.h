//=====================================================================================================================
/**
 * @file     bandpowersettingsview.h
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
 * @brief    Contains the declaration of the bandpowersettingsview class.
 *
 */
//=====================================================================================================================
#ifndef BANDPOWERSETTINGSVIEW_H
#define BANDPOWERSETTINGSVIEW_H

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================

//=====================================================================================================================
// QT INCLUDES
//=====================================================================================================================
#include <QWidget>

//=====================================================================================================================
// EIGEN INCLUDES
//=====================================================================================================================

//=====================================================================================================================
// FORWARD DECLARATIONS
//=====================================================================================================================
namespace Ui{
    class BandpowerSettingsView;
}

//=====================================================================================================================
// DEFINE NAMESPACE BANDPOWERPLUGIN
//=====================================================================================================================
namespace BANDPOWERPLUGIN
{

//=====================================================================================================================
// BANDPOWERPLUGIN FORWARD DECLARATIONS
//=====================================================================================================================
class BandpowerChannelSelectionView;

//=====================================================================================================================
/**
 * DECLARE CLASS BandpowerSettingsView
 *
 * @brief The BandpowerSettingsView class provides a bandpower settings widget docked in quickcontrol window.
 */
class BandpowerSettingsView : public QWidget
{
    Q_OBJECT

public:
    typedef QSharedPointer<BandpowerSettingsView> SPtr;         /**< Shared pointer type for BandpowerSettingsView. */
    typedef QSharedPointer<const BandpowerSettingsView> ConstSPtr; /**< Const shared pointer type for BandpowerSettingsView. */

    //=================================================================================================================
    /**
     * @brief Constructs a BandpowerSettingsView.
     * @param [in] sSettingsPath
     * @param [in] parent
     */
    explicit BandpowerSettingsView(const QString& sSettingsPath = "",
                                   const QStringList& sEEGChNames = {},
                                   QStringList sPickedChNames = {"0"},
                                   const double& dSampFreq = 100.0,
                                   QWidget *parent = 0);

    //=================================================================================================================
    /**
     * @brief Destroys the BandpowerSettingsView.
     */
    ~BandpowerSettingsView();

    //=========================================================================================================
    /**
     * @brief Slot called when m_qDoubleSpinBox_MinFreq change.
     * @param currentValue
     */
    void onDoubleSpinBoxMinFreq(double currentValue);

    //=========================================================================================================
    /**
     * @brief Slot called when m_qDoubleSpinBox_MaxFreq change.
     * @param currentValue
     */
    void onDoubleSpinBoxMaxFreq(double currentValue);

    //=========================================================================================================
    /**
     * @brief Slot called when m_qSpinBox_NumBins change.
     * @param currentValue
     */
    void onSpinBoxNumBins(int currentValue);

    //=========================================================================================================
    /**
     * @brief Slot called when m_qPushButton_SelectChannels change.
     */
    void onPushButtonSelectChannels();

    //=========================================================================================================
    /**
     * @brief Slot called when m_qDoubleSpinBox_SegmentLength change.
     * @param currentValue
     */
    void onDoubleSpinBoxSegmentLength(double currentValue);

    //=========================================================================================================
    /**
     * @brief Slot called when m_qDoubleSpinBox_SegmentStep change.
     * @param currentValue
     */
    void onDoubleSpinBoxSegmentStep(double currentValue);

    //=========================================================================================================
    /**
     * @brief Slot called when m_qComboBox_SpectrumMethod change.
     * @param sSpectrumMethod
     */
    void onComboBoxSpectrumMethod(const QString &sSpectrumMethod);

    //=========================================================================================================
    /**
     * @brief Slot called when m_qComboBox_DetrendMethod change.
     * @param sDetrendMethod
     */
    void onComboBoxDetrendMethod(const QString &sDetrendMethod);

    //=================================================================================================================
    /**
     * @brief slot called when picked channels change.
     * @param sPickedChNames
     */
    void onUpdatePickedChannelNames(QStringList sPickedChNames);

private:
    //=================================================================================================================
    /**
     * @brief Saves all important settings of this view via QSettings.
     */
    void saveSettings();

    //=================================================================================================================
    /**
     * @brief Loads and inits all important settings of this view via QSettings.
     */
    void loadSettings();

private:
    QString                     m_sSettingsPath;    /**< The settings path to store the GUI settings to. */
    Ui::BandpowerSettingsView*  m_pUi;              /**< The UI class specified in the designer. */

    double      m_dDataSampFreq;
    QStringList m_sEEGChNames;
    QStringList m_sPickedChIndex;

    double      m_dBinWidth;
    double      m_dMaxFreq;
    double      m_dMinFreq;
    double      m_dSegmentLength;
    double      m_dSegmentStep;
    int         m_iNumBins;
    int         m_iNumPickedChannels;
    QString     m_sSpectrumMethod;
    QString     m_sDetrendMethod;

    //BandpowerChannelSelectionView *m_pChannelSelectionView;
    QSharedPointer<BandpowerChannelSelectionView> m_pChannelSelectionView;

signals:
    //=================================================================================================================
    /**
     * @brief Emitted whenever the settings changed and are ready to be retreived.
     * @param minFreq
     */
    void sig_initBandpowerSettings(const QString& sSettingsPath);
    void sig_updateMinFreq(double minFreq);
    void sig_updateMaxFreq(double maxFreq);
    void sig_updatePickedChannels(QStringList sPickedChIndex);
    void sig_updateNumBins(int value);
    void sig_updateSegmentLength(int value);
    void sig_updateSegmentStep(int value);
    void sig_updateSpectrumMethod(const QString& sSpectrumMethod);
    void sig_updateDetrendMethod(const QString& sDetrendMethod);
};
}   //namespace

#endif // BANDPOWERSETTINGSVIEW_H
