//=============================================================================================================
/**
* @file     serialportcontroller.cpp
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
* @brief    Definition of the SerialPortController class.
*/

//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "serialportcontroller.h"

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

SerialPortController::SerialPortController(QString portName,
                                           QQueue<sendInfo> writeQueue,
                                           QQueue<receiveInfo> readQueue)
{
    qRegisterMetaType<sendInfo>("sendInfo");
    qRegisterMetaType<receiveInfo>("receiveInfo");
    qRegisterMetaType<messageInfo>("messageInfo");

    this->m_portName = portName;
    this->m_writeQueue = writeQueue;
    this->m_readQueue = readQueue;
}

//=============================================================================================================

SerialPortController::~SerialPortController()
{

}

//=============================================================================================================

void SerialPortController::updateWriteQueue(sendInfo info)
{
    QMutexLocker locker(&m_mutex);  // This locker will lock the mutex until it's destroyed
    // i.e. when this function call goes out of scope
    //this->m_serialWriteQueue.push_back(info);
    m_writeQueue.enqueue(info);
    // QQueue is a first in, first out (FIFO) structure.
    // Items are added to the tail of the queue using enqueue()
    // and retrieved from the head using dequeue().
    // The head() function provides access to the head item without removing it.
    // The last() function provides access to the last item without removing it.
}

//=============================================================================================================

void SerialPortController::run()
{
    // N.N. most of these settings are actually the default, but just being careful.
    QSerialPort m_serialPort;
    m_serialPort.setPortName(this->m_portName);
    m_serialPort.open(QIODevice::ReadWrite);
    m_serialPort.setBaudRate(QSerialPort::Baud9600);
    m_serialPort.setDataBits(QSerialPort::Data8);
    m_serialPort.setStopBits(QSerialPort::OneStop);
    m_serialPort.setParity(QSerialPort::NoParity);
    m_serialPort.setFlowControl(QSerialPort::NoFlowControl);
    m_serialPort.setRequestToSend(false);  // Make sure the RTS pin is set to off.

    while (true)
    {
        QMutexLocker locker(&m_mutex);

        if (!m_writeQueue.isEmpty())    // check if the WriteQueue is empty.
        {
            // get the sendInfo in the front of the writeQueue <QByteArray, QString, int>--<MessageBytArr, replyStr, readBytesInt>
            QByteArray messageBytArr = std::get<0>(m_writeQueue.head());
            QString replyStr = std::get<1>(m_writeQueue.head());
            int readBytesInt = std::get<2>(m_writeQueue.head());
            m_writeQueue.dequeue(); // remove the sendInfo in the front of writeQueue

            float messageFloat = messageBytArr.toFloat();
            //char c = 65;

            if (replyStr.contains("closePort"))
            {
                break;  // If the first part of the message is None this signals the process to close the port and stop
                // close the serial port and stop; break the while-loop then m_serialPort.close() will be executed.
            }
            else if (int(messageFloat) == 1)
            {
                m_serialPort.setRequestToSend(true);   // trigger a quick fire using the RTS pin
            }
            else if(int(messageFloat) == -1)
            {
                m_serialPort.setRequestToSend(false);  // reset the RTS pin to false
            }
            else    // Otherwise, message is a command string
            {
                if (m_serialPort.readBufferSize() != 0)
                {
                    m_serialPort.clear(QSerialPort::AllDirections); // There shouldn't be any other rubbish in the input buffer,
                    // but here check and clear it just in case.
                }
                try {
                    // try writing to the serial port
                    QString messageDataStr = QString::fromLocal8Bit(messageBytArr.data());
                    m_serialPort.write(messageBytArr);
                    m_serialPort.waitForBytesWritten(300);

                    //qDebug() << "Write: messageBytArr------------------------" << messageBytArr;

                    try {
                        m_serialPort.waitForReadyRead(300);
                        //m_serialPort.read(&c, 1); // read the response from the magstrim device
                        //messageBytArr = (&c);
                        messageBytArr = m_serialPort.read(1);

                        if (messageBytArr.at(0) == 'N') // read version number
                        {
                            while (int(messageBytArr.back()) > 0)
                            {
                                m_serialPort.waitForReadyRead(300);
                                //m_serialPort.read(&c, 1);
                                //messageBytArr.append(c);
                                messageBytArr += m_serialPort.read(1);
                            }
                            m_serialPort.waitForReadyRead(300);
                            //m_serialPort.read(&c, 1); // after the end of the version number, read one more byte to grab the CRC
                            //messageBytArr.append(c);
                            messageBytArr += m_serialPort.read(1);
                        }
                        // If the first byte is not '?', then the message was understood
                        // so carry on reading in the response (if it was a '?', then this will be the only returned byte).
                        else if (messageBytArr.at(0) != '?')
                        {
                            m_serialPort.waitForReadyRead(300);
                            //m_serialPort.read(&c, 1);
                            //messageBytArr.append(c);
                            messageBytArr += m_serialPort.read(1);
                            // If the second returned byte is a '?' or 'S', then the data value supplied either wasn't
                            // acceptable ('?') or the command conflicted with the current settings ('S'), then,
                            // just grab the CRC - otherwise, everything is ok so carry on reading the rest of the message
                            if (messageBytArr.at(1) != '?' && messageBytArr.at(1) != 'S')
                            {
                                while (readBytesInt - 2 > 0)
                                {
                                    m_serialPort.waitForReadyRead(300);
                                    //m_serialPort.read(&c, 1);
                                    //messageBytArr.append(c);
                                    messageBytArr += m_serialPort.read(1);
                                    readBytesInt --;
                                }
                            }
                            else
                            {
                                m_serialPort.waitForReadyRead(300);
                                //m_serialPort.read(&c,1);
                                //messageBytArr.append(c);
                                messageBytArr += m_serialPort.read(1);
                            }
                        }

                        //qDebug() << "Read: messageBytArr------------------------" << messageBytArr;

                        if (!replyStr.isEmpty())
                        {
                            QString messageDataStr = QString::fromLocal8Bit(messageBytArr.data());
                            std::string useless(messageDataStr.toStdString());
                            emit updateReadQueue(std::make_tuple(0, messageBytArr));
                        }
                    }  catch (...) {
                        emit updateReadQueue(std::make_tuple(SerialPortController::SERIAL_READ_ERROR, messageBytArr));
                    }
                }  catch (...) {
                    emit updateReadQueue(std::make_tuple(SerialPortController::SERIAL_WRITE_ERROR, messageBytArr));
                } // try...catch...
            } // else
        } //if (!m_serialWriteQueue.empty())
    } // while-loop


    m_serialPort.close();
}
