//=============================================================================================================
/**
* @file     rapid.h
* @author   Hannes Oppermann <hannes.oppermann@tu-ilmenau.de>;
*           Felix Wichum <felix.wichum@tu-ilmenau.de>
* @version  1.0
* @date     November, 2019
*
* This software was derived from the python toolbox MagPy by N. McNair
* @section  LICENSE
*
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
* @brief    Contains the declaration of the Rapid class.
*
* This is a sub-class of the parent Magstim class used for controlling Rapid^2 Magstim units. It allows firing
* in either single-pulse mode or rTMS mode. In single-pulse mode, the maximum firing frequency is 1 Hz (0.5 Hz
* if enhanced-power mode is enabled and power is 100 - 110%). To enable rTMS mode, you must first call rTMSMode(True).
* To disable rTMS mode, call rTMSMode(False).
*   N.B. In rTMS mode the maximum frequency allowed is dependent on the power level. Also, there is a dependent
*   relationship between the Duration, NPulses, and Frequency parameter settings. Therefore it is recommended
*   either to seek confirmation of any change in settings or to evaluate allowable changes beforehand. In addition,
*   after each rTMS train there is an enforced delay (minimum 500 ms) before any subsequent train can be initiated
*   or before any rTMS parameter settings can be altered.
*
*
* @brief This is a sub-class of the parent Magstim class used for controlling Rapid^2 Magstim units
*/

#ifndef RAPID_H
#define RAPID_H

//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "magstim.h"

//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

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
 * DECLARE CLASS Rapid
 *
 * @brief The MagStim class provides a MagStim to control serial communication.
 */
class Rapid : public MagStim
{
public:
    //=========================================================================================================
    /**
     * Constructs a Rapid. Overloaded function:
     *
     * @param [in] portName     a QString gives the serial port name, e.g. "COM1".
     * @param [in] superRapid   TODO Doxygen.
     * @param [in] unlockCode   if provided, leave empty if not.
     * @param [in] voltage      240/115 V.
     * @param [in] version      software version.
     *
     */
    Rapid(QString portName, int superRapid, QString unlockCode, int voltage, std::tuple<int, int, int> version);
    //=========================================================================================================
    /**
     * // Constructs a Rapid using defaults settings. Overloaded function
     *
     * @param [in] portName     a QString gives the serial port name, e.g. "COM1".
     *
     */
    Rapid(QString portName);

    //=========================================================================================================
    /**
     * Destroys the Rapid.
     */
    ~Rapid();

    //=========================================================================================================
    /**
     * Calculate minimum wait time between trains for given power, frequency, and number of pulses. // TODO Doxygen
     *
     * @param [in] power:       TODO Doxygen
     * @param [in] nPulses:     TODO Doxygen
     * @param [in] frequency:   TODO Doxygen
     * @param [out] return      TODO Doxygen
     *
     */
    float getRapidMinWaitTime(int power, int nPulses, float frequency);

    //=========================================================================================================
    /**
     * Calculate maximum train duration per minute for given power and frequency. // TODO Doxygen
     * If greater than 60 seconds, then it will allow for continuous operation for up to 6000 pulses.
     *
     * @param [in] power:       TODO Doxygen
     * @param [in] frequency:   TODO Doxygen
     * @param [out] return      TODO Doxygen
     *
     */
    float getRapidMaxOnTime(int power, float frequency);

    //=========================================================================================================
    /**
     * Calculate maximum frequency that will allow for continuous operation (up to 6000 pulses). // TODO Doxygen
     *
     * @param [in] power:       TODO Doxygen
     * @param [in] nPulses:     TODO Doxygen
     * @param [in] frequency:   TODO Doxygen
     * @param [out] return      TODO Doxygen
     *
     */
    float getRapidMaxContinuousOperationsFrequency(int power);  // TODO Doxygen

    void setupSerialPort(QString portName);
    // Opens the serialportcontroller to connect with the MagStim unit
    // serialConnection: serialPort, e.g. Windows: "COM1"; Linux: "/dev/ttyS0"

    std::tuple<int, int, int> getVersion(int &error = MagStim::m_er);
    // Get MagStim software version number. This is needed when obtaining parameters from the MagStim unit

    int getErrorCode(); // Get current error code from Rapid
    void connect(int &error = MagStim::m_er);
    void disconnect(int &error = MagStim::m_er);
    void rTMSMode(bool enable, messageInfo &message = MagStim::m_mes, int &error = MagStim::m_er);
    int ignoreCoilSafetySwitch(int &error = MagStim::m_er);
    void remoteControl(bool enable, messageInfo &message = MagStim::m_mes, int &error = MagStim::m_er);
    void enhancedPowerMode(bool enable, messageInfo &message = MagStim::m_mes, int &error = MagStim::m_er);
    bool isEnhanced();
    int setFrequency(float newFrequency, messageInfo &message = MagStim::m_mes, int &error = MagStim::m_er);
    int setNPulses(int newPulses, messageInfo &message = MagStim::m_mes);
    int setDuration(float newDuration, messageInfo &message = MagStim::m_mes);
    messageInfo getParameters(int &error = MagStim::m_er);
    void setPower(int newPower, bool delay, messageInfo &message = MagStim::m_mes, int &error = MagStim::m_er);
    int setChargeDelay(int newDelay, messageInfo &message = MagStim::m_mes, int &error = MagStim::m_er);
    int getChargeDelay(messageInfo &message = MagStim::m_mes, int &error = MagStim::m_er);
    void fire(int &error = MagStim::m_er);
    void quickFire(int & error = MagStim::m_er);
    int validateSequence();
    int getSystemStatus(messageInfo &message = MagStim::m_mes);
private:
    void setDefault();  // Set member variables as default settings. Read from yaml file if possible.

    int         DEFAULT_RAPID_TYPE;
    int         DEFAULT_VOLTAGE;
    bool        ENFORCE_ENERGY_SAFETY;
    QString     DEFAULT_UNLOCK_CODE;
    std::tuple<int, int, int>                               DEFAULT_VIRTUAL_VERSION;
    std::map<QString, std::map<QString, int>>               DEFAULT_MESSAGE;  // <sendInfo>
    QMap<QString, QVariant>                                 JOULES;
    QMap<QString, QMap<QString, QMap<QString, QVariant>>>   MAX_FREQUENCY;

    int     m_super;                        // Super Rapid Mode
    int     m_voltage;                      // Voltage of the stimulator unit
    bool    m_sequenceValidated;            // MagStim allows current settings
    bool    m_repetitiveMode;               // rTMS mode
    QString m_unlockCode;                   // Unlock code
    std::tuple<int, int, int> m_version;    // Software version (X, Y, Z)
};
} // NAMESPACE
#endif // RAPID_H






