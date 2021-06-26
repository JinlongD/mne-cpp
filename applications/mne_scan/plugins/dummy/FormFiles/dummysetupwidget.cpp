﻿//=====================================================================================================================
/**
 * @file     dummysetupwidget.cpp
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
 * @brief    Definition of the dummysetupwidget class.
 *
 */
//=====================================================================================================================

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================
#include "dummysetupwidget.h"
#include "ui_dummysetupwidget.h"

#include "dummy.h"

//=====================================================================================================================
// QT INCLUDES
//=====================================================================================================================
#include <QSettings>
#include <QDebug>

//=====================================================================================================================
// USED NAMESPACES
//=====================================================================================================================
using namespace DUMMYPLUGIN;

//=====================================================================================================================
// DEFINE MEMBER METHODS
//=====================================================================================================================
DummySetupWidget::DummySetupWidget(Dummy* pdummy, const QString& sSettingsPath, QWidget* parent)
    : QWidget(parent)
    , m_pDummy(pdummy)
    , m_pUi(new Ui::DummySetupWidget)
    , m_sSettingsPath(sSettingsPath)
{
    m_pUi->setupUi(this);
}

//=====================================================================================================================
DummySetupWidget::~DummySetupWidget()
{
}

//=====================================================================================================================
void DummySetupWidget::saveSettings()
{
    if(m_sSettingsPath.isEmpty()) {
        return;
    }

    //QSettings settings("MNECPP");
    //settings.setValue(m_sSettingsPath + QString("/valueName"), m_pUi->m_pDoubleSpinBox_dummy->value());
}

//=====================================================================================================================
void DummySetupWidget::loadSettings()
{
    if(m_sSettingsPath.isEmpty()) {
        return;
    }

    //QSettings settings("MNECPP");
    //m_pUi->m_pDoubleSpinBox_dummy->setValue(settings.value(m_sSettingsPath + QString("/valueName"), 10).toInt());
}