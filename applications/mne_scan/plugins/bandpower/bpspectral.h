//=====================================================================================================================
/**
 * @file     bpspectral.h
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
 * @brief    Contains the declaration of the bpspectral class.
 *
 */
//=====================================================================================================================
#ifndef BPSPECTRAL_H
#define BPSPECTRAL_H

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================
#include "bandpower_global.h"
#include <utils/spectral.h>

//=====================================================================================================================
// QT INCLUDES
//=====================================================================================================================
//#include <QString>
//#include <QPair>
//#include <QSharedPointer>

//#include <QPointer>

//=====================================================================================================================
// EIGEN INCLUDES
//=====================================================================================================================
#include <Eigen/Core>

//=====================================================================================================================
// FORWARD DECLARATIONS
//=====================================================================================================================

//=====================================================================================================================
// DEFINE NAMESPACE BANDPOWERPLUGIN
//=====================================================================================================================
namespace BANDPOWERPLUGIN {

struct ARWeightsMEMInputData {
    Eigen::RowVectorXd vecData;
    int iOrder;
};

//=====================================================================================================================
// BANDPOWERPLUGIN FORWARD DECLARATIONS
//=====================================================================================================================

//=====================================================================================================================
/**
 * DECLARE CLASS BPSpectral
 * Computes spectral measures of input data such as spectra, power spectral density, cross-spectral density.
 *
 * @brief The BPSpectral class provides a BPSpectral algorithm computing spectral measures of input data.
 */
class BANDPOWERSHARED_EXPORT BPSpectral: public UTILSLIB::Spectral
{
public:
    //=================================================================================================================
    /**
     * @brief deleted default constructor (static class).
     */
    BPSpectral() = delete;

    //=================================================================================================================
    /**
     * @brief Calculates the power spectral density of given tapered spectrum
     * @param [in] matTapSpectrum   Vector of tapered spectra, for which the PSD is calculated
     * @param [in] vecTapWeights    taper weights
     * @param [in] iNfft            FFT length
     * @param [in] dSampFreq        sampling frequency of the input data
     * @param [in] bUseMultithread  if true, use multi threads
     * @return power spectral density of a given tapered spectrum
     */
    static QVector<Eigen::VectorXd> psdFromTaperedSpectra_BP(const QVector<Eigen::MatrixXcd> &matTapSpectrum,
                                                             const Eigen::VectorXd &vecTapWeights,
                                                             int iNfft,
                                                             double dSampFreq=1.0,
                                                             bool bUseMultithread = true);

    //=================================================================================================================
    /**
     * @brief Calculates weights of autoregressive model with maximum entropy method (Burg algorithm)
     * @param [in] vecData         input row data (time domain), for which the spectrum is computed
     * @param [in] iOrder          order of model
     * @return Pair containing vector of AR weights and power
     */
    static QPair<Eigen::VectorXd, double> calculateARWeightsMEMRow(const Eigen::RowVectorXd &vecData, int iOrder);

    //=================================================================================================================
    /**
     * @brief Calculates weights of autoregressive model with maximum entropy method (Burg algorithm)
     * @param matData               input data (time domain), for which the spectrum is computed
     * @param [in] iOrder           order of model
     * @param bUseMultithread       if true, use multi threads
     * @return Pair containing vector of AR weights and power
     */
    static QVector<QPair<Eigen::VectorXd, double>> calculateARWeightsMEMMatrix(const Eigen::MatrixXd &matData, int iOrder,
                                                                               bool bUseMultithread);
    //=================================================================================================================
    /**
     * @brief Computes the ARWeights for a row vector. This function gets called in parallel.
     * @param inputData         The input data.
     * @return The tapered spectra for one data row.
     */
    static QPair<Eigen::VectorXd, double> computeAR(const ARWeightsMEMInputData& inputData);

    //=================================================================================================================
    /**
     * @brief Reduces the ARWEights results to a final result. This function gets called in parallel.
     * @param [out] finalData    The final data data.
     * @param [in]  resultData   The resulting data from the computation step.
     */
    static void reduceAR(QVector<QPair<Eigen::VectorXd, double>>& finalData, const QPair<Eigen::VectorXd, double>& resultData);

    //=================================================================================================================
    /**
     * @brief Calculates the evaluation points for spectrum computation
     * @param [in] dBottomFreq    lower end of frequency spectrum
     * @param [in] dTopFreq       upper end of frequency spectrum
     * @param [in] iBins          number of bins/evaluation points
     * @param [in] iEvalPerBin    evaluations per bin
     * @param [in] bCentered      toggle whether spectrum is evaluated at bin edges (false) or bin centers (true)
     * @return matrix of evaluation weights
     */
    static Eigen::MatrixXcd generateARSpectraWeights(double dBottomFreq, double dTopFreq, int iBins, int iEvalPerBin,
                                                     bool bCentered = false);

    //=================================================================================================================
    /**
     * @brief Calculates the power spectral density of given tapered spectrum
     * @param [in] ARweights             weights for AR model, output of calculateARweightsMEM
     * @param [in] matSpectraweights     weights for frequency spectrum calculation
     * @param [in] dSampFreq             sampling frequency of the input data
     * @return  power spectral density
     */
    static Eigen::VectorXd psdFromARSpectra(const QPair<Eigen::VectorXd, double> &ARWeights,
                                            const Eigen::MatrixXcd &matSpectraWeights,
                                            double dSampFreq);

    //=================================================================================================================
    /**
     * @brief Calculates the power spectral density of given tapered spectrum for multiple rows at once
     * @param [in] ARweights             weights for AR model, output of calculateARweightsMEM
     * @param [in] matSpectraweights     weights for frequency spectrum calculation
     * @param [in] dSampFreq             sampling frequency of the input data
     * @param [in] bUseMultithread
     * @return power spectral density
     */
    static QVector<Eigen::VectorXd> psdFromARSpectra(const QVector<QPair<Eigen::VectorXd, double>> &ARWeights,
                                                     const Eigen::MatrixXcd &matSpectraWeights,
                                                     double dSampFreq,
                                                     bool bUseMultithread);

    //=================================================================================================================
    /**
     * @brief Calculate the bandpower from the (equally spaced) spectrum entries using Simpson's rule.
     * @param [in] spectrumentries  Entries of the power spectrum.
     * @param [in] stepsize
     * @return
     */
    static double bandpowerFromSpectrumEntries(const Eigen::VectorXd &spectrumentries, double stepsize);

    //=================================================================================================================
    /**
     * @brief Calculate the bandpower from the (equally spaced) spectrum entries using Simpson's rule.
     *          for intervalls that do not match the evaluation frequencies
     * @param [in] spectrumbins
     * @param [in] spectrumentries  Entries of the power spectrum.
     * @param [in] minFreq
     * @param [in] maxFreq
     * @param [in] eps
     * @return
     */
    static double bandpowerFromSpectrumEntriesOffset(const Eigen::VectorXd &spectrumBins,
                                                     const Eigen::VectorXd &spectrumEntries,
                                                     double minFreq, double maxFreq,
                                                     double eps = std::numeric_limits<double>::epsilon());

    //=================================================================================================================
    /**
     * @brief Detrend equally spaced data.
     * @param [in] data      Data matrix.
     * @param [in] method    Detrending method (0 = none, 1 = remove mean, 2 = remove linear trend).
     * @return
     */
    static Eigen::MatrixXd detrendData(const Eigen::MatrixXd &data, const QString &sDetrendMethod);

    //=================================================================================================================
    /**
     * @brief Linear detrend equally spaced data.
     * @param [in] data      Data matrix.
     * @return
     */
    static Eigen::MatrixXd linearDetrend(const Eigen::MatrixXd &data);

private:
    //=================================================================================================================
    /**
     * @brief Calculates a hanning window of given length
     * @param [in] iSignalLength     length of the hanning window
     * @return hanning window
     */
    //static Eigen::MatrixXd hanningWindow(int iSignalLength);
};
} // NAMESPACE

#endif // BPSPECTRAL_H
