//=====================================================================================================================
/**
 * @file     bandpowerchannelselectionview.cpp
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
 * @brief    Definition of the bandpowerchannelselectionview class.
 *
 */
//=====================================================================================================================

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================
#include "bandpowerchannelselectionview.h"
#include "ui_bandpowerchannelselectionview.h"

//=====================================================================================================================
// QT INCLUDES
//=====================================================================================================================
#include <QSettings>
#include <QCloseEvent>
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
BandpowerChannelSelectionView::BandpowerChannelSelectionView(const QString& sSettingsPath,
                                                             QWidget *parent)
    : QWidget(parent)
    , m_pUi(new Ui::BandpowerChannelSelectionView)
{
    m_sSettingsPath = sSettingsPath;
    m_pUi->setupUi(this);

    this->setWindowTitle(QString("Channel selection"));
    this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    loadSettings();

    //m_pUi->m_qListWidget_AvailableChannels->installEventFilter(this);
    //m_pUi->m_qListWidget_SelectedChannels->installEventFilter(this);

    this->initAvailableChannels();
    this->initPickedChannels();
    //this->setFocusProxy(m_pUi->m_qListWidget_AvailableChannels);
    //m_pUi->m_qListWidget_AvailableChannels->setCurrentRow(0);
    m_pUi->m_qListWidget_AvailableChannels->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_pUi->m_qListWidget_SelectedChannels->setSelectionMode(QAbstractItemView::ExtendedSelection);

    connect(m_pUi->m_qPushButton_AddSelection, &QPushButton::clicked,
            this, &BandpowerChannelSelectionView::onPushButtonAddSelection);
    connect(m_pUi->m_qPushButton_RemoveSelection, &QPushButton::clicked,
            this, &BandpowerChannelSelectionView::onPushButtonRemoveSelection);
    connect(m_pUi->m_qPushButton_AddAll, &QPushButton::clicked,
            this, &BandpowerChannelSelectionView::onPushButtonAddAll);
    connect(m_pUi->m_qPushButton_ClearAll, &QPushButton::clicked,
            this, &BandpowerChannelSelectionView::onPushButtonClearAll);
    connect(m_pUi->m_qPushButton_Done, &QPushButton::clicked,
            this, &BandpowerChannelSelectionView::onPushButtonDone);

    connect(m_pUi->m_qListWidget_AvailableChannels, &QListWidget::itemDoubleClicked,
            this, &BandpowerChannelSelectionView::onDoubleClickedAddItem);
    connect(m_pUi->m_qListWidget_SelectedChannels, &QListWidget::itemDoubleClicked,
            this, &BandpowerChannelSelectionView::onDoubleClickedRemoveItem);
}

//=====================================================================================================================
BandpowerChannelSelectionView::~BandpowerChannelSelectionView()
{
    saveSettings();
    qDebug() << "[BandpowerChannelSelectionView::~BandpowerChannelSelectionView]: " << m_sPickedChIndex.size();
    delete m_pUi;
}

//=====================================================================================================================
void BandpowerChannelSelectionView::onPushButtonAddSelection()
{
    if (m_pUi->m_qListWidget_AvailableChannels->selectedItems().size() > 0) {
        for (int i = 0; i < m_pUi->m_qListWidget_AvailableChannels->selectedItems().size(); ++i) {
            QListWidgetItem *pListWidgetItem = m_pUi->m_qListWidget_AvailableChannels->selectedItems().at(i);
            QString sChIndex = m_mapNameToIndex.value(pListWidgetItem->text());
            if (!m_sPickedChIndex.contains(sChIndex)) {
                m_sPickedChIndex.append(sChIndex);
                m_pUi->m_qListWidget_SelectedChannels->addItem(pListWidgetItem->text());
                m_pUi->m_qListWidget_AvailableChannels->item(sChIndex.toInt())->setCheckState(Qt::Checked);
            }
        }
        m_pUi->m_qListWidget_SelectedChannels->sortItems(Qt::AscendingOrder);
        //emit sig_updatePickedChNames(m_sPickedChIndex);
        saveSettings();
    }
}

//=====================================================================================================================
void BandpowerChannelSelectionView::onPushButtonRemoveSelection()
{
    if (m_pUi->m_qListWidget_SelectedChannels->selectedItems().size() > 0) {
        int numItems = m_pUi->m_qListWidget_SelectedChannels->selectedItems().size(); // selectedItems is changing in the for-loop.
        QListWidgetItem *pListWidgetItem;
        for (int i = 0; i < numItems; ++i) {
            pListWidgetItem = m_pUi->m_qListWidget_SelectedChannels->selectedItems().first();
            QString sChIndex = m_mapNameToIndex.value(pListWidgetItem->text());
            int iRow = m_pUi->m_qListWidget_SelectedChannels->row(pListWidgetItem);

            m_sPickedChIndex.removeAt(m_sPickedChIndex.indexOf(sChIndex));
            delete m_pUi->m_qListWidget_SelectedChannels->takeItem(iRow);
            m_pUi->m_qListWidget_AvailableChannels->item(sChIndex.toInt())->setCheckState(Qt::Unchecked);
        }
        //emit sig_updatePickedChNames(m_sPickedChIndex);
        saveSettings();
    }
}

//=====================================================================================================================
void BandpowerChannelSelectionView::onPushButtonAddAll()
{
    if (!m_sEEGChNames.isEmpty()) {
        m_sPickedChIndex.clear();
        m_pUi->m_qListWidget_SelectedChannels->clear();
        for (int i = 0; i < m_sEEGChNames.size(); ++i) {
            m_sPickedChIndex.append(QString::number(i));
            m_pUi->m_qListWidget_SelectedChannels->addItem(m_sEEGChNames.at(i));
            m_pUi->m_qListWidget_AvailableChannels->item(i)->setCheckState(Qt::Checked);
        }
        //emit sig_updatePickedChNames(m_sPickedChIndex);
        saveSettings();
    } else {
        qDebug() << "[BandpowerChannelSelectionView::onPushButtonAddAll()]: no available channels.";
    }
}

//=====================================================================================================================
void BandpowerChannelSelectionView::onPushButtonClearAll()
{
    if (!m_sPickedChIndex.isEmpty()) {
        for (int i = 0; i < m_sPickedChIndex.size(); ++i) {
            m_pUi->m_qListWidget_AvailableChannels->item(m_sPickedChIndex.at(i).toInt())->setCheckState(Qt::Unchecked);
        }
        m_pUi->m_qListWidget_SelectedChannels->clear();
        m_sPickedChIndex.clear();

        //emit sig_updatePickedChNames(m_sPickedChIndex);
        saveSettings();
    }
}

//=====================================================================================================================
void BandpowerChannelSelectionView::onPushButtonDone()
{
    //this->saveSettings();
    //emit sig_updatePickedChNames(m_sPickedChIndex);
    //this->hide();
    this->close();
}

//=====================================================================================================================
void BandpowerChannelSelectionView::onDoubleClickedAddItem(QListWidgetItem *clickedItem)
{
    QString sChIndex = m_mapNameToIndex.value(clickedItem->text());
    if (!m_sPickedChIndex.contains(sChIndex)) {
        m_sPickedChIndex.append(sChIndex);
        m_pUi->m_qListWidget_SelectedChannels->addItem(clickedItem->text());
        m_pUi->m_qListWidget_AvailableChannels->item(sChIndex.toInt())->setCheckState(Qt::Checked);
        m_pUi->m_qListWidget_SelectedChannels->sortItems(Qt::AscendingOrder);

        //emit sig_updatePickedChNames(m_sPickedChIndex);
        saveSettings();
    }
}

//=====================================================================================================================
void BandpowerChannelSelectionView::onDoubleClickedRemoveItem(QListWidgetItem *clickedItem)
{
    QString sChIndex = m_mapNameToIndex.value(clickedItem->text());
    int iRow = m_pUi->m_qListWidget_SelectedChannels->row(clickedItem);

    m_sPickedChIndex.removeAt(m_sPickedChIndex.indexOf(sChIndex));
    delete m_pUi->m_qListWidget_SelectedChannels->takeItem(iRow);
    m_pUi->m_qListWidget_AvailableChannels->item(sChIndex.toInt())->setCheckState(Qt::Unchecked);
    //emit sig_updatePickedChNames(m_sPickedChIndex);
    saveSettings();

}

//=====================================================================================================================
void BandpowerChannelSelectionView::closeEvent(QCloseEvent *pEvent)
{
    this->saveSettings();
    emit sig_updatePickedChNames(m_sPickedChIndex);
    pEvent->accept();
    //pEvent->ignore();
}

//=====================================================================================================================
bool BandpowerChannelSelectionView::initAvailableChannels()
{
    if (!m_sEEGChNames.isEmpty()) {
        QListWidgetItem *pListWidgetItem;
        //m_pUi->m_qListWidget_AvailableChannels->addItems(m_sEEGChNames);
        for (int i = 0; i < m_sEEGChNames.size(); ++i) {
            pListWidgetItem = new QListWidgetItem;
            pListWidgetItem->setText(m_sEEGChNames.at(i));
            pListWidgetItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            //pListWidgetItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            pListWidgetItem->setCheckState(Qt::Unchecked);
            m_pUi->m_qListWidget_AvailableChannels->addItem(pListWidgetItem);

            m_mapNameToIndex[m_sEEGChNames.at(i)] = QString::number(i);
        }
        return true;
    } else {
        return false;
    }
}

//=====================================================================================================================
bool BandpowerChannelSelectionView::initPickedChannels()
{
    if (!m_sPickedChIndex.isEmpty()) {
        //QListWidgetItem *pListWidgetItem;
        for (int i = 0; i < m_sPickedChIndex.size(); ++i) {
            m_pUi->m_qListWidget_SelectedChannels->addItem(m_sEEGChNames.at(m_sPickedChIndex.at(i).toInt()));
            m_pUi->m_qListWidget_AvailableChannels->item(m_sPickedChIndex.at(i).toInt())->setCheckState(Qt::Checked);
        }
        m_pUi->m_qListWidget_SelectedChannels->sortItems(Qt::AscendingOrder);
        return true;
    } else {
        qDebug() << "[BandpowerChannelSelectionView::initPickedChannels]: no channels has been selected at present.";
        return false;
    }
}

//=====================================================================================================================
void BandpowerChannelSelectionView::saveSettings()
{
    if(m_sSettingsPath.isEmpty()) {
        return;
    }

    // Store Settings
    QSettings settings("MNECPP");
    settings.setValue(m_sSettingsPath + QString("/pickedChNames"), m_sPickedChIndex);
}

//=====================================================================================================================
void BandpowerChannelSelectionView::loadSettings()
{
    if(m_sSettingsPath.isEmpty()) {
        return;
    }

    // Load Settings
    QSettings settings("MNECPP");
    m_sPickedChIndex    = settings.value(m_sSettingsPath + QString("/pickedChNames"), {}).toStringList(); // init/default
    m_sEEGChNames       = settings.value(m_sSettingsPath + QString("/eegChNames"), {}).toStringList(); // default empty.
}
