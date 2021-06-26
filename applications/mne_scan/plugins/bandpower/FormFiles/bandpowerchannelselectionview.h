//=====================================================================================================================
/**
 * @file     bandpowerchannelselectionview.h
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
 * @brief    Contains the declaration of the bandpowerchannelselectionview class.
 *
 */
//=====================================================================================================================
#ifndef BANDPOWERCHANNELSELECTIONVIEW_H
#define BANDPOWERCHANNELSELECTIONVIEW_H

//=====================================================================================================================
// INCLUDES
//=====================================================================================================================

//=====================================================================================================================
// QT INCLUDES
//=====================================================================================================================
#include <QWidget>
#include <QListWidgetItem>

//=====================================================================================================================
// EIGEN INCLUDES
//=====================================================================================================================

//=====================================================================================================================
// FORWARD DECLARATIONS
//=====================================================================================================================
namespace Ui {
class BandpowerChannelSelectionView;
}

//class QListWidgetItem;

//=====================================================================================================================
// DEFINE NAMESPACE BANDPOWERPLUGIN
//=====================================================================================================================
namespace BANDPOWERPLUGIN {

//=====================================================================================================================
// BANPOWERPLUGIN FORWARD DECLARATIONS
//=====================================================================================================================

//=====================================================================================================================
/**
 * DECLARE CLASS BandpowerChannelSelectionView
 *
 * @brief The BandpowerChannelSelectionView class provides the channel selection window.
 */
class BandpowerChannelSelectionView : public QWidget
{
    Q_OBJECT

public:
    typedef QSharedPointer<BandpowerChannelSelectionView> SPtr; /**< Shared pointer type for BandpowerChannelSelectionView. */
    typedef QSharedPointer<const BandpowerChannelSelectionView> ConstSPtr; /**< Const shared pointer type. */

    //=================================================================================================================
    /**
     * @brief constructs a BandpowerChannelSelectionView
     * @param [in] sSettingsPath
     * @param [in] parent
     */
    explicit BandpowerChannelSelectionView(const QStringList &sEEGChNames = {}, // transfer params by QSettings.
                                           const QStringList &sPickedChNames = {},
                                           QWidget *parent = 0);

    //=================================================================================================================
    /**
     * @brief Destroys the BandpowerChannelSelectionView.
     */
    ~BandpowerChannelSelectionView();

    //=================================================================================================================
    /**
     * @brief slot called when m_qPushButton_AddSelection clicked.
     */
    void onPushButtonAddSelection();

    //=================================================================================================================
    /**
     * @brief slot called when m_qPushButton_RemoveSelection clicked.
     */
    void onPushButtonRemoveSelection();

    //=================================================================================================================
    /**
     * @brief slot called when m_qPushButton_AddAll clicked.
     */
    void onPushButtonAddAll();

    //=================================================================================================================
    /**
     * @brief slot called when m_qPushButton_ClearAll clicked.
     */
    void onPushButtonClearAll();

    //=================================================================================================================
    /**
     * @brief slot called when m_qPushButton_Done clicked.
     */
    void onPushButtonDone();

    //=================================================================================================================
    /**
     * @brief onChangeDoubleClickedAddSelection
     * @param item
     */
    void onDoubleClickedAddItem(QListWidgetItem *clickedItem);

    //=================================================================================================================
    /**
     * @brief onChangeDoubleClickedRemoveSelection
     * @param item
     */
    void onDoubleClickedRemoveItem(QListWidgetItem *clickedItem);

protected:
    //=================================================================================================================
    /**
     * @brief closeEvent
     * @param event
     */
    void closeEvent(QCloseEvent *pEvent) override;

private:
    //=================================================================================================================
    /**
     * @brief eventFilter
     * @param obj
     * @param event
     * @return
     */
    //bool eventFilter(QObject *obj, QEvent *event) override;

    //=================================================================================================================
    /**
     * @brief initAvailableChannels
     * @param sEEGChNames
     * @return
     */
    bool initAvailableChannels();

    //=================================================================================================================
    /**
     * @brief initPickedChannels
     * @param sPickedChNames
     * @return
     */
    bool initPickedChannels();

private:
    Ui::BandpowerChannelSelectionView *m_pUi;   /**< The UI class specified in the designer. */

    QStringList m_sEEGChNames;
    QStringList m_sPickedChIndex;
    QMap<QString, QString> m_mapNameToIndex;

signals:
    //=================================================================================================================
    /**
     * @brief sig_updatePickedChs
     * @param sPickedChNames
     */
    void sig_updatePickedChNames(QStringList sPickedChIndex);
};
} // NAMESPACE

#endif // BANDPOWERCHANNELSELECTIONVIEW_H
