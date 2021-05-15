//=============================================================================================================
/**
* @file     magstim.h
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
* @brief    Contains the declaration of the MagStim class.
*
* The base Magstim class. This is used for controlling 200^2 Magstim units, and acts as a parent class for
* the BiStim^2 and Rapid^2 sub-classes. It also creates two additional processes; one for the purposes
* of directly controlling the serial port and another for maintaining constant contact with the Magstim.
*   N.B. This class can effect limited control over BiStim^2 and Rapid^2 units, however some functionality
*   will not be able to be accessed and return values (including confirmation of commands) may be invalid.
*   To begin sending commands to the Magstim, and start the additional processes, you must first call connect().
*
* @brief The base Magstim class is used for controlling the unit.
*/

#ifndef MAGSTIM_H
#define MAGSTIM_H

//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "../SerialConnection/connectionrobot.h"
#include "../SerialConnection/serialportcontroller.h"

//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QQueue>
#include <iostream>

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

//=============================================================================================================
/**
 * DECLARE CLASS MagStim
 *
 * @brief The MagStim class provides a MagStim to control serial communication.
 */
class MagStim : public QObject
{
    Q_OBJECT
public:
    //typedef std::tuple<QByteArray, QString, int> sendInfo;
    //typedef std::tuple<int, QByteArray> receiveInfo;
    //typedef std::map<QString, std::map<QString, double>> messageInfo;

    //=========================================================================================================
    /**
     * Constructs a SerialPortController.
     *
     * @param [in] portName     a QString gives the serial port name, e.g. "COM1".
     *
     */
    MagStim(QString portName, QObject *parent = nullptr);
    // TODO: Destructor

    //=========================================================================================================
    /**
     * Destroys the SerialPortController.
     */
    ~MagStim();

    //=========================================================================================================
    /**
     * Update received message queue.
     */
    void updateReceiveQueue(receiveInfo info);

public:
    static int m_er;                            // Used as a placeholder: error
    static std::tuple<int, int, int> &m_ver;    // Used as a placeholder: version
    static messageInfo &m_mes;                  // Used as a placeholder: message
    // Error Codes
    const int INVALID_COMMAND_ERR       = 3; // Invalid command sent.
    const int INVALID_DATA_ERR          = 4; // Invalid data provided.
    const int COMMAND_CONFLICT_ERR      = 5; // Command conflicts with current system configuratin.
    const int INVALID_CONFIRMATION_ERR  = 6; // Upexpected command confirmation received.
    const int CRC_MISMATCH_ERR          = 7; // Message contents and CRC value do not match.
    const int NO_REMOTE_CONTROL_ERR     = 8; // You have not established control of the Magstim unit.
    const int PARAMETER_ACQUISTION_ERR  = 9; // Could not obtain prior parameter settings.
    const int PARAMETER_UPDATE_ERR      = 10;// Could not update secondary parameter to accommodate primary parameter change.
    const int PARAMETER_FLOAT_ERR       = 11;// A float value is not allowed for this parameter.
    const int PARAMETER_PRECISION_ERR   = 12;// Only one decimal placed allowed for this parameter.
    const int PARAMETER_RANGE_ERR       = 13;// Parameter value is outside the allowed range.
    const int GET_SYSTEM_STATUS_ERR     = 14;// Cannot call getSystemStatus() until software version has been established.
    const int SYSTEM_STATUS_VERSION_ERR = 15;// Method getSystemStatus() is not compatible with your software version.
    const int SEQUENCE_VALIDATION_ERR   = 16;// You must call validateSequence() before you can run a rTMS train.
    const int MIN_WAIT_TIME_ERR         = 17;// Minimum wait time between trains violated. Call isReadyToFire() to check.
    const int MAX_ON_TIME_ERR           = 18;// Maximum on time exceeded for current train.

protected:
    QQueue<sendInfo>        m_sendQueue;            // Queue for writing to MagStim unit
    QQueue<receiveInfo>     m_receiveQueue;         // Queue for reading from MagStim unit
    QQueue<float>           m_robotQueue;           // Queue for controlling the connectionRobot
    sendInfo                m_connectionCommand;    // Command to stay in contact with MagStim unit.
    bool                    m_connected;            // Status about current connection
    int                     m_parameterReturnByte;  // Number of parameters expected to read from unit, depends on software version
    QEventLoop              m_eventLoop;            // Do nothing while waiting for response from MagStim
    SerialPortController*   m_serial;               // Controls the serial port
    ConnectionRobot*        m_robot;                // Keeps remote control

signals:
    void updateSendQueue(const sendInfo &info);
    void updateRobotQueue(const float &info);
    void readInfo();

public:
    // This starts the serial port controller, as well as a process
    // that constantly keeps in contact with the MagStim unit so as not to lose control.
    virtual void connect(int &error = MagStim::m_er);     //Connect to the MagStim unit.
    virtual void disconnect(int &error = MagStim::m_er);  // Disconnect from the MagStim unit.
    // Enable/Disable remote control, Disabling will first disarm MagStim unit.
    virtual void remoteControl(bool enable,
                               messageInfo &message = MagStim::m_mes,
                               int &error = MagStim::m_er);
    // Must allow at least 1 second for the stimulator to arm.
    virtual void arm(bool delay = false,
                     messageInfo &message = MagStim::m_mes,
                     int &error = MagStim::m_er);
    virtual void disarm(messageInfo &message = MagStim::m_mes,
                        int &error = MagStim::m_er);
    virtual void setPower(int newPower,
                          bool delay = false,
                          int &error = MagStim::m_er,
                          QString commandByte = "@",
                          messageInfo &message = MagStim::m_mes);
    virtual void poke(bool silent = false);
    virtual void fire(messageInfo &message = MagStim::m_mes,
                      int &error = MagStim::m_er);
    virtual void quikeFire();
    virtual void resetQuikeFire();


    virtual messageInfo getParameters(int &error = MagStim::m_er);
    virtual messageInfo getTemperature(int &error = MagStim::m_er);

    virtual bool isArmed();
    virtual bool isUnderControl();
    virtual bool isReadyToFire();
protected:
    virtual messageInfo parseMagstimResponse(std::list<int> responseString,
                                             QString responseType);
    virtual std::tuple<int, int, int> parseMagstimResponse_version(std::list<int> responseString);
    virtual void setupSerialPort(QString portName);
    virtual int processCommand(QString commandString,
                               QString receiptType,
                               int readBytes,
                               messageInfo &message);
    virtual int processCommand(QString commandString,
                               QString receiptType,
                               int readBytes,
                               std::tuple<int, int, int> &version);
    virtual int processCommand(QString commandString,
                               QString receiptType,
                               int readBytes,
                               std::tuple<int, int, int> &version,
                               messageInfo &message);
    virtual char calcCRC(QByteArray command);
};
}   // NAMESPACE

Q_DECLARE_METATYPE(std::tuple<>);

#endif // MAGSTIM_H
