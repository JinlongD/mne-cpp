//=====================================================================================================================
/**
 * @file     bandpowerarsettingsview.cpp
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

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================
#include "bandpowerarsettingsview.h"
#include "ui_bandpowerarsettingsview.h"

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
BandpowerARSettingsView::BandpowerARSettingsView(const QString& sSettingsPath,
                                                 QWidget *parent)
    : QWidget(parent)
    , m_pUi(new Ui::BandpowerARSettingsView)
    , m_sSettingsPath(sSettingsPath)
{
    m_pUi->setupUi(this);

    loadSettings();

    //m_pUi->m_qSpinBox_AROrder->setMaximumWidth(100);
    m_pUi->m_qSpinBox_AROrder->setValue(m_iAROrder);
    //connect(m_pUi->m_qSpinBox_AROrder, &QSpinBox::editingFinished, this, &BandpowerARSettingsView::onSpinBoxAROrder);
    connect(m_pUi->m_qSpinBox_AROrder, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &BandpowerARSettingsView::onSpinBoxAROrder);

    m_pUi->m_qSpinBox_EvaluationPoints->setValue(m_iEvaluationPoints);
    connect(m_pUi->m_qSpinBox_EvaluationPoints, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &BandpowerARSettingsView::onSpinBoxEvaluationPoints);

    m_pUi->m_qLabel_SamplingPoints->setDisabled(true);
    m_pUi->m_qSpinBox_SamplingPoints->setDisabled(true);
}

//=====================================================================================================================
BandpowerARSettingsView::~BandpowerARSettingsView()
{
    saveSettings();

    delete m_pUi;
}

//=====================================================================================================================
void BandpowerARSettingsView::saveSettings()
{
    if(m_sSettingsPath.isEmpty()) {
        return;
    }

    // Store Settings
    QSettings settings("MNECPP");
    settings.setValue(m_sSettingsPath + QString("/arOrder"), m_iAROrder);
    settings.setValue(m_sSettingsPath + QString("/evaluationsPoints"), m_iEvaluationPoints);
}

//=====================================================================================================================
void BandpowerARSettingsView::loadSettings()
{
    if(m_sSettingsPath.isEmpty()) {
        return;
    }

    // Load Settings
    QSettings settings("MNECPP");
    m_iAROrder          = settings.value(m_sSettingsPath + QString("/arOrder"), 16).toInt(); // init/default
    m_iEvaluationPoints = settings.value(m_sSettingsPath + QString("/evaluationPoints"), 1).toInt();
}

//=====================================================================================================================
void BandpowerARSettingsView::onSpinBoxAROrder(int value)
{
    if (value < 1) {
        value = 1;
        qDebug() << "ARSettingsView::onChangeSpinBoxAROrder: "
                 << "AR order < 1 not allowed. Value has been reset to 1.";
    }

    m_iAROrder = value;

    emit sig_updateAROrder(m_iAROrder);

    saveSettings();
}

//=====================================================================================================================
void BandpowerARSettingsView::onSpinBoxEvaluationPoints(int value)
{
    m_iEvaluationPoints = value;

    emit sig_updateAREvaluationPoints(m_iEvaluationPoints);

    saveSettings();
}

//=====================================================================================================================
void BandpowerARSettingsView::onSpinBoxSamplingPoints(int value)
{
    m_iSamplingPoints = value;

    saveSettings();
}
