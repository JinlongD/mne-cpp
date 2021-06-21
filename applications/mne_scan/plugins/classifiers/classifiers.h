//=====================================================================================================================
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
#include <QThread>

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

private:
    //=====================================================================================================================
    // IAlgorithm members
    FIFFLIB::FiffInfo::SPtr         m_pFiffInfo;                /**< Fiff measurement info.*/
    QSharedPointer<UTILSLIB::CircularBuffer_Matrix_double>      m_pCircularBuffer;              /**< Holds incoming raw data. */
    SCSHAREDLIB::PluginInputData<SCMEASLIB::RealTimeMultiSampleArray>::SPtr      m_pInput;      /**< The incoming data.*/
    SCSHAREDLIB::PluginOutputData<SCMEASLIB::RealTimeMultiSampleArray>::SPtr     m_pOutput;     /**< The outgoing data.*/

    QMutex              m_qMutex;
    bool                m_bPluginControlWidgetsInit;
    QString             m_sSettingsPath;            // settings path string for load/save parameters.

public:
    //=====================================================================================================================
    // Add your public method functions here.
    //=====================================================================================================================
    /**
     * @brief setMatByteArray
     * @param byteArray
     */
    void setMatByteArray(const QByteArray &byteArray);

    //=====================================================================================================================
    /**
     * @brief parsingMat
     */
    void parsingMat();

    //=====================================================================================================================
    /**
     * @brief onClassifiersChanged
     * @param index
     */
    void onClassifiersChanged(int classifierIndex);

    //=====================================================================================================================
    /**
     * @brief onTriggerThresholdChanged
     * @param threshold
     */
    void onTriggerThresholdChanged(int triggerThreshold);

    //=====================================================================================================================
    /**
     * @brief onTriggerClassChanged
     * @param currentClass
     */
    void onTriggerClassChanged(int triggerClass);

    //=====================================================================================================================
    // Add your public members here.
    //=====================================================================================================================
    bool                m_bIsClassifiersInit;
    QString             m_sClassifiersInfo;
    QString             m_sFullFileName;

private:
    //=====================================================================================================================
    // Add your private method functions here.
    //=====================================================================================================================
    /**
     * @brief parsingIsFinished
     */
    void onIsParsingFinished();

    //=====================================================================================================================
    /**
     * @brief predictLDA
     * @param matFeature
     * @return
     */
    inline bool predictLDA(const Eigen::MatrixXd &matFeature, const Eigen::MatrixXd &matWeight, const Eigen::MatrixXd &vecBias) {
        Eigen::VectorXd vec;
        Eigen::VectorXd::Index maxIndex;
        vec = matWeight * matFeature + vecBias;
        vec.maxCoeff(&maxIndex);
        if (maxIndex != m_iTriggerClass) {
            return false;
        } else {
            return true;
        }
    }

    //=====================================================================================================================
    /**
     * @brief predictFDA
     * @param matFeature
     * @return
     */
    inline bool predictFDA(const Eigen::MatrixXd &matFeature, const Eigen::MatrixXd &matWeight,
                           const Eigen::VectorXd &matMean, const qint32 &classNum) {
        Eigen::VectorXd vec(classNum);
        Eigen::VectorXd::Index minIndex;
        Eigen::VectorXd vecProj = matWeight * matFeature; // Nc-by-1: (Nc-by-Nd) * (Nd-by-1)
        Eigen::VectorXd vecTemp;
        for (int i = 0; i < classNum; ++i) {
            vecTemp = vecProj - matMean.col(i);
            vec(i) = vecTemp.dot(vecTemp);
        }
        vec.minCoeff(&minIndex);
        if (minIndex != m_iTriggerClass) {
            return false;
        } else {
            return true;
        }
    }

    //=====================================================================================================================
    // Add your private members here.
    //=====================================================================================================================
    // parameters from ClassifiersSettingsView
    qint8               m_iCurrentClassifier;
    qint8               m_iTriggerThreshold;
    int                 m_iTriggerClass;


    // parameters from ClassifiersSetupWidget
    QThread*            m_pParserThread;
    MatParser*          m_pMatParser;

signals:
    //=====================================================================================================================
    // Add your signals here.
    //=====================================================================================================================
    /**
     * @brief Emitted when fiffInfo is available
     */
    void fiffInfoAvailable();

    //=====================================================================================================================
    /**
     * @brief sig_getClassifiersFromMat
     */
    void sig_getClassifiersFromMat();

    //=====================================================================================================================
    /**
     * @brief sig_updateClassifiersInfo
     */
    void sig_updateClassifiersInfo(const QString &info);
};
} // NAMESPACE

#endif // CLASSIFIERS_H
