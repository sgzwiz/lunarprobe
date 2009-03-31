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
 *  \file   HttpClientIface.h
 *
 *  \brief  A http (comet) client interface.
 *
 *  \version
 *        - S Panyam  30/03/2009
 *        Initial version.
 *
 *****************************************************************************/

#ifndef _HTTP_CLIENT_INTERFACE_H_
#define _HTTP_CLIENT_INTERFACE_H_

#include "ClientIface.h"
#include "json/json.h"
#include "eds/http/bayeux/channel.h"
#include "thread/mutex.h"

LUNARPROBE_NS_BEGIN

//*****************************************************************************
/*!
 *  \class  HttpClientIface
 *
 *  \brief  A bayeux (comet) implementation of the debugger.
 *****************************************************************************/
class HttpClientIface : public ClientIface, public SBayeuxChannel
{
public:
    // Constructor
    HttpClientIface(SBayeuxModule *pModule, const std::string &name);

    // Destructor
    ~HttpClientIface();

    // Sends a message to the client
    virtual int     SendMessage(const char *data, unsigned datasize);

    //! Handles an event.
    virtual void HandleEvent(const JsonNodePtr &event, JsonNodePtr &output);

private:
    //! Read lock on the socket
    SMutex              socketReadMutex;

    //! Write lock on the socket
    SMutex              socketWriteMutex;
};

LUNARPROBE_NS_END

#endif

