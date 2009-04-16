/*****************************************************************************/
/*!
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *****************************************************************************
 *
 *  \file   BayeuxClientIface.h
 *
 *  \brief  The bayeux channel used to communicate with a http server.
 *
 *  \version
 *        - S Panyam  30/03/2009
 *        Initial version.
 *
 *****************************************************************************/

#ifndef _BAYEUX_CLIENT_IFACE_H_
#define _BAYEUX_CLIENT_IFACE_H_

#include "ClientIface.h"
#include "halley.h"

LUNARPROBE_NS_BEGIN

//*****************************************************************************
/*!
 *  \class  BayeuxClientIface
 *
 *  \brief  A bayeux (comet) implementation of the debugger.
 *****************************************************************************/
class BayeuxClientIface : public ClientIface, public SBayeuxChannel
{
public:
    // Constructor
    BayeuxClientIface(const std::string &name, SBayeuxModule *pMod_ = NULL);

    // Destructor
    ~BayeuxClientIface();

    // Sends a message to the client
    virtual int     SendMessage(const char *data, unsigned datasize);

    //! Handles an event.
    virtual void HandleEvent(const JsonNodePtr &event, JsonNodePtr &output);

private:
    //! Write lock on the socket
    SMutex              socketWriteMutex;
};

LUNARPROBE_NS_END

#endif

