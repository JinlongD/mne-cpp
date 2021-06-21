//=====================================================================================================================
/**
 * @file     classifierssetupwidget.cpp
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
 * @brief    Definition of the classifierssetupwidget class.
 *
 */
//=====================================================================================================================

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================
#include "classifierssetupwidget.h"
#include "ui_classifierssetupwidget.h"

#include "../classifiers.h"

//=====================================================================================================================
// QT INCLUDES
//=====================================================================================================================
#include <QtWidgets>
#include <QSettings>
#include <QDebug>

//=====================================================================================================================
// USED NAMESPACES
//=====================================================================================================================
using namespace CLASSIFIERSPLUGIN;

//=====================================================================================================================
// DEFINE MEMBER METHODS
//=====================================================================================================================
ClassifiersSetupWidget::ClassifiersSetupWidget(Classifiers* pclassifiers, QWidget* parent)
    : QWidget(parent)
    , m_pClassifiers(pclassifiers)
    , m_pUi(new Ui::ClassifiersSetupWidget)
    //, m_sSettingsPath(sSettingsPath)
{
    m_pUi->setupUi(this);
    connect(m_pUi->m_qPushButton_LoadClassifier, &QPushButton::clicked,
            this, &ClassifiersSetupWidget::onPushbuttonLoadTrainedClassifier);
    connect(m_pUi->m_qPushButton_LoadFeature, &QPushButton::clicked,
            this, &ClassifiersSetupWidget::onPushbuttonLoadFeatureData);
    connect(m_pUi->m_qPushButton_LoadRaw, &QPushButton::clicked,
            this, &ClassifiersSetupWidget::onPushbuttonLoadRawData);
    connect(m_pUi->m_qPushButton_TrainFeature, &QPushButton::clicked,
            this, &ClassifiersSetupWidget::onPushbuttonTrainFromFeature);
    connect(m_pUi->m_qPushButton_TrainRaw, &QPushButton::clicked,
            this, &ClassifiersSetupWidget::onPushbuttonTrainFromRaw);

    connect(m_pClassifiers, &Classifiers::sig_updateClassifiersInfo,
            this, &ClassifiersSetupWidget::onUpdateClassifiersInfo);

    if (m_pClassifiers->m_bIsClassifiersInit) {
        m_pUi->m_qLabel_OpenedClassifier->setText(m_pClassifiers->m_sFullFileName);
        m_pUi->m_qTextBrowser_ClassifiersInfo->setPlainText(m_pClassifiers->m_sClassifiersInfo);
    }
}

//=====================================================================================================================
ClassifiersSetupWidget::~ClassifiersSetupWidget()
{
}

//=====================================================================================================================
void ClassifiersSetupWidget::saveSettings()
{
    if(m_sSettingsPath.isEmpty()) {
        return;
    }

    QSettings settings("MNECPP");

    //settings.setValue(m_sSettingsPath + QString("/valueName"), m_pUi->m_pDoubleSpinBox_dummy->value());
}

//=====================================================================================================================
void ClassifiersSetupWidget::loadSettings()
{
    if(m_sSettingsPath.isEmpty()) {
        return;
    }

    QSettings settings("MNECPP");

    //m_pUi->m_pDoubleSpinBox_dummy->setValue(settings.value(m_sSettingsPath + QString("/valueName"), 10).toInt());
}

//=====================================================================================================================
void ClassifiersSetupWidget::onPushbuttonLoadTrainedClassifier()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load trained classifier"), "./MNE-sample-data",
                                                    tr("All (*.*);; %1;; %2;; %3;; %4;; %5")
                                                    .arg("Mat files (*.mat)")
                                                    .arg("fiff raw (*.fiff)")
                                                    .arg("Text files (*.txt)")
                                                    .arg("Json files (*json)")
                                                    .arg("XML files (*.xml)"),
                                                    nullptr);
    if (fileName.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(fileName);
    QString fileExt = fileInfo.suffix();
    if (fileExt == "mat") {
        this->loadMatFile(fileName);
    } else if (fileExt == "fiff") {
        this->loadFiffRawFile(fileName);
    } else if (fileExt == "txt") {
        this->loadTextFile(fileName);
    } else if (fileExt == "json") {
        this->loadJsonFile(fileName);
    } else if (fileExt == "xml") {
        this->loadXmlFile(fileName);
    } else {
        QMessageBox::warning(nullptr, "Warning", "Invalid file type.", QMessageBox::Yes);
        return;
    }

    m_pUi->m_qLabel_OpenedClassifier->setText(fileInfo.absoluteFilePath());
    m_pClassifiers->m_sFullFileName = fileInfo.absoluteFilePath();
}

//=====================================================================================================================
void ClassifiersSetupWidget::onPushbuttonLoadFeatureData()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load trained classifier"), "./MNE-sample-data",
                                                    tr("All (*.*);; %1;; %2;; %3;; %4")
                                                    .arg("Mat files (*.mat)")
                                                    .arg("fiff raw (*.fiff)")
                                                    .arg("Text files (*.txt)")
                                                    .arg("Json files (*json)")
                                                    .arg("XML files (*.xml)"),
                                                    nullptr);
    if (fileName.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(fileName);
    QString fileExt = fileInfo.suffix();
    if (fileExt == "mat") {
        this->loadMatFile(fileName);
    } else if (fileExt == "fiff") {
        this->loadFiffRawFile(fileName);
    } else if (fileExt == "txt") {
        this->loadTextFile(fileName);
    } else if (fileExt == "json") {
        this->loadJsonFile(fileName);
    } else if (fileExt == "xml") {
        this->loadXmlFile(fileName);
    } else {
        QMessageBox::warning(nullptr, "Warning", "Invalid file type.", QMessageBox::Yes);
        return;
    }

    m_pUi->m_qLabel_OpenedFeature->setText(fileInfo.absoluteFilePath());
}


//=====================================================================================================================
void ClassifiersSetupWidget::onPushbuttonLoadRawData()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load trained classifier"), "./MNE-sample-data",
                                                    tr("All (*.*);; %1;; %2;; %3;; %4")
                                                    .arg("Mat files (*.mat)")
                                                    .arg("fiff raw (*.fiff)")
                                                    .arg("Text files (*.txt)")
                                                    .arg("Json files (*json)")
                                                    .arg("XML files (*.xml)"),
                                                    nullptr);
    if (fileName.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(fileName);
    QString fileExt = fileInfo.suffix();
    if (fileExt == "mat") {
        this->loadMatFile(fileName);
    } else if (fileExt == "fiff") {
        this->loadFiffRawFile(fileName);
    } else if (fileExt == "txt") {
        this->loadTextFile(fileName);
    } else if (fileExt == "json") {
        this->loadJsonFile(fileName);
    } else if (fileExt == "xml") {
        this->loadXmlFile(fileName);
    } else {
        QMessageBox::warning(nullptr, "Warning", "Invalid file type.", QMessageBox::Yes);
        return;
    }

    m_pUi->m_qLabel_OpenedRaw->setText(fileInfo.absoluteFilePath());
}

//=====================================================================================================================
void ClassifiersSetupWidget::onPushbuttonTrainFromFeature()
{

}

//=====================================================================================================================
void ClassifiersSetupWidget::onPushbuttonTrainFromRaw()
{

}

//=====================================================================================================================
void ClassifiersSetupWidget::loadMatFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("Load MAT-File"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QByteArray byteArray = file.readAll();
    file.close();

    m_pClassifiers->setMatByteArray(byteArray);
    m_pClassifiers->parsingMat();

    /*connect(m_pMatParser, &MatParser::sig_updateVariableInfo,
            this, &ClassifiersSetupWidget::onUpdateVariableInfo);*/
}

//=====================================================================================================================
void ClassifiersSetupWidget::loadFiffRawFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("Load Fiff Raw File"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QByteArray byteArray = file.readAll();
    file.close();
}

//=====================================================================================================================
void ClassifiersSetupWidget::loadTextFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("Load Text File"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QByteArray byteArray = file.readAll();
    file.close();
}

//=====================================================================================================================
void ClassifiersSetupWidget::loadJsonFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("Load Json File"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QByteArray byteArray = file.readAll();
    file.close();
}

//=====================================================================================================================
void ClassifiersSetupWidget::loadXmlFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::warning(this, tr("Load XML File"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QByteArray byteArray = file.readAll();
    file.close();
}

//=====================================================================================================================
void ClassifiersSetupWidget::onUpdateClassifiersInfo(const QString &info)
{
    m_pUi->m_qTextBrowser_ClassifiersInfo->append(info);
}


