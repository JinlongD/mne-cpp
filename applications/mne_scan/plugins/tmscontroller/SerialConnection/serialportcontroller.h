//=============================================================================================================
/**
* @file     serialportcontroller.h
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
* @brief    Contains the declaration of the SerialPortController class.
*
*
* The class creates a Thread which has direct control of the serial port.
* Commands for relaying via the serial port are received from signals.
* N.B. Note that all functions except for run, are run in the callers thread
*
*
*     // serialConnection:    The serial port, e.g COM1
*     // serialWriteQueue:    Queue to send messages to MagStim unit
*     // serialReadQueue:     Queue to receive messages from MagStim unit
*/
//=============================================================================================================

#ifndef SERIALPORTCONTROLLER_H
#define SERIALPORTCONTROLLER_H

//=============================================================================================================
// INCLUDES
//=============================================================================================================

//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QThread>
#include <QSerialPort>
#include <QQueue>
#include <QMutex>
#include <iostream>
#include <QMap>
#include <QMetaType>

//=============================================================================================================
// EIGEN INCLUDES
//=============================================================================================================

//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================

//=============================================================================================================
// DEFINE NAMESPACE TMSCONTROLLERPLUGIN
//=============================================================================================================

namespace TMSCONTROLLERPLUGIN
{

//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================

typedef std::tuple<QByteArray, QString, int> sendInfo;
//Q_DECLARE_METATYPE(sendInfo);
typedef std::tuple<int, QByteArray> receiveInfo;
//Q_DECLARE_METATYPE(receiveInfo);
typedef std::map<QString, std::map<QString, double>> messageInfo;
//Q_DECLARE_METATYPE(messageInfo);

//=============================================================================================================
/**
 * DECLARE CLASS SerialPortController
 *
 * @brief The SerialPortController class provides a SerialPortController thread to control serial communication.
 */
class SerialPortController : public QThread
{
    Q_OBJECT
public:
    //typedef std::tuple<QByteArray, QString, int> sendInfo;
    //Q_DECLARE_METATYPE(sendInfo);
    //typedef std::tuple<int, QByteArray> receiveInfo;
    //Q_DECLARE_METATYPE(receiveInfo);

    //=========================================================================================================
    /**
     * Constructs a SerialPortController.
     *
     * @param [in] portName     a QString gives the serial port name, e.g. "COM1".
     * @param [in] writeQueue   a QQueue to send messages to magstim devices.
     * @param [in] readQueue    a QQueue to receive messages from magstim devices.
     *
     */
    SerialPortController(QString portName,
                         QQueue<sendInfo> writeQueue,
                         QQueue<receiveInfo> readQueue);

    //=========================================================================================================
    /**
     * Destroys the SerialPortController.
     */
    ~SerialPortController();

    //=========================================================================================================
    /**
     * Update write message queue.
     *
     * @param [in] info     Message to send to magstim devices; std::tuple<QByteArray, QString, int>.
     */
    void updateWriteQueue(sendInfo info); //

protected:
    //=========================================================================================================
    /**
     * Override Thread function.
     */
    void run() override;

private:
    QString             m_portName;     // Address of the serial port, i.e. port name
    QQueue<sendInfo>    m_writeQueue;   // Queue for writing to MagStim
    QQueue<receiveInfo> m_readQueue;    // Queue for reading from MagStim
    QMutex              m_mutex;        // To protect data in this thread, allowing only one thread at a time can access it.

    const int SERIAL_WRITE_ERROR = 1;   // SERIAL_WRITE_ERR: Could not send the command.
    const int SERIAL_READ_ERROR  = 2;   // SERIAL_READ_ERR: Could not read the MagStim response.

signals:
    //=========================================================================================================
    /**
     * Emitted when read message from magstim devices is available
     */
    void updateReadQueue(const receiveInfo &info); // A message from the MagStim unit was read.
};
}   //  NAMESPACE

Q_DECLARE_METATYPE(TMSCONTROLLERPLUGIN::sendInfo)
Q_DECLARE_METATYPE(TMSCONTROLLERPLUGIN::receiveInfo);
Q_DECLARE_METATYPE(TMSCONTROLLERPLUGIN::messageInfo);
//Q_DECLARE_METATYPE(std::tuple<>);

#endif // SERIALPORTCONTROLLER_H
