//=============================================================================================================
/**
* @file     connectionrobot.cpp
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
* @brief    Definition of the ConnectionRobot class.
*/

//=============================================================================================================
// INCLUDES
//=============================================================================================================

#include "connectionrobot.h"

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

ConnectionRobot::ConnectionRobot(QQueue<sendInfo> writeQueue,
                                 QQueue<float> updateRobotQueue)
{
    this->m_writeQueue = writeQueue;
    this->m_connectionCommand = std::make_tuple(",", "", 0);
    this->m_updateRobotQueue = updateRobotQueue;
    this->m_stopped = false;
    this->m_paused = true;
    this->m_nextPokeTime = NAN;

    // Quit when message arrived in Queue
    QObject::connect(this, &ConnectionRobot::readInfo,
            &this->m_eventloop, &QEventLoop::quit, Qt::ConnectionType::DirectConnection);
    // Quit when countdown
    QObject::connect(&this->m_timer, &QTimer::timeout,
                     &this->m_eventloop, &QEventLoop::quit, Qt::ConnectionType::DirectConnection);
}

//=============================================================================================================

ConnectionRobot::~ConnectionRobot()
{

}

//=============================================================================================================

void ConnectionRobot::setCommand(sendInfo connectionCommand)
{
    // This locker will lock the mutex until it is destroyed, i.e. when this function call goes out of scope
    QMutexLocker locker(&m_mutex);
    this->m_connectionCommand = connectionCommand;
}

//=============================================================================================================

void ConnectionRobot::updateUpdateRobotQueue(const float info)
{
    // info: message for Robot (NAN: stop; -1: pause; 0: go on; 1: slow down; 2: speed up)
    this->m_updateRobotQueue.enqueue(info);
    emit readInfo();
}

//=============================================================================================================

void ConnectionRobot::run()
{
    // This sends an "enable remote control" command to the serial port controller every 500ms (if armed)
    // or 5000ms (if disarmed); only runs once the stimulator is armed.
    double pokeLatency = 5;

    while (true)
    {
        QMutexLocker locker(&m_mutex);
        // If the robot is currently paused, wait until we get a None (stop) or a non-negative number (start/resume) in the queue
        while(this->m_paused)
        {
            if (!this->m_updateRobotQueue.isEmpty())
            {
                //float message = this->m_updateRobotQueue.head();
                float message = this->m_updateRobotQueue.dequeue();

                // info: message for Robot (NAN: stop; -1: pause; 0: go on; 1: slow down; 2: speed up)
                if (std::isnan(message))    // message = NAN: stop the connection
                {
                    this->m_stopped = true;
                    this->m_paused = false;
                }
                else if (int(message) >= 0) // message = 0, 1, or 2
                {
                    if (int(message) == 2)  // message = 2: speed up
                    {
                        pokeLatency = 0.5;
                        // If message is a 2, that means we've just armed so speed up the poke latency
                        // (not sure that's possible while paused, but just in case)
                    }
                    else if (int(message) == 1) // message = 1: slow down
                    {
                        pokeLatency = 5;
                    }
                    this->m_paused = false;
                }
            } // if (!this->m_updateRobotQueue.empty())
        } // while(this->m_paused)

        if(this->m_stopped)
        {
            break;  // Check if we're stopping the robot, break the while-loop
        }

        this->m_nextPokeTime = clock() + pokeLatency * CLOCKS_PER_SEC; // Update next poll time to next poke latency
        bool interrupted = false;
        while (clock() < this->m_nextPokeTime) // While waiting for next poll...
        {
            if (!this->m_updateRobotQueue.isEmpty())
            {
                // Check to see if there has been an update send from the parent MagStim object
                //float message = this->m_updateRobotQueue.head();
                float message = this->m_updateRobotQueue.dequeue();

                if (std::isnan(message))
                {
                    this->m_stopped = true;
                    interrupted = true;
                    break;
                }
                else if (int(message) == -1)
                {
                    // If message is -1, we've relinquished remote control so signal the process to pause
                    pokeLatency = 5;
                    this->m_paused = true;
                    interrupted = true;
                    break;
                }
                else
                {
                    // Any other message signals a command has been sent to the serial port controller
                    if (int(message) == 2)
                    {
                        // If message is 2, we've just armed so speed up the poke latency
                        pokeLatency = 0.5;
                    }
                    else if (int(message) == 1)
                    {
                        // If message is 1, we've just disarmed so slow down the poke latency
                        pokeLatency = 5;
                    }
                    this->m_nextPokeTime = clock() + pokeLatency * CLOCKS_PER_SEC;
                }
            } //if (!this->m_updateRobotQueue.empty())
        } //while (clock() < this->m_nextPokeTime)


        if  (clock() >= this->m_nextPokeTime && !interrupted)
        {
            // Send emssage if not stopped or paused
            emit this->updateWriteQueue(this->m_connectionCommand);
        }
    } // while (true)
    return;
}
