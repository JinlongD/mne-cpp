//=============================================================================================================
/**
* @file     rapid.cpp
* @author   Hannes Oppermann <hannes.oppermann@tu-ilmenau.de>;
*           Felix Wichum <felix.wichum@tu-ilmenau.de>
* @version  1.0
* @date     November, 2019
*
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
* @brief    Definition of the Rapid class.
*/

//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "rapid.h"

//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

//=============================================================================================================
// EIGEN INCLUDES
//=============================================================================================================

//=============================================================================================================
// USED NAMESPACES
//=============================================================================================================

using namespace TMSCONTROLLERPLUGIN;

//=============================================================================================================
// DEFINE MEMBER METHODS
//=============================================================================================================

Rapid::Rapid(QString portName, int superRapid, QString unlockCode,
             int voltage, std::tuple<int, int, int> version) : MagStim(portName)
{
    try {
        this->setDefault();
    }  catch (const char* msg) {
        std::cout << msg << std::endl;
    }
    this->m_super = superRapid;
    this->m_unlockCode = unlockCode;
    this->m_voltage = voltage;
    this->m_parameterReturnByte = NAN;
    this->m_sequenceValidated = false;
    this->m_repetitiveMode = false;
    //this->m_version = std::make_tuple(0, 0, 0);
    this->m_version = version;

    if (!unlockCode.isEmpty())
    {
        this->m_connectionCommand = std::make_tuple(QString("x@G").toUtf8(), "", 6);
    }
}

//=============================================================================================================

Rapid::Rapid(QString portName):MagStim(portName)
{
    try {
        this->setDefault();
    }  catch (const char* msg) {
        std::cout << msg << std::endl;
    }

    Rapid(portName, this->DEFAULT_RAPID_TYPE, this->DEFAULT_UNLOCK_CODE,
          this->DEFAULT_VOLTAGE, this->DEFAULT_VIRTUAL_VERSION);
}

//=============================================================================================================

Rapid::~Rapid()
{

}

//=============================================================================================================

float Rapid::getRapidMinWaitTime(int power, int nPulses, float frequency)
{
    float minWaitTime = 0.5;
    float calcWaitTime = (nPulses *((frequency * this->JOULES[QString::number(power)].toFloat()) - float(1050.0)));

    if (minWaitTime < calcWaitTime)
    {
        return calcWaitTime;
    }
    else
    {
        return minWaitTime;
    }
}

//=============================================================================================================

float Rapid::getRapidMaxOnTime(int power, float frequency)
{
    float PulseNum = 63000.0;
    float FreqPow = frequency * this->JOULES[QString::number(power)].toFloat();

    return PulseNum / FreqPow;
}

//=============================================================================================================

float Rapid::getRapidMaxContinuousOperationsFrequency(int power)
{
    float a = 1050.0;
    float b = this->JOULES[QString::number(power)].toFloat();

    return a / b;
}

//=============================================================================================================

void Rapid::setupSerialPort(QString portName)
{
    MagStim::setupSerialPort(portName);
}

//=============================================================================================================

std::tuple<int, int, int> Rapid::getVersion(int &error)
{
    std::tuple<int, int, int> tempVersion;
    int helper = 0;
    error = this->processCommand("ND", "version", helper, tempVersion);

    if (error == 0)
    {
        // if we didn't receive an error, update the version number and the number of bytes that will be
        // returned by a getParameters() command
        this->m_version = tempVersion;
        if (std::get<0>(this->m_version) >= 9)
        {
            this->m_parameterReturnByte = 24;
        }
        else if (std::get<0>(this->m_version) >= 7)
        {
            this->m_parameterReturnByte = 22;
        }
        else
        {
            this->m_parameterReturnByte = 21;
        }
    }

    return tempVersion;
}

//=============================================================================================================

int Rapid::getErrorCode()
{
    std::tuple<int, int, int> tempVersion;

    return this->processCommand("I@", "error", 6, tempVersion);
}

//=============================================================================================================

void Rapid::connect(int &error)
{
    MagStim::connect(error);

    if (!error)
    {
        std::ignore = this->getVersion(error);
        if (error)
        {
            this->disconnect(error);
            throw new std::string("Could not determine the software version of Rapid. Disconnecting.");
        }
    }
}

//=============================================================================================================

void Rapid::disconnect(int &error)
{
    // Just some housekeeping before we call the base MagStim class method disconnect
    this->m_sequenceValidated = false;
    this->m_repetitiveMode = false;
    MagStim::disconnect(error);
    return;
}

//=============================================================================================================

void Rapid::rTMSMode(bool enable, messageInfo &message, int &error)
{
    error = 0;
    this->m_sequenceValidated = false;
    QString commandString = "";
    // Durations of 1 or 0 are used to toggle repetitive mode on and off
    if (std::get<0>(this->m_version) >= 9)
    {
        if (enable)
        {
            commandString = "0010";
        }
        else
        {
            commandString = "0000";
        }
    }
    else
    {
        if (enable)
        {
            commandString = "010";
        }
        else
        {
            commandString = "000";
        }
    }

    error = this->processCommand(commandString, "instrRapid", 4, message);
    if (error == 0)
    {
        if (enable)
        {
            this->m_repetitiveMode = true;
            messageInfo mes;
            int updateError = 0;
            mes = getParameters(updateError);
            if (updateError == 0)
            {
                if (mes["rapidParam"]["frequency"] == 0)
                {
                    updateError = this->processCommand("B0010", "instrRapid", 4, mes);
                    if (updateError)
                    {
                        error = MagStim::PARAMETER_UPDATE_ERR;
                    }
                    else
                    {
                        error = MagStim::PARAMETER_ACQUISTION_ERR;
                    }
                } // if (mes["rapidParam"]["frequency"] == 0)
                else
                {
                    this->m_repetitiveMode = false;
                }
            } // if (updateError == 0)
        }
    }
}

//=============================================================================================================

int Rapid::ignoreCoilSafetySwitch(int &error)
{
    messageInfo mes;
    return this->processCommand("b@", "instr", 3, mes);
}

//=============================================================================================================

void Rapid::remoteControl(bool enable, messageInfo &message, int &error)
{
    this->m_sequenceValidated = false;
    if (this->m_unlockCode.isEmpty())
    {
        if (enable)
        {
            error = this->processCommand("Q@", "instr", 3, message);
        }
        else
        {
            error = this->processCommand("R@", "instr", 3, message);
        }
    } // if (this->m_unlockCode.isEmpty())
    else
    {
        if (enable)
        {
            QString string = "Q" + this->m_unlockCode.toLatin1();
            error = this->processCommand(string, "instr", 3, message);
        }
        else
        {
            error = this->processCommand("R@", "instr", 3, message);
        }
    }
}

//=============================================================================================================

void Rapid::enhancedPowerMode(bool enable, messageInfo &message, int &error)
{
    if (enable)
    {
        error = this->processCommand("^@", "instrRapid", 4, message);
    }
    else
    {
        error = this->processCommand("_@", "instrRapid", 4, message);
    }
    return;
}

//=============================================================================================================

bool Rapid::isEnhanced()
{
    int error = 0;
    messageInfo mes;
    remoteControl(true, mes, error);

    if (error)
    {
        return false;
    }
    else
    {
        return mes["rapid"]["enhancedPowerMode"];
    }
}

//=============================================================================================================

int Rapid::setFrequency(float newFrequency, messageInfo &message, int &error)
{
    this->m_sequenceValidated = false;
    newFrequency *= 10; // Convert to tenths of a Hz
    int helpFreq = int(newFrequency);

    // Make sure we have a valid frequency value
    if (helpFreq - newFrequency != 0)
    {
        return MagStim::PARAMETER_PRECISION_ERR;
    }

    messageInfo currentParameters;
    int updateError = 0;
    currentParameters = getParameters(updateError);

    if (updateError)
    {
        return MagStim::PARAMETER_ACQUISTION_ERR;
    }
    else
    {
        int maxFrequency = this->MAX_FREQUENCY[QString::number(this->m_voltage)][QString::number(this->m_super)]
                [QString::number(currentParameters["rapidParam"]["power"])].toInt();
        if (newFrequency < 0 || newFrequency > maxFrequency)
        {
            return MagStim::PARAMETER_RANGE_ERR;
        }
    }

    // Send command
    QString commandStrg = QString::number(newFrequency).rightJustified(4, '0');
    error = this->processCommand("B"+commandStrg, "instr", 4, message);

    // If we didn't get an error, update the other parameters accordingly
    if (error == 0)
    {
        messageInfo currentParameters;
        int updateError = 0;
        currentParameters = getParameters(updateError);

        if (updateError == 0)
        {
            int duration = currentParameters["rapidParam"]["duration"];
            int frequency = currentParameters["rapidParam"]["frequency"];
            QString str1 = QString::number(duration * frequency).rightJustified(5, '0');
            QString str2 = QString::number(duration * frequency).rightJustified(4, '0');
            QString str3 = "D";

            if (std::get<0>(this->m_version) >= 9)
            {
                commandStrg = str3 + str1;
                updateError = this->processCommand(commandStrg, "instrRapid", 4, currentParameters);
            }
            else
            {
                commandStrg = str3 + str2;
                updateError = this->processCommand(commandStrg, "instrRapid", 4, currentParameters);
            }

            if (updateError)
            {
                return MagStim::PARAMETER_UPDATE_ERR;
            }

        }
        else
        {
            return MagStim::PARAMETER_ACQUISTION_ERR;
        }
    }
    else
    {
        return error;
    }
}

//=============================================================================================================

int Rapid::setNPulses(int newPulses, messageInfo &message)
{
    this->m_sequenceValidated = false;
    if (newPulses % 1)
    {
        // Make sure we have a valid number of pulses value
        return MagStim::PARAMETER_FLOAT_ERR;
    }
    else if (newPulses < 0 || newPulses > 6000)
    {
        return MagStim::PARAMETER_RANGE_ERR;
    }

    // Send command
    int error;
    QString str1 = QString::number(newPulses).rightJustified(5, '0');
    QString str2 = QString::number(newPulses).rightJustified(4, '0');
    QString str3 = "D";

    if (std::get<0>(this->m_version) >= 9)
    {
        error = this->processCommand(str3 + str1, "instr", 4, message);
    }
    else
    {
        error = this->processCommand(str3 + str1, "instr", 4, message);
    }

    // If we didn't get an error, update the other parameters accordingly
    if (error == 0)
    {
        messageInfo currentParameters;
        int updateError = 0;
        currentParameters = getParameters(updateError);
        int nPulses = currentParameters["rapidParam"]["nPulses"];
        int frequency = currentParameters["rapidParam"]["frequency"];
        QString str1 = QString::number(nPulses / frequency).rightJustified(4, '0');
        QString str2 = QString::number(nPulses / frequency).rightJustified(3, '0');
        QString str3 = "[";

        if (updateError == 0)
        {
            if (std::get<0>(this->m_version) >= 9)
            {
                updateError = this->processCommand(str3 + str1, "instrRapid", 4, currentParameters);
            }
            else
            {
                updateError = this->processCommand(str3 + str2, "isntrRapid", 4, currentParameters);
            }

            if (updateError)
            {
                return MagStim::PARAMETER_UPDATE_ERR;
            }
            return 0;
        }
        else
        {
            return MagStim::PARAMETER_ACQUISTION_ERR;
        }
    }
    else
    {
        return error;
    }
}

//=============================================================================================================

int Rapid::setDuration(float newDuration, messageInfo &message)
{
    this->m_sequenceValidated = false;
    newDuration *= 10;  // Convert to tenths of a second
    int helpDuration = int(newDuration);

    // Make sure we have a valid duration value
    if (helpDuration % 1)
    {
        return MagStim::PARAMETER_PRECISION_ERR;
    }

    if (std::get<0>(this->m_version) >= 9)
    {
        if (newDuration < 0 || newDuration > 999)
        {
            return MagStim::PARAMETER_RANGE_ERR;
        }
    }
    else if (newDuration < 0 || newDuration > 999)
    {
        return MagStim::PARAMETER_RANGE_ERR;
    }

    // Send command
    int error;
    QString str1 = QString::number(newDuration).rightJustified(4, '0');
    QString str2 = QString::number(newDuration).rightJustified(3, '0');
    QString str3 = "[";

    if (std::get<0>(this->m_version) >= 9)
    {
        error = this->processCommand(str3 + str1, "instrRapid", 4, message);
    }
    else
    {
        error = this->processCommand(str3 + str2, "instrRapid", 4, message);
    }

    if (error == 0)
    {
        messageInfo currentParameters;
        int updateError = 0;
        currentParameters = getParameters(updateError);
        int duration = currentParameters["rapidParam"]["duration"];
        int frequency = currentParameters["rapidParam"]["frequency"];
        QString str1 = QString::number(duration * frequency).rightJustified(5, '0');
        QString str2 = QString::number(duration * frequency).rightJustified(4, '0');
        QString str3 = "D";

        if (updateError == 0)
        {
            if (std::get<0>(this->m_version) >= 9)
            {
                updateError = this->processCommand(str3 + str1, "instrRapid", 4, currentParameters);
            }
            else
            {
                updateError = this->processCommand(str3 + str2, "instrRapid", 4, currentParameters);
            }

            if (updateError)
            {
                return MagStim::PARAMETER_UPDATE_ERR;
            }
            return 0;
        }
        else
        {
            return MagStim::PARAMETER_ACQUISTION_ERR;
        }
    }
    else
    {
        return error;
    }
}

//=============================================================================================================

messageInfo Rapid::getParameters(int &error)
{
    messageInfo message;
    int helpNumber = this->m_parameterReturnByte;
    error = this->processCommand("\\@", "rapidParam", helpNumber, message);

    return message ;
}

//=============================================================================================================

void Rapid::setPower(int newPower, bool delay, messageInfo &message, int &error)
{
    this->m_sequenceValidated = false;

    if (newPower % 1)
    {
        // Make sure we have a valid power
        error = MagStim::PARAMETER_FLOAT_ERR;
        return;
    }
    else
    {
        if(this->isEnhanced())
        {
            if (newPower < 0 || newPower > 110)
            {
                error = MagStim::PARAMETER_RANGE_ERR;
                return;
            }
            else if (newPower < 0 || newPower > 100)
            {
                error = MagStim::PARAMETER_RANGE_ERR;
                return;
            }
        }
    }

    MagStim::setPower(newPower, delay, error, "@", message);

    if (error == 0)
    {
        messageInfo currentParameters;
        int updateError = 0;
        currentParameters = this->getParameters(updateError);
        if (updateError == 0)
        {
            if (bool(currentParameters["rapid"]["singlePulseMode"]) == false)
            {
                int maxFrequency = this->MAX_FREQUENCY[QString::number(this->m_voltage)][QString::number(this->m_super)]
                        [QString::number(currentParameters["rapidParam"]["power"])].toInt();
                if (currentParameters["rapidParam"]["frequency"] > maxFrequency)
                {
                    if (this->setFrequency(maxFrequency, message, error) != 0)
                    {
                        error = MagStim::PARAMETER_UPDATE_ERR;
                        return;
                    }
                }
            }
        } // if (updateError == 0)
        else
        {
            error = MagStim::PARAMETER_ACQUISTION_ERR;
            return;
        }
    }
}

//=============================================================================================================

int Rapid::setChargeDelay(int newDelay, messageInfo &message, int &error)
{
    if(std::get<0>(this->m_version) == 0)
    {
        return MagStim::GET_SYSTEM_STATUS_ERR;
    }
    else if(std::get<0>(this->m_version) < 9)
    {
        return MagStim::SYSTEM_STATUS_VERSION_ERR;
    }
    this->m_sequenceValidated = false;

    // Make sure we have a valid delay duration value
    if(newDelay % 1)
    {
        return MagStim::PARAMETER_FLOAT_ERR;
    }

    QString str1 = QString::number(newDelay).rightJustified(5, '0');
    QString str2 = QString::number(newDelay).rightJustified(4, '0');
    QString str3 = "n";
    if (std::get<0>(this->m_version) >= 10)
    {
        error = this->processCommand(str3 + str1, "systemRapid", 6, message);
    }
    else
    {
        error = this->processCommand(str3 + str2, "systemRapid", 4, message);
    }

    return error;
}

//=============================================================================================================

int Rapid::getChargeDelay(messageInfo &message, int &error)
{
    if(std::get<0>(this->m_version) == 0)
    {
        return MagStim::GET_SYSTEM_STATUS_ERR;
    }
    else if(std::get<0>(this->m_version) < 9)
    {
        return MagStim::SYSTEM_STATUS_VERSION_ERR;
    }

    if(std::get<0>(this->m_version) > 9)
    {
        error = this->processCommand("o@", "instrCharge", 8, message);
    }
    else
    {
        error = this->processCommand("o@", "instrCharge", 7, message);
    }
    return error;
}

//=============================================================================================================

void Rapid::fire(int &error)
{
    if (this->m_repetitiveMode && Rapid::ENFORCE_ENERGY_SAFETY && !this->m_sequenceValidated)
    {
        error = MagStim::SEQUENCE_VALIDATION_ERR;
    }
    else
    {
        messageInfo message;
        int error;
        MagStim::fire(message, error);
    }
}

//=============================================================================================================

void Rapid::quickFire(int &error)
{
    if(this->m_repetitiveMode && Rapid::ENFORCE_ENERGY_SAFETY && !this->m_sequenceValidated)
    {
        error = MagStim::SEQUENCE_VALIDATION_ERR;
    }
    else
    {
        MagStim::quikeFire();
    }
}

//=============================================================================================================

int Rapid::validateSequence()
{
    // pre-initialization
    messageInfo parameters;
    int error = 0;
    parameters = getParameters(error);
    float duration = parameters["rapidParam"]["duration"] / 10;
    float MaxOnTime = Rapid::getRapidMaxOnTime(parameters["rapidParam"]["duration"], parameters["rapidParam"]["frequency"]);
    float TimeHelp;

    if (duration < 60)
    {
        TimeHelp = duration;
    }
    else
    {
        TimeHelp = 60;
    }

    if (error)
    {
        return MagStim::PARAMETER_ACQUISTION_ERR;
    }
    else if (TimeHelp > MaxOnTime)
    {
        return MagStim::MAX_ON_TIME_ERR;
    }
    else
    {
        this->m_sequenceValidated = true;
        return 0;
    }
}

//=============================================================================================================

int Rapid::getSystemStatus(messageInfo &message)
{
    if (std::get<0>(this->m_version) == 0)
    {
        return MagStim::GET_SYSTEM_STATUS_ERR;
    }
    else if (std::get<0>(this->m_version) >= 9)
    {
        int error;
        error = this->processCommand("x@", "systemRapid", 6, message);
        return error;
    }
    else
    {
        return MagStim::SYSTEM_STATUS_VERSION_ERR;
    }
}

//=============================================================================================================

void Rapid::setDefault()
{
    QString path = ":/configurations/configuration/rapid_config.json";
    if (QFile::exists(path))
    {
        QFile file(path);
        file.open(QIODevice::ReadOnly);
        QTextStream file_text(&file);
        QString json_file = file_text.readAll();
        QJsonObject config = QJsonDocument::fromJson(json_file.toUtf8()).object();
        file.close();

        this->DEFAULT_RAPID_TYPE    = config["defaultRapidType"].toInt();
        this->DEFAULT_VOLTAGE       = config["defaultVoltage"].toInt();
        this->DEFAULT_UNLOCK_CODE   = config["unlockCode"].toString();
        this->ENFORCE_ENERGY_SAFETY = config["enforceEnergySafety"].toBool();

        QString version = config["virtualVersionNumber"].toString();
        QStringList ver_list = version.mid(1,version.length()-2).split(',');
        this->DEFAULT_VIRTUAL_VERSION = std::make_tuple(ver_list.at(0).toInt(), ver_list.at(1).toInt(), ver_list.at(2).toInt());
    }
    else
    {
        this->DEFAULT_RAPID_TYPE      = 0;
        this->DEFAULT_VOLTAGE         = 240;
        this->DEFAULT_UNLOCK_CODE     = "";
        this->ENFORCE_ENERGY_SAFETY   = true;
        this->DEFAULT_VIRTUAL_VERSION = std::make_tuple(7,0,0);

        //throw "Could not read Rapid Configuration.";
        qDebug() << "Could not read Rapid Configuration.";
    }



    path = ":/configurations/configuration/rapid_system_info.json";
    if (QFile::exists(path))
    {
        QFile file(path);
        file.open(QIODevice::ReadOnly);
        QTextStream file_text(&file);
        QString json_file = file_text.readAll();
        QJsonObject config_json = QJsonDocument::fromJson(json_file.toUtf8()).object();
        file.close();

        QVariantMap config = config_json.toVariantMap();
        this->JOULES = config["joules"].toMap();

        QVariantMap vmap_maxf = config["maxFrequency"].toMap();
        QStringList volts = vmap_maxf.keys();
        for (int i = 0; i < volts.length(); ++i)
        {
            QMap<QString, QMap<QString, QVariant>> map_volt;
            QVariantMap vmap_volt = vmap_maxf[volts.at(i)].toMap();
            QStringList modes = vmap_volt.keys();
            for (int j = 0; j < modes.length(); ++j)
            {
                QVariantMap vmap_mode = vmap_volt[modes.at(j)].toMap();
                map_volt.insert(modes.at(j),vmap_mode);
            }
            this->MAX_FREQUENCY.insert(volts.at(i), map_volt);
        }
    }
    else
    {
        //throw "Could not read Rapid System Info. rTMS mode for Rapid is not possible.";
        qDebug() << "Could not read Rapid System Info. rTMS mode for Rapid is not possible.";
    }
}

