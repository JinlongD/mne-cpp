//=====================================================================================================================
/**
 * @file     bandpower.h
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
 * @brief    Contains the declaration of the bandpower class.
 *
 */
//=====================================================================================================================
#ifndef BANDPOWER_H
#define BANDPOWER_H

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================
#include "bandpower_global.h"

#include <scShared/Plugins/abstractalgorithm.h>
#include <utils/generics/circularbuffer.h>
#include <scMeas/realtimemultisamplearray.h>

//#include <chrono>

//=====================================================================================================================
// QT INCLUDES
//=====================================================================================================================
//#include <QtWidgets>
//#include <QtCore/QtPlugin>
//#include <QPointer>

//=====================================================================================================================
// EIGEN INCLUDES
//=====================================================================================================================
//#include <Eigen/Core>
#include <Eigen/SparseCore>

//=====================================================================================================================
// FORWARD DECLARATIONS
//=====================================================================================================================
/*namespace DISPLIB {
    class ChannelSelectionView;
    class ChannelInfoModel;
}*/

//=====================================================================================================================
// DEFINE NAMESPACE BANDPOWERPLUGIN
//=====================================================================================================================
namespace BANDPOWERPLUGIN
{

//=====================================================================================================================
// BANDPOWERPLUGIN FORWARD DECLARATIONS
//=====================================================================================================================
//class BandpowerSettingsView;
//class BandpowerARSettingsView;

//=====================================================================================================================
/**
 * DECLARE CLASS Bandpower
 *
 * @brief The Bandpower class provides a Bandpower algorithm structure.
 */
class BANDPOWERSHARED_EXPORT Bandpower : public SCSHAREDLIB::AbstractAlgorithm
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "scsharedlib/1.0" FILE "bandpower.json") //New Qt5 Plugin system replaces Q_EXPORT_PLUGIN2 macro
    // Use the Q_INTERFACES() macro to tell Qt's meta-object system about the interfaces
    Q_INTERFACES(SCSHAREDLIB::AbstractAlgorithm)

public:
    //=================================================================================================================
    /**
     * @brief Constructs a Bandpower.
     */
    Bandpower();

    //=================================================================================================================
    /**
     * @brief Destroys the Bandpower.
     */
    ~Bandpower();

    //=================================================================================================================
    /**
     * AbstractAlgorithm functions
     */
    virtual QSharedPointer<SCSHAREDLIB::AbstractPlugin> clone() const;
    virtual void init();
    virtual void unload();
    virtual bool start();
    virtual bool stop();
    virtual SCSHAREDLIB::AbstractPlugin::PluginType getType() const;
    virtual QString getName() const;
    virtual QWidget* setupWidget();

    //=================================================================================================================
    /**
     * @brief Upates the pugin with new (incoming) data.
     * @param [in] pMeasurement     The incoming data in form of a generalized Measurement.
     */
    void update(SCMEASLIB::Measurement::SPtr pMeasurement);

    //=================================================================================================================
    /**
     * @brief updates the bottom frequency of the power spectrum.
     * @param [in] dMinFreq         Desired bottom frequency.
     */
    void onUpdateBandpowerMinFreq(double dMinFreq);

    //=================================================================================================================
    /**
     * @brief updates the top frequency of the power spectrum.
     * @param [in] dMaxFreq         Desired top frequency.
     */
    void onUpdateBandpowerMaxFreq(double dMaxFreq);

    //=================================================================================================================
    /**
     * @brief updates the number of bins in which bandpower is calculated for each channel.
     * @param [in] iNumBins         Number of bins.
     */
    void onUpdateBandpowerNumBins(int iNumBins);

    //=================================================================================================================
    /**
     * @brief updates the number of channels for which bandpower is analyzed.
     * @param [in] iNumChannels     Number of channels.
     */
    void onUpdateBandpowerPickedChannels(QStringList sPickedChIndex);

    //=================================================================================================================
    /**
     * @brief updates the segment length of the data block for which bandpower is analyzed.
     * @param [in] iSegmentLength   segment length of the data block (number of samples in each segment).
     */
    void onUpdateBandpowerSegmentLength(int iSegmentLength);

    //=================================================================================================================
    /**
     * @brief updates the segment updating step of the data block.
     * @param [in] iSegmentStep     update step of the data block (number of samples).
     */
    void onUpdateBandpowerSegmentStep(int iSegmentStep);

    //=================================================================================================================
    /**
     * @brief updates the method to calculate the power spectrum.
     * @param [in] sSpectrumMethod  spectrum method.
     */
    void onUpdateBandpowerSpectrumMethod(const QString &sSpectrumMethod);

    //=================================================================================================================
    /**
     * @brief updates the detrend method for the data.
     * @param [in] sDetrendMethod   Detrend method (0: none, 1: remove mean, 2: remove linear trend).
     */
    void onUpdateBandpowerDetrendMethod(const QString &sDetrendMethod);

    //=================================================================================================================
    /**
     * @brief updates the order of the AR for spectrum estimation.
     * @param [in] arOrder          Order of the AR method.
     */
    void onUpdateBandpowerAROrder(int arOrder);

    //=================================================================================================================
    /**
     * @brief updates the number of evaluatin points for the AR for spectrum estimation.
     * @param [in] iAREvaluationPoints      Number of AR evaluation points.
     */
    void onUpdateBandpowerAREvaluationPoints(int iAREvaluationPoints);

protected:
    //=================================================================================================================
    /**
     * @brief Inits widgets which are used to control this plugin, then emits them in form of a QList.
     */
    virtual void initPluginControlWidgets();

    //=================================================================================================================
    /**
     * @brief AbstractAlgorithm function
     */
    virtual void run();

private:
    FIFFLIB::FiffInfo::SPtr     m_pFiffInfoInput;     /**< Input fiff measurement info.*/
    FIFFLIB::FiffInfo::SPtr     m_pFiffInfoOutput;    /**< Output fiff measurement info.*/

    //QSharedPointer<BandpowerSettingsView>    m_pSettingsView; /**< The widget used to control this plugin by the user.*/

    QSharedPointer<UTILSLIB::CircularBuffer_Matrix_double>       m_pBandpowerBuffer;          /**< Holds incoming raw data. */

    SCSHAREDLIB::PluginInputData<SCMEASLIB::RealTimeMultiSampleArray>::SPtr      m_pBandpowerInput;      /**< The incoming data.*/
    SCSHAREDLIB::PluginOutputData<SCMEASLIB::RealTimeMultiSampleArray>::SPtr     m_pBandpowerOutput;     /**< The outgoing data.*/

    QMutex      m_qMutex;
    bool        m_bPluginControlWidgetsInit;

    // parameters from bandpowersettingsview
    //bool        m_bStartComputation;        // current state of the bandpower computation.
    double      m_dMaxFreq;                 // maximum value of the frequency band. (in Hz).
    double      m_dMinFreq;                 // minimum value of the frequency band. (in Hz).
    int         m_iSegmentLength;           // width of the EEG raw data segment to calculate band powers. (sample number of segment).
    int         m_iSegmentStep;             // segment moving/updating step. (sample number).
    int         m_iNumBins;                 // number of frequency bins: (m_dMaxFreq - m_dMinFreq)/m_dBinWidth.
    int         m_iNumPickedChannels;       // number of picked EEG channels to calculate band powers.
    QString     m_sDetrendMethod;           // detrend method.
    QString     m_sSpectrumMethod;          // spectrum method.

    // parameters from arsettingsview
    int         m_iAROrder;                 // value of ar order.
    int         m_iAREvaluationPoints;      // value of ar evaluation points.

    // parameters for bandpower
    bool        m_bBandpowerChNumReset;     // if bandpower channel number is to be reset, effected by pickedChannels and Bin number.
    bool        m_bBandpowerSegmentReset;   // if bandpower data block is to be reset, effected by segment length and its update step.
    double      m_dBinWidth;                // bin width. (in Hz): (m_dMaxFreq - m_dMinFreq)/m_iNumBins.
    double      m_dDataSampFreq;            // sampling frequency of the original EEG raw data.(in Hz).
    int         m_iDataBufferSize;          // buffer/sample size of the origin input data.
    int         m_iDataNumChannels;         // number of channels of the original input data.
    int         m_iDataNumEEGChs;           // number of EEG channels from the original input data.
    int         m_iBandpowerNumChannels;    // number of computed bandpower channels: m_iSelectedNumChannels*m_iNumBins.

    QString     m_sSettingsPath;            // settings path string for load/save parameters.
    QStringList m_sDataEEGChNames;          // names of EEG channels of the original input data.
    QStringList m_sPickedChNames;           // names of picked EEG channels, provided by the indices in m_sDataEEGChNames.
    Eigen::RowVectorXi m_vecEEGChPicks;     // index vector of EEG channels.

    // settings view
    //BandpowerSettingsView       *m_pBandpowerSettingsView;
    //BandpowerARSettingsView     *m_pBandpowerARSettingsView;

    // channel selection view
    //QSharedPointer<DISPLIB::ChannelSelectionView>   m_pChannelSelectionView;    /**< ChannelSelectionView. */
    //QSharedPointer<DISPLIB::ChannelInfoModel>       m_pChannelInfoModel;        /**< ChannelInfoModel. */

signals:
    //=================================================================================================================
    /**
     * @brief Emitted when fiffInfo is available
     */
    void fiffInfoAvailable();

    //=================================================================================================================
    /**
     * @brief sig_resetRTMSAChNum   emitted when one is to reset rtmsa channel number.
     * @param val
     */
    void sig_resetRTMSAChNum(const bool val);
};
} // NAMESPACE

#endif // BANDPOWER_H
