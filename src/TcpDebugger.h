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
 *  \file   Debugger.h
 *
 *  \brief  The top level interface for the lua debugger for debugging lua
 *  files as the game is running.
 *
 *  \version
 *        - S Panyam  23/10/2008
 *        Initial version.
 *
 *****************************************************************************/

#ifndef _TCP_DEBUGGER_H_
#define _TCP_DEBUGGER_H_

#include "Debugger.h"
#include "net/connhandler.h"
#include "thread/mutex.h"

LUNARPROBE_NS_BEGIN

//*****************************************************************************
/*!
 *  \class  TcpDebugger
 *
 *  \brief  A custom tcp implementation of the debugger.
 *****************************************************************************/
class TcpDebugger : public Debugger, public SConnHandler
{
public:
    // OVerridden to check client status
    virtual void    HandleDebugHook(LuaStack pStack, LuaDebug pDebug);

    // Sends a message to the connected client
    virtual int     SendMessage(const char *data, unsigned datasize);

protected:
    // Handle a new client connection
    bool HandleConnection();

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

