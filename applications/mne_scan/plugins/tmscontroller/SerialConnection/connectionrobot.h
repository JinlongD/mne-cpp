//=============================================================================================================
/**
* @file     connectionrobot.h
* @author   Hannes Oppermann <hannes.oppermann@tu-ilmenau.de>;
*           Felix Wichum <felix.wichum@tu-ilmenau.de>
* @version  1.0
* @date     November, 2019
*
* @section  LICENSE
*
* This software was derived from the python toolbox MagPy by N. McNair
* Copyright (C) 2019, Hannes Oppermann and Felix Wichum. All rights reserved.
*
* GNU General Public License v3.0 (LICENSE)
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <https://www.gnu.org/licenses/>.
*
* @brief    Contains the declaration of the ConnectionRobot class.
*
*
* The class creates a thread which sends an 'enable remote control' command to the Magstim via the
* serialPortController thread every 500ms.
*   N.B. Note that all functions except for run, are run in the callers thread
*
* Continuously send commands to the serialPortController thread at regular intervals, while also monitoring
* the updateTimeQueue for commands from the parent thread if this should be delayed, paused, or stopped.
*   N.B. This should be called via start()
*
* @brief Keeps remote control.
*/
//=============================================================================================================

#ifndef CONNECTIONROBOT_H
#define CONNECTIONROBOT_H

//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "serialportcontroller.h"

//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QThread>
#include <QQueue>
#include <QMutex>
#include <iostream>
#include <QTimer>
#include <QEventLoop>


//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================

//=============================================================================================================
// DEFINE NAMESPACE TMSCONTROLLERPLUGIN
//=============================================================================================================

namespace TMSCONTROLLERPLUGIN
{

//=============================================================================================================
// TMSCONTROLLERPLUGIN FORWARD DECLARATIONS
//=============================================================================================================

//class SerialPortController;

//=============================================================================================================
/**
 * DECLARE CLASS ConnectionRobot
 *
 * @brief The TmsControllerSetupWidget class provides the TmsController configuration window.
 */
class ConnectionRobot : public QThread
{
    Q_OBJECT
public:
    //typedef std::tuple<QByteArray, QString, int> sendInfo;
    //typedef std::tuple<int, QByteArray> receiveInfo;

    //=========================================================================================================
    /**
     * Constructs a SerialPortController.
     *
     * @param [in] writeQueue   a QQueue to send messages to magstim devices.
     * @param [in] readQueue    a QQueue to receive messages from magstim devices.
     *
     */
    ConnectionRobot(QQueue<sendInfo> writeQueue,
                    QQueue<float> updateRobotQueue);

    //=========================================================================================================
    /**
     * Destroys the SerialPortController.
     */
    ~ConnectionRobot();

    //=========================================================================================================
    /**
     * Set command of send messages.
     *
     * @param [in] connectionCommand    command to stay in connection with magstim device; std::tuple<QByteArray, QString, int>.
     */
    void setCommand(sendInfo connectionCommand);

    //=========================================================================================================
    /**
     * Set command of send messages.
     *
     * @param [in] info     Updates the update queue to control the robot.
     *                      info: message for Robot (NAN: stop; -1: pause; 0: go on; 1: slow down; 2: speed up)
     */
    void updateUpdateRobotQueue(const float info);

protected:
    //=========================================================================================================
    /**
     * Override Thread function.
     */
    void run() override;

private:
    QQueue<sendInfo>    m_writeQueue;           // Queue for writing to MagStim
    sendInfo            m_connectionCommand;    // command send to the MagStim
    QQueue<float>       m_updateRobotQueue;     // Queue for controllign the connectionRobot
    bool                m_stopped;              // The Robot stoped
    bool                m_paused;               // The Robot paused
    double              m_nextPokeTime;         // Next time sending a command to the MagStim
    QMutex              m_mutex;                // To protect data in this thread
    QEventLoop          m_eventloop;            // Wait for Signals. Execution stops when data arrived.
    QTimer              m_timer;                // Counter

signals:
    void updateWriteQueue(const sendInfo info);   // send a message to the MagStim unit
    void readInfo();        // A message was received. Allows to leave QEventLoop
};
}   // NAMESPACE
#endif // CONNECTIONROBOT_H
