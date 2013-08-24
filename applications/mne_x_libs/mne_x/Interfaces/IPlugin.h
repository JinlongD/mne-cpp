//=============================================================================================================
/**
* @file     IPlugin.h
* @author   Christoph Dinh <chdinh@nmr.mgh.harvard.edu>;
*           Matti Hamalainen <msh@nmr.mgh.harvard.edu>
* @version  1.0
* @date     February, 2013
*
* @section  LICENSE
*
* Copyright (C) 2013, Christoph Dinh and Matti Hamalainen. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that
* the following conditions are met:
*     * Redistributions of source code must retain the above copyright notice, this list of conditions and the
*       following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
*       the following disclaimer in the documentation and/or other materials provided with the distribution.
*     * Neither the name of the Massachusetts General Hospital nor the names of its contributors may be used
*       to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
* PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MASSACHUSETTS GENERAL HOSPITAL BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*
*
* @brief    Contains declaration of IPlugin interface class.
*
*/

#ifndef IPLUGIN_H
#define IPLUGIN_H

//*************************************************************************************************************
//=============================================================================================================
// INCLUDES
//=============================================================================================================

//#include "../Management/plugininputconnector.h"
//#include "../Management/pluginoutputconnector.h"

// ToDo REMOVE
#include <xMeas/Nomenclature/nomenclature.h>
// ToDo REMOVE End


//*************************************************************************************************************
//=============================================================================================================
// QT INCLUDES
//=============================================================================================================

#include <QThread>
#include <QCoreApplication>
#include <QSharedPointer>


//*************************************************************************************************************
//=============================================================================================================
// DEFINE NAMESPACE MNEX
//=============================================================================================================

namespace MNEX
{


//*************************************************************************************************************
//=============================================================================================================
// ENUMERATIONS
//=============================================================================================================

//=============================================================================================================
/**
* Plugin Type enumeration.
*/
enum Type
{
    _ISensor,       /**< Type for a sensor plugin. */
    _IAlgorithm,    /**< Type for a real-time algorithm plugin. */
    _IIO            /**< Type for a real-time I/O plugin. */
};


//*************************************************************************************************************
//=============================================================================================================
// FORWARD DECLARATIONS
//=============================================================================================================

class PluginInputConnector;
class PluginOutputConnector;


//=========================================================================================================
/**
* DECLARE CLASS IPlugin
*
* @brief The IPlugin class is the base interface class of all plugins.
*/
class IPlugin : public QThread
{
//    Q_OBJECT
public:
    typedef QSharedPointer<IPlugin> SPtr;               /**< Shared pointer type for IPlugin. */
    typedef QSharedPointer<const IPlugin> ConstSPtr;    /**< Const shared pointer type for IPlugin. */

    typedef QVector< QSharedPointer< PluginInputConnector > > InputConnectorList;  /**< List of input connectors. */
    typedef QVector< QSharedPointer< PluginOutputConnector > > OutputConnectorList; /**< List of output connectors. */

    //=========================================================================================================
    /**
    * Destroys the IPlugin.
    */
    virtual ~IPlugin() {};

    //=========================================================================================================
    /**
    * Clone the plugin
    */
    virtual QSharedPointer<IPlugin> clone() = 0;

    //=========================================================================================================
    /**
    * Starts the IPlugin.
    * Pure virtual method.
    *
    * @return true if success, false otherwise
    */
    virtual bool start() = 0;// = 0 call is not longer possible - it has to be reimplemented in child;

    //=========================================================================================================
    /**
    * Stops the IPlugin.
    * Pure virtual method.
    *
    * @return true if success, false otherwise
    */
    virtual bool stop() = 0;

    //=========================================================================================================
    /**
    * Returns the plugin type.
    * Pure virtual method.
    *
    * @return type of the IPlugin
    */
    virtual Type getType() const = 0;

    //=========================================================================================================
    /**
    * Returns the plugin name.
    * Pure virtual method.
    *
    * @return the name of plugin.
    */
    virtual const char* getName() const = 0;

    //=========================================================================================================
    /**
    * Returns the set up widget for configuration of the IPlugin.
    * Pure virtual method.
    *
    * @return the setup widget.
    */
    virtual QWidget* setupWidget() = 0; //setup()

    //=========================================================================================================
    /**
    * Sets the activation status of the plugin.
    *
    * @param [in] status the new activation status of the plugin.
    */
    inline void setStatus(bool status);

    //=========================================================================================================
    /**
    * Returns the activation status of the plugin.
    *
    * @return true if plugin is activated.
    */
    inline bool isActive() const;





// ToDo REMOVE
    //=========================================================================================================
    /**
    * Returns the provider id
    *
    * @return the provider id (Plugin ID).
    */
    inline XMEASLIB::PLG_ID::Plugin_ID getPlugin_ID() const;

    //=========================================================================================================
    /**
    * Returns the widget which is shown under configuration tab while running mode.
    * Pure virtual method.
    *
    * @return the run widget.
    */
    virtual QWidget* runWidget() = 0;
// ToDo REMOVE End


protected:
    //=========================================================================================================
    /**
    * The starting point for the thread. After calling start(), the newly created thread calls this function.
    * Returning from this method will end the execution of the thread.
    * Pure virtual method inherited by QThread
    */
    virtual void run() = 0;





// ToDo REMOVE
    XMEASLIB::PLG_ID::Plugin_ID m_PLG_ID;     /**< Holds the plugin id.*/
// ToDo REMOVE end
private:
    bool m_bStatus;                 /**< The activation status. */
};

//*************************************************************************************************************
//=============================================================================================================
// INLINE DEFINITIONS
//=============================================================================================================


inline void IPlugin::setStatus(bool status)
{
    m_bStatus = status;
}


//*************************************************************************************************************

inline bool IPlugin::isActive() const
{
    return m_bStatus;
}




//*************************************************************************************************************
// ToDo REMOVE
inline XMEASLIB::PLG_ID::Plugin_ID IPlugin::getPlugin_ID() const
{
    return m_PLG_ID;
}
// ToDo REMOVE end

} //Namespace

Q_DECLARE_INTERFACE(MNEX::IPlugin, "mne_x/1.0")

#endif //IPLUGIN_H
