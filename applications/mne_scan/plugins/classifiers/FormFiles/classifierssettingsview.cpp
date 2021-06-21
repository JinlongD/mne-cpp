//=====================================================================================================================
/**
 * @file     classifierssettingsview.cpp
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
 * @brief    Definition of the classifierssettingsview class.
 *
 */
//=====================================================================================================================

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================
#include "classifierssettingsview.h"
#include "ui_classifierssettingsview.h"

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
using namespace CLASSIFIERSPLUGIN;

//=====================================================================================================================
// DEFINE MEMBER METHODS
//=====================================================================================================================
ClassifiersSettingsView::ClassifiersSettingsView(const QStringList &classifierNames, const QStringList &classNames,
                                                 const int &threshold, const QString &sSettingsPath, QWidget *parent)
    : QWidget(parent)
    , m_pUi(new Ui::ClassifiersSettingsView)
    , m_sSettingsPath(sSettingsPath)
{
    m_pUi->setupUi(this);

    //m_pUi->m_qComboBox_Classifiers->clear();
    m_pUi->m_qComboBox_Classifiers->addItems(classifierNames);
    //m_pUi->m_qComboBox_Classifiers->setCurrentIndex(1);

    //m_pUi->m_qComboBox_TriggerClass->clear();
    m_pUi->m_qComboBox_TriggerClass->addItems(classNames);
    //m_pUi->m_qComboBox_TriggerClass->setCurrentIndex(1);

    m_pUi->m_qSpinBox_TriggerThreshold->setValue(threshold);

    connect(m_pUi->m_qComboBox_Classifiers, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ClassifiersSettingsView::onComboBoxClassifiersChanged);
    connect(m_pUi->m_qSpinBox_TriggerThreshold, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ClassifiersSettingsView::onSpinBoxTriggerThresholdChanged);
    connect(m_pUi->m_qComboBox_TriggerClass, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ClassifiersSettingsView::onComboBoxTriggerClassChanged);

    loadSettings();
}

//=====================================================================================================================
ClassifiersSettingsView::~ClassifiersSettingsView()
{
    saveSettings();

    delete m_pUi;
}

//=====================================================================================================================
void ClassifiersSettingsView::saveSettings()
{
    if(m_sSettingsPath.isEmpty()) {
        return;
    }

    QSettings settings("MNECPP");

    settings.setValue(m_sSettingsPath + QString("/classifierIndex"), m_pUi->m_qComboBox_Classifiers->currentIndex());
    settings.setValue(m_sSettingsPath + QString("/triggerThreshold"), m_pUi->m_qSpinBox_TriggerThreshold->value());
    settings.setValue(m_sSettingsPath + QString("/triggerClass"), m_pUi->m_qComboBox_TriggerClass->currentIndex());
}

//=====================================================================================================================
void ClassifiersSettingsView::loadSettings()
{
    if(m_sSettingsPath.isEmpty()) {
        return;
    }

    QSettings settings("MNECPP");

    m_pUi->m_qComboBox_Classifiers->setCurrentIndex(settings.value(m_sSettingsPath + QString("/classifierIndex"), 0).toInt());
    m_pUi->m_qSpinBox_TriggerThreshold->setValue(settings.value(m_sSettingsPath + QString("/triggerThreshold"), 3).toInt());
    m_pUi->m_qComboBox_TriggerClass->setCurrentIndex(settings.value(m_sSettingsPath + QString("/triggerClass"), 1).toInt());
}

//=====================================================================================================================
void ClassifiersSettingsView::onComboBoxClassifiersChanged(int classifierIndex)
{
    emit sig_updateClassifiers(classifierIndex);
}

//=====================================================================================================================
void ClassifiersSettingsView::onSpinBoxTriggerThresholdChanged(int triggerThreshold)
{
    emit sig_updateTriggerThreshold(triggerThreshold);
}

//=====================================================================================================================
void ClassifiersSettingsView::onComboBoxTriggerClassChanged(int triggerClass)
{
    emit sig_updateTriggerClass(triggerClass);
}
