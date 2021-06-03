//=====================================================================================================================
/**
 * @file     bandpowerarsettingsview.h
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
 * @brief    Contains the declaration of the bandpowerarsettingsview class.
 *
 */
//=====================================================================================================================
#ifndef BANDPOWERARSETTINGSVIEW_H
#define BANDPOWERARSETTINGSVIEW_H

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
namespace Ui {
    class BandpowerARSettingsView;
}

//=====================================================================================================================
// DEFINE NAMESPACE BANDPOWERPLUGIN
//=====================================================================================================================
namespace BANDPOWERPLUGIN {

//=====================================================================================================================
// BANPOWERPLUGIN FORWARD DECLARATIONS
//=====================================================================================================================

//=====================================================================================================================
/**
 * DECLARE CLASS BandpowerARSettingsView
 *
 * @brief The BandpowerARSettingsView class provides the Bandpower quickControl/Settings window.
 */
class BandpowerARSettingsView : public QWidget
{
    Q_OBJECT

public:
    typedef QSharedPointer<BandpowerARSettingsView> SPtr;    /**< Shared pointer type for BandpowerARSettingsView. */
    typedef QSharedPointer<const BandpowerARSettingsView> ConstSPtr; /**< Const shared pointer type for BandpowerARSettingsView. */

    //=================================================================================================================
    /**
     * @brief Constructs a BandpowerARSettingsView.
     * @param [in] sSettingsPath
     * @param [in] parent
     */
    explicit BandpowerARSettingsView(const QString& sSettingsPath = "", QWidget *parent = 0);

    //=================================================================================================================
    /**
     * @brief Destroys the BandpowerARSettingsView.
     */
    ~BandpowerARSettingsView();

    //=================================================================================================================
    /**
     * @brief Slot called when m_qSpinBox_AROrder change.
     */
    void onSpinBoxAROrder();

    //=================================================================================================================
    /**
     * @brief Slot called when m_qSpinBox_EvaluationPoints change.
     */
    void onSpinBoxEvaluationPoints();

    //=================================================================================================================
    /**
     * @brief Slot called when m_qSpinBox_SamplingPoints change.
     */
    void onSpinBoxSamplingPoints();

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
    int         m_iAROrder;
    int         m_iEvaluationPoints;
    QString     m_sSettingsPath;    /**< The settings path to store the GUI settings to. */
    Ui::BandpowerARSettingsView*      m_pUi; /**< The UI class specified in the designer. */

signals:
    //=================================================================================================================
    /**
     * @brief Emitted whenever the settings changed and are ready to be retreived.
     * @param value
     */
    void sig_updateAROrder(int value);
    void sig_updateAREvaluationPoints(int value);

};
} // NAMESPACE

#endif // BANDPOWERARSETTINGSVIEW_H
