//=====================================================================================================================
/**
 * @file     bpspectral.cpp
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
 * @brief    Definition of the bpspectral class.
 *
 */
//=====================================================================================================================

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================
#include "bpspectral.h"
#include "math.h"

//=====================================================================================================================
// QT INCLUDES
//=====================================================================================================================
#include <QDebug>
#include <QtMath>
#include <QtConcurrent>
#include <QVector>

//=====================================================================================================================
// EIGEN INCLUDES
//=====================================================================================================================
#include <unsupported/Eigen/FFT>

//=====================================================================================================================
// USED NAMESPACES
//=====================================================================================================================
using namespace BANDPOWERPLUGIN;
//using namespace UTILSLIB;
//using namespace Eigen;

//=====================================================================================================================
// DEFINE MEMBER METHODS
//=====================================================================================================================
QVector<Eigen::VectorXd> BPSpectral::psdFromTaperedSpectra_BP(const QVector<Eigen::MatrixXcd> &matTapSpectrum,
                                                              const Eigen::VectorXd &vecTapWeights,
                                                              int iNfft,
                                                              double dSampFreq,
                                                              bool bUseMultithread)
{
#ifdef EIGEN_FFTW_DEFAULT
    fftw_make_planner_thread_safe();
#endif

    QVector<Eigen::VectorXd> finalResult;
    Eigen::RowVectorXd tmpResult;

    if(!bUseMultithread)
        for(int i=0; i< matTapSpectrum.size(); i++)
        {
            tmpResult = psdFromTaperedSpectra(matTapSpectrum.at(i), vecTapWeights, iNfft, dSampFreq);
            finalResult.append(tmpResult.transpose());
        }
    else
    {
        std::function<Eigen::VectorXd(const Eigen::MatrixXcd &)> loccompute =
                [&vecTapWeights, iNfft, dSampFreq](const Eigen::MatrixXcd & locTapSpectrum) {
            return psdFromTaperedSpectra(locTapSpectrum, vecTapWeights, iNfft, dSampFreq).transpose();
        };
        QFuture<Eigen::VectorXd> result = QtConcurrent::mapped(matTapSpectrum, loccompute);
        result.waitForFinished();
        finalResult = result.results().toVector();
    }

    return finalResult;
}

//=====================================================================================================================
QPair<Eigen::VectorXd, double> BPSpectral::calculateARWeightsMEMRow(const Eigen::RowVectorXd &vecData, int iOrder)
{
    int n = vecData.size();

    if (n < iOrder)
        qCritical() << "Error Occured in Spectral::calculateARweightssMEM: Number of data samples smaller than order of regression! ";

    Eigen::VectorXd coeffs = Eigen::VectorXd::Zero(iOrder + 1);
    Eigen::VectorXd vecf = vecData;
    Eigen::VectorXd vecb = vecData;
    Eigen::VectorXd vecwkm = Eigen::VectorXd::Zero(coeffs.size());

    double dE = vecf.dot(vecf);
    double dDen = 2.0*dE;

    dE /= static_cast<double>(n);

    double dNum = 0.0;
    double dq = 1.0;
    coeffs[0] = 1.0;

    for (int k=1; k <= iOrder; k++)
    {
        dNum = 0.0;

        for (int j=0; j < (n-k); j++) // mit dot product ersetzen
            dNum += vecf[j+1]*vecb[j];
        //dNum = vecf.segment(1,n-k).dot(vecb.segment(0,n-k));

        dDen = dq*dDen - vecf[0]*vecf[0] - vecb[n-k]*vecb[n-k]; // is this correct?

        coeffs[k] = -2.0*dNum/dDen;
        dq = 1.0 - coeffs[k]*coeffs[k];
        dE *= dq;

        for (int i=1; i<k; i++)
            coeffs[i] = vecwkm[i] + coeffs[k]*vecwkm[k-i]; // check +-
        //coeffs.segment(1,k-1) = vecwkm.segment(1,k-1) + coeffs[k]*vecwkm.segment(1,k-1).reverse();

        if (k < iOrder)
        {
            for (int i=1; i<=k; i++)
                vecwkm[i] = coeffs[i];
            //vecwkm.segment(1,k) = coeffs.segment(1,k);

            for (int j=0; j<n-k; j++) // evtl. mit eigen, braucht aber zwischenspeicher für vecf
            {
                vecf[j] = vecf[j+1] + vecwkm[k]*vecb[j]; // check +-
                vecb[j] = vecb[j] + vecwkm[k]*vecf[j+1]; // check +-
            }
        }

        // check indices!!!
    }

    //for (int i=1; i<=iOrder; i++)
    //    coeffs[i] *= -1; // check +-

    QPair<Eigen::VectorXd, double> out;

    out.first = coeffs;
    out.second = dE;

    return out;
}

//=====================================================================================================================
QVector<QPair<Eigen::VectorXd, double>> BPSpectral::calculateARWeightsMEMMatrix(const Eigen::MatrixXd &matData,
                                                                                int iOrder, bool bUseMultithread)
{
#ifdef EIGEN_FFTW_DEFAULT
    fftw_make_planner_thread_safe();
#endif

    QVector<QPair<Eigen::VectorXd, double>> finalResult;
    QPair<Eigen::VectorXd, double> tmpResult;

    if(!bUseMultithread)
    {
        for (int i=0; i<matData.rows(); i++)
        {
            tmpResult = calculateARWeightsMEMRow(matData.row(i),iOrder);
            finalResult.append(tmpResult);
        }
    }
    else
    {
        QVector<ARWeightsMEMInputData> lData;
        ARWeightsMEMInputData tempData;
        tempData.iOrder = iOrder;

        for (int i=0; i<matData.rows(); i++)
        {
            tempData.vecData = matData.row(i);
            lData.append(tempData);
        }

        QFuture<QPair<Eigen::VectorXd, double>> result = QtConcurrent::mapped(lData, computeAR);
        result.waitForFinished();
        finalResult = result.results().toVector();
    }
    return finalResult;
}

//=====================================================================================================================
QPair<Eigen::VectorXd, double> BPSpectral::computeAR(const ARWeightsMEMInputData& inputData)
{
    //qDebug() << "Spectral::compute";
    return calculateARWeightsMEMRow(inputData.vecData,
                                    inputData.iOrder);
}

//=====================================================================================================================
/*void Spectral::reduceAR(QVector<QPair<Eigen::VectorXd, double>>& finalData,
                      const QPair<Eigen::VectorXd, double>& resultData)
{
    //qDebug() << "Spectral::reduce";
    finalData.append(resultData);
}*/

//=====================================================================================================================
Eigen::MatrixXcd BPSpectral::generateARSpectraWeights(double dBottomFreq, double dTopFreq, int iBins, int iEvalPerBin,
                                                      bool bCentered)
{
    if (dTopFreq < dBottomFreq)
        qCritical() << "Error Occured in Spectral::generateARSpectraWeights: Bottom frequency has to be smaller than top frequency! ";

    double twopi = 2*M_PI;
    double delta, tempfreq;

    if (bCentered)
    {
        delta = (dTopFreq - dBottomFreq) / (iBins * iEvalPerBin);
        tempfreq = twopi * (dBottomFreq + delta/2);
    }
    else
    {
        delta = (dTopFreq - dBottomFreq) / ((iBins - 1) * iEvalPerBin);
        tempfreq = twopi * (dBottomFreq - (iEvalPerBin - 1)*delta/2);
    }

    double deltapi = twopi * delta;

    Eigen::MatrixXcd mat_Theta = Eigen::MatrixXcd::Zero(iBins,iEvalPerBin);

    for (int i=0; i<iBins; i++)
    {
        for (int j=0; j<iEvalPerBin; j++)
        {
            mat_Theta(i,j) = std::polar<double>(1.0,tempfreq);
            tempfreq += deltapi;
        }
    }

    return mat_Theta;
}

//=====================================================================================================================
Eigen::VectorXd BPSpectral::psdFromARSpectra(const QPair<Eigen::VectorXd, double> &ARWeights,
                                             const Eigen::MatrixXcd &matSpectraWeights,  double dSampFreq)
{
    Eigen::VectorXd coeffs = ARWeights.first;
    double dNum = ARWeights.second;

    Eigen::MatrixXcd matSpectraWeightsWrk = matSpectraWeights;

    Eigen::MatrixXcd dDen = Eigen::MatrixXcd::Ones(matSpectraWeights.rows(),matSpectraWeights.cols());

    for (int i=1; i<coeffs.size(); i++) // coeff[0] is one and already in dDen
    {
        dDen += coeffs(i)*matSpectraWeightsWrk;
        if (i<coeffs.size()-1)
            matSpectraWeightsWrk = matSpectraWeightsWrk.cwiseProduct(matSpectraWeights);
    }

    Eigen::MatrixXd SpectraBinned;

    SpectraBinned.array() = (2.0 * dNum) / (dDen.real().array().pow(2) + dDen.imag().array().pow(2));
    // Multiply by 2 to account for symmetry of spectrum

    /*    if(SpectraBinned.array().isNaN().any())
        for(int i=0; i < SpectraBinned.rows(); i++)
            for(int j=0; j< SpectraBinned.cols(); j++)
                if(std::isnan(SpectraBinned(i,j))){
                    if(std::abs(dNum) < std::numeric_limits<double>::epsilon()){
                        SpectraBinned(i,j) = 0;
                        qDebug() << "[Spectral::psdFromARSpectra] Corrected NaN to 0";
                    }
                    else
                        qDebug() << "[Spectral::psdFromARSpectra] Value is NaN. Value of dNum is" << dNum;
                }*/


    Eigen::VectorXd vecSpectraOut = SpectraBinned.rowwise().mean() / dSampFreq; // Take mean over evaluations in bin

    return vecSpectraOut;
}

//=====================================================================================================================
QVector<Eigen::VectorXd> BPSpectral::psdFromARSpectra(const QVector<QPair<Eigen::VectorXd, double>> &ARWeights,
                                                      const Eigen::MatrixXcd &matSpectraWeights,  double dSampFreq,
                                                      bool bUseMultithread)
{
#ifdef EIGEN_FFTW_DEFAULT
    fftw_make_planner_thread_safe();
#endif

    QVector<Eigen::VectorXd> finalResult;
    Eigen::VectorXd tmpResult;

    if(!bUseMultithread)
        for(int i=0; i< ARWeights.size(); i++)
        {
            tmpResult = psdFromARSpectra(ARWeights.at(i),matSpectraWeights, dSampFreq);
            finalResult.append(tmpResult);
        }
    else
    {
        std::function<Eigen::VectorXd(const QPair<Eigen::VectorXd,double> &)> loccompute =
                [&matSpectraWeights, dSampFreq](const QPair<Eigen::VectorXd,double> & locARWeights) {
            return psdFromARSpectra(locARWeights,matSpectraWeights,dSampFreq);
        };
        QFuture<Eigen::VectorXd> result = QtConcurrent::mapped(ARWeights, loccompute);
        result.waitForFinished();
        finalResult = result.results().toVector();
    }

    return finalResult;
}

//=====================================================================================================================
double BPSpectral::bandpowerFromSpectrumEntries(const Eigen::VectorXd &spectrumentries, double stepsize)
{
    double power;

    if (spectrumentries.size() % 2 == 0){
        Eigen::RowVectorXd weights = Eigen::RowVectorXd::Ones(spectrumentries.size());
        for (int i=0; i<spectrumentries.size()/2; i++)
            weights(2*i+1) = 4.0;
        for (int i=0; i<spectrumentries.size()/2 - 1; i++)
            weights(2*i+2) = 2.0;
        power =  weights * spectrumentries;
        power *= (stepsize / 3.0);
    } else {
        // Use Simpson's rule for n-1 entries and trapezoid rule for the last intervall and average over 2 cases
        Eigen::RowVectorXd weights = Eigen::RowVectorXd::Ones(spectrumentries.size() - 1);
        for (int i=0; i<spectrumentries.size()/2; i++)
            weights(2*i+1) = 4.0;
        for (int i=0; i<spectrumentries.size()/2 - 1; i++)
            weights(2*i+2) = 2.0;
        power = weights * spectrumentries.head(spectrumentries.size()-1);
        power = weights * spectrumentries.tail(spectrumentries.size()-1);
        power += stepsize/2.0 * (spectrumentries(0) + spectrumentries(1) + spectrumentries(spectrumentries.size()-2)
                                 + spectrumentries(spectrumentries.size()-1));
        power *= 0.5;
    }

    return power;
}

//=====================================================================================================================
double BPSpectral::bandpowerFromSpectrumEntriesOffset(const Eigen::VectorXd &spectrumbins,
                                                      const Eigen::VectorXd &spectrumentries,
                                                      double minFreq, double maxFreq, double eps)
{
    if ((minFreq < spectrumbins(0)) || (maxFreq > spectrumbins(spectrumbins.size()-1)))
    {
        qWarning() << "[BandPower::bandpowerFromSpectrumEntries] minFreq or maxFreq out of Spectrum";
    }

    double stepsize = spectrumbins(1) - spectrumbins(0);

    int minIndex, maxIndex;
    double deltaMin, deltaMax;

    Eigen::VectorXd spectrumbinsMin = spectrumbins;
    spectrumbinsMin.array() -= minFreq;
    spectrumbinsMin.cwiseAbs().minCoeff(&minIndex);
    deltaMin = minFreq - spectrumbins(minIndex);

    Eigen::VectorXd spectrumbinsMax = spectrumbins;
    spectrumbinsMax.array() -= maxFreq;
    spectrumbinsMax.cwiseAbs().minCoeff(&maxIndex);
    deltaMax = maxFreq - spectrumbins(maxIndex);

    if (std::abs(deltaMin) < eps && std::abs(deltaMax) < eps)
        // everything fits perfectly, nothing to do here
        return bandpowerFromSpectrumEntries(spectrumentries,stepsize);
    else
    {
        if (deltaMin > 0)
            minIndex++;
        else
            deltaMin *= -1.0;

        if (deltaMax < 0)
        {
            maxIndex--;
            deltaMax *= -1.0;
        }
        double result = 0;

        int diffIndex = maxIndex - minIndex;

        /*if (diffIndex == 0)
          // minFreq and maxFreq in the same intervall (should not happen) apply trapezoid rule with linear interpolation
        {
            result = ((spectrumentries(minIndex) + (spectrumentries(minIndex-1)-spectrumentries(minIndex))*deltaMin/stepsize)
                      + (spectrumentries(maxIndex)
                         + (spectrumentries(maxIndex+1)-spectrumentries(maxIndex))*deltaMax/stepsize))*(maxFreq-minFreq)/2.0;
        } else
        {*/
        // add the contributions from the incomplete bins
        if (minIndex > 0)
            result += (spectrumentries(minIndex)
                       + (spectrumentries(minIndex-1)-spectrumentries(minIndex))*deltaMin/stepsize)*deltaMin/2.0;

        if (maxIndex < spectrumentries.size() - 1)
            result += (spectrumentries(maxIndex)
                       + (spectrumentries(maxIndex+1)-spectrumentries(maxIndex))*deltaMax/stepsize)*deltaMax/2.0;

        if (diffIndex > 1) // use Simpsons's rule for more than two points
            result += bandpowerFromSpectrumEntries(spectrumentries.segment(minIndex, maxIndex - minIndex + 1), stepsize);
        else if (diffIndex > 0) // use trapezoid rule for two points
            result += (spectrumentries(minIndex) + spectrumentries(maxIndex))*stepsize/2.0;
        //        }

        return result;
    }
}

//=====================================================================================================================
Eigen::MatrixXd BPSpectral::detrendData(const Eigen::MatrixXd &data, const QString &sDetrendMethod)
{
    if (sDetrendMethod == "None") {
        return data;
    } else if (sDetrendMethod == " Mean") {
        Eigen::MatrixXd outputData = data.colwise() - data.rowwise().mean();
        return outputData;
    } else if (sDetrendMethod == "Linear") {
        Eigen::MatrixXd outputData = linearDetrend(data);
        return outputData;
    } else {
        qDebug() << "[Spectral::detrendData] unknown detrend method " << sDetrendMethod << " Returning data";
        return data;
    }
}

//=====================================================================================================================
Eigen::MatrixXd BPSpectral::linearDetrend(const Eigen::MatrixXd &data)
{
    int n = data.cols();

    Eigen::VectorXd linspaced = Eigen::VectorXd::LinSpaced(n,0,n-1);

    Eigen::VectorXd xy = data * linspaced;

    double  linspaced_sum = n * (n - 1) / 2,
            linspaced_sum2 = (n - 1) * n * (2 * n - 1) / 6;

    Eigen::VectorXd data_rowwise_sum = data.rowwise().sum();

    double denom_a = n*linspaced_sum2-linspaced_sum*linspaced_sum;

    Eigen::VectorXd a = (n*xy - linspaced_sum*data_rowwise_sum)/denom_a;
    Eigen::VectorXd b = (data_rowwise_sum - linspaced_sum*a)/n;

    Eigen::MatrixXd data_noav = data.colwise() - b;
    data_noav -= a*linspaced.transpose();

    return data_noav;
}
