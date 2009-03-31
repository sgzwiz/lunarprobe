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
 *  \file   DebugServer.h
 *
 *  \brief  A custom TCP based client interface.
 *
 *  \version
 *        - S Panyam  23/10/2008
 *        Initial version.
 *
 *****************************************************************************/

#ifndef _TCP_DEBUG_SERVER_H_
#define _TCP_DEBUG_SERVER_H_

#include "DebugServer.h"
#include "net/connhandler.h"
#include "thread/mutex.h"

LUNARPROBE_NS_BEGIN

//*****************************************************************************
/*!
 *  \class  TcpDebugServer
 *
 *  \brief  A custom tcp implementation of the debugger.
 *****************************************************************************/
class TcpDebugServer : public DebugServer, public SConnHandler
{
public:
    // OVerridden to check client status
    virtual void    HandleDebugHook(LuaStack pStack, LuaDebug pDebug);

    // Sends a message to the connected client
    virtual int     SendMessage(const char *data, unsigned datasize);

    // Handle a new client connection
    bool HandleConnection();

protected:
    // Reads a string from the socket
    virtual bool ReadString(std::string &result);

private:
    //! Read lock on the socket
    SMutex              socketReadMutex;

    //! Write lock on the socket
    SMutex              socketWriteMutex;
};

LUNARPROBE_NS_END

#endif

