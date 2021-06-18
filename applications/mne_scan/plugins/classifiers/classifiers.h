﻿//=====================================================================================================================
/**
 * @file     classifiers.h
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
 * @brief    Contains the declaration of the classifiers class.
 *
 */
//=====================================================================================================================
#ifndef CLASSIFIERS_H
#define CLASSIFIERS_H

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================
#include "classifiers_global.h"

#include <scShared/Plugins/abstractalgorithm.h>
#include <scMeas/realtimemultisamplearray.h>
#include <utils/generics/circularbuffer.h>

//=====================================================================================================================
// QT INCLUDES
//=====================================================================================================================
#include <QtWidgets>
#include <QtCore/QtPlugin>

//=====================================================================================================================
// EIGEN INCLUDES
//=====================================================================================================================
//#include <Eigen/Core>
#include <Eigen/SparseCore>

//=====================================================================================================================
// FORWARD DECLARATIONS
//=====================================================================================================================

//=====================================================================================================================
// DEFINE NAMESPACE CLASSIFIERSPLUGIN
//=====================================================================================================================
namespace CLASSIFIERSPLUGIN
{

//=====================================================================================================================
// CLASSIFIERSPLUGIN FORWARD DECLARATIONS
//=====================================================================================================================
class MatParser;

//=====================================================================================================================
/**
 * @brief The paramLDA struct   parameters for Linear Discriminant Analysis (LDA) classifier.
 */
struct paramLDA {
    Eigen::MatrixXd         matWeight;      // weight matrix (linear)
    Eigen::VectorXd         vecBias;        // bias vector
    QStringList             sClassNames;    // class names
    qint32                  iClassNum;      // number of classes
};

//=====================================================================================================================
/**
 * @brief The paramFDA struct   parameters for Fisher's Discriminant Analysis (FDA) classifier.
 */
struct paramFDA {
    Eigen::MatrixXd         matWeight;      // weight matrix
    Eigen::MatrixXd         vecMeanProj;    // projected mean vectors (columns)
    QStringList             sClassNames;    // class names
    qint32                  iClassNum;      // number of classes
};

//=====================================================================================================================
/**
 * @brief The paramMD struct    parameters for Mahalanobis Distance (MD) based classifier.
 */
struct paramMD {
    QList<Eigen::MatrixXd>  matCovariance;  // weight matrix
    Eigen::MatrixXd         vecMean;        // mean vectors (columns)
    QStringList             sClassNames;    // class names
    qint32                  iClassNum;      // number of classes
};

//=====================================================================================================================
/**
 * @brief The paramQDA struct   parameters for Quadratic Discriminant Analysis (QDA) classifier.
 */
struct paramQDA {
    QList<Eigen::MatrixXd>  matWeightQuad;  // quadratic weight matrix
    Eigen::MatrixXd         matWeight;      // linear weight matrix
    Eigen::VectorXd         vecBias;        // bias vector
    QStringList             sClassNames;    // class names
    qint32                  iClassNum;      // number of classes
};

//=====================================================================================================================
/**
 * DECLARE CLASS Classifiers
 *
 * @brief The Classifiers class provides classification algorithms.
 */
class CLASSIFIERSSHARED_EXPORT Classifiers : public SCSHAREDLIB::AbstractAlgorithm
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "scsharedlib/1.0" FILE "classifiers.json") //New Qt5 Plugin system replaces Q_EXPORT_PLUGIN2 macro
    // Use the Q_INTERFACES() macro to tell Qt's meta-object system about the interfaces
    Q_INTERFACES(SCSHAREDLIB::AbstractAlgorithm)

public:
    //=====================================================================================================================
    /**
     * @brief Classifiers       Constructs a Classifiers.
     */
    Classifiers();

    //=====================================================================================================================
    /**
     * @brief ~Classifiers      Destroys the Classifiers.
     */
    ~Classifiers();

    //=====================================================================================================================
    /**
     * @brief IAlgorithm functions
     */
    virtual QSharedPointer<SCSHAREDLIB::AbstractPlugin> clone() const;
    virtual void init();
    virtual void unload();
    virtual bool start();
    virtual bool stop();
    virtual SCSHAREDLIB::AbstractPlugin::PluginType getType() const;
    virtual QString getName() const;
    virtual QWidget* setupWidget();

    //=====================================================================================================================
    /**
     * @brief update                Updates the pugin with new (incoming) data.
     * @param [in] pMeasurement     The incoming data in form of a generalized Measurement.
     */
    void update(SCMEASLIB::Measurement::SPtr pMeasurement);

    //=====================================================================================================================
    /**
     * @brief getMatParser
     * @return
     */
    MatParser* getMatParser();

protected:
    //=====================================================================================================================
    /**
     * @brief Inits widgets which are used to control this plugin, then emits them in form of a QList.
     */
    virtual void initPluginControlWidgets();

    //=====================================================================================================================
    /**
     * @brief IAlgorithm function
     */
    virtual void run();

public:
    //=====================================================================================================================
    // Add your public method functions/members here.
    //=====================================================================================================================

    //=====================================================================================================================

private:
    //=====================================================================================================================
    // Add your private method functions/members here.
    //=====================================================================================================================

    //=====================================================================================================================
    // IAlgorithm members
    FIFFLIB::FiffInfo::SPtr         m_pFiffInfo;                /**< Fiff measurement info.*/
    QSharedPointer<UTILSLIB::CircularBuffer_Matrix_double>      m_pCircularBuffer;              /**< Holds incoming raw data. */
    SCSHAREDLIB::PluginInputData<SCMEASLIB::RealTimeMultiSampleArray>::SPtr      m_pInput;      /**< The incoming data.*/
    SCSHAREDLIB::PluginOutputData<SCMEASLIB::RealTimeMultiSampleArray>::SPtr     m_pOutput;     /**< The outgoing data.*/

    QMutex              m_qMutex;
    bool                m_bPluginControlWidgetsInit;

    // parameters from ClassifiersSettingsView
    bool                m_bChNumReset;
    double              m_dDataSampFreq;
    int                 m_iDataBufferSize;
    int                 m_iNumPickedCh;
    Eigen::RowVectorXi  m_vecEEGChPicks;            // index vector of EEG channels.
    QStringList         m_sEEGChNames;
    QStringList         m_sPickedChNames;

    // parameters from ClassifiersSetupWidget
    MatParser*          m_pMatParser;
    paramFDA            m_classifierFDA;
    paramLDA            m_classifierLDA;
    paramQDA            m_classifierQDA;
    paramMD             m_classifierMD;

    // parameters for classifiers
    QString             m_sSettingsPath;            // settings path string for load/save parameters.

signals:
    //=====================================================================================================================
    // Add your signals here.
    //=====================================================================================================================
    //=====================================================================================================================
    /**
     * @brief Emitted when fiffInfo is available
     */
    void fiffInfoAvailable();
};
} // NAMESPACE

#endif // CLASSIFIERS_H
