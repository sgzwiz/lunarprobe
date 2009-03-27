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
 *  \file   Debugger.cpp
 *
 *  \brief  Implementation of the Debugger.
 *
 *  \version
 *      - S Panyam   05/12/2008
 *      Initial version.
 */
//*****************************************************************************


#include <iostream> 
#include <string> 
#include <sstream> 
#include <errno.h>
#include <assert.h>
#include <netdb.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/sendfile.h>

#include "lpfwddefs.h"
#include "Debugger.h"
#include "DebugContext.h"
#include "LuaBindings.h"

LUNARPROBE_NS_BEGIN

//*****************************************************************************
/*!
 *  \brief  Creates a new instance of the lua debugger.
 *
 *  \param  The port on which the server will be started.
 *
 *  \version
 *      - S Panyam  27/10/2008
 *      Initial version.
 */
//*****************************************************************************
Debugger::Debugger() : pLuaBindings(NULL)
{
}

//*****************************************************************************
/*!
 *  \brief  Destroys the debugger.
 *
 *  \version
 *      - S Panyam  27/10/2008
 *      Initial version.
 */
//*****************************************************************************
Debugger::~Debugger()
{
    StopServer();

    // remove any debug contexts that wasnt removed (via StopDebugging)
    for (DebugContextMap::iterator iter = debugContexts.begin();iter != debugContexts.end(); ++iter)
    {
        DebugContext *ctx = iter->second;
        if (ctx != NULL)
            delete ctx;
    }

    if (pLuaBindings != NULL)
        delete pLuaBindings;
}

//*****************************************************************************
/*!
 *  \brief  Sets the port on which the server will be started.
 *
 *  Only will be set if the server is stopped.  
 *
 *  \return true if port set false otherwise.
 *
 *  \version
 *      - S Panyam  18/11/2008
 *      Initial version.
 */
//*****************************************************************************
bool Debugger::SetPort(int port)
{
    if (ServerRunning())
        return false;

    serverPort = port;
    return true;
}

//*****************************************************************************
/*!
 *  \brief  Starts debugging of a lua stack.
 *
 *  \version
 *      - S Panyam  23/10/2008
 *      Initial version.
 */
//*****************************************************************************
bool Debugger::StartDebugging(LuaStack pStack, const char *name)
{
    DebugContext *pContext = GetDebugContext(pStack);
    if (pContext != NULL)
    {
        return false;
    }

    pContext = AddDebugContext(pStack, name);

    return true;
}


//*****************************************************************************
/*!
 *  \brief  Stops debugging of a lua stack.
 *
 *  \version
 *      - S Panyam  23/10/2008
 *      Initial version.
 */
//*****************************************************************************
bool Debugger::StopDebugging(LuaStack pStack)
{
    DebugContextMap::iterator iter = debugContexts.find(pStack);

    if (iter != debugContexts.end())
    {
        DebugContext *pContext = iter->second;

        debugContexts.erase(iter);

        GetLuaBindings()->ContextRemoved(pContext);

        delete pContext;
    }

    return true;
}

//*****************************************************************************
/*!
 *  \brief  Gets the debug context associated with a lua_State object.
 *
 *  \version
 *      - S Panyam  23/10/2008
 *      Initial version.
 */
//*****************************************************************************
DebugContext *Debugger::GetDebugContext(LuaStack pStack)
{
    DebugContextMap::iterator iter = debugContexts.find(pStack);
    if (iter == debugContexts.end())
        return NULL;

    return iter->second;
}

//*****************************************************************************
/*!
 *  \brief  Adds a new lua stack to the list of stacks being debugged.
 *
 *  \param  LuaStack  The new lua stack to be debugged.
 *
 *  \return The DebugContext object that maintains the debugger state
 *  for this stack.
 *
 *  \version
 *      - S Panyam  23/10/2008
 *      Initial version.
 */
//*****************************************************************************
DebugContext *Debugger::AddDebugContext(LuaStack pStack, const char *name)
{
    DebugContext *pContext = new DebugContext(pStack, name);
    debugContexts[pStack] = pContext;

    GetLuaBindings()->ContextAdded(pContext);

    return pContext;
}

//*****************************************************************************
/*!
 *  \brief  Called by the liblua (actually via LunarProbe::HookFunction)
 *  when a breakpoint is hit.
 *
 *  \version
 *      - S Panyam  27/10/2008
 *      Initial version.
 */
//*****************************************************************************
void Debugger::HandleDebugHook(LuaStack pStack, LuaDebug pDebug)
{
    if (clientSocket < 0)
        return ;

    DebugContext *pContext = GetDebugContext(pStack);

    if (pContext == NULL)
    {
        assert(" ===================    DebugContext cannot be NULL.  " && pContext != NULL);
        return ;
    }

    // get the stack info about the curr function
    lua_getinfo(pStack, "nSluf", pDebug);
    lua_pop(pStack, 1);     // pop the name of the function off the stack

    // ignore non-lua functions
    if (pDebug->what == NULL || strcasecmp(pDebug->what, "lua") != 0)
    {
        // we are in a non-lua file so return straight away!
        // TODO: Should we alert someone or do SOMETHING here?
        return ;
    }

    switch (pDebug->event)
    {
        case LUA_HOOKCALL:
        {
            if (pDebug->name != NULL)
            {
                GetLuaBindings()->HandleBreakpoint(pContext, pDebug);
            }
        } break ;
        case LUA_HOOKLINE:
        {
            if (pDebug->source[0] == '@')
            {
                GetLuaBindings()->HandleBreakpoint(pContext, pDebug);
            }
        } break ;
        case LUA_HOOKRET:
        {
            GetLuaBindings()->HandleBreakpoint(pContext, pDebug);
        } break ;
        case LUA_HOOKCOUNT:
        {
            /*
            std::cerr << "========================================================" << std::endl << std::endl
                      << "LUA_HOOKCOUNT not yet handled..........." << std::endl << std::endl
                      << "========================================================" << std::endl << std::endl;
            */
        } break ;
        case LUA_HOOKTAILRET:
        {
            // can come here with a "step", "next" or "return"
            // leave it out for now
            std::cerr << "========================================================" << std::endl << std::endl
                      << "LUA_HOOKTAILRET not yet handled ..........." << std::endl << std::endl
                      << "========================================================" << std::endl << std::endl;
        } break ;
    }
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////   Server related Stuff   ///////////////////////////
///////////////////////////////////////////////////////////////////////////////


//*****************************************************************************
/*!
 *  \brief  Stops the debug server.
 *
 *  \version
 *      - S Panyam  27/10/2008
 *      Initial version.
 */
//*****************************************************************************
void Debugger::StopServer()
{
    if (serverSocket >= 0)
    {
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
    }

    serverSocket = -1;
}

//*****************************************************************************
/*!
 *  \brief  Here is where a client connection to the debugger is handled.
 *
 *  \version
 *      - S Panyam  27/10/2008
 *      Initial version.
 */
//*****************************************************************************
bool Debugger::HandleConnection()
{
    // read and handle messages one by one
    std::string message;

    // reload lua scripts at the start of each connection
    // GetLuaBindings()->RequestReload();

    while (ReadString(message))
    {
        GetLuaBindings()->HandleMessage(message);
    }

    // and go through all paused contexts and
    // resume them!!
    for (DebugContextMap::iterator iter = debugContexts.begin();
         iter != debugContexts.end(); ++iter)
    {
        DebugContext *ctx = iter->second;
        if (ctx != NULL)
            ctx->Resume();
    }

    return true;
}


//*****************************************************************************
/*!
 *  \brief  Reads a string from the socket.  
 *
 *  Read a string from the socket.  All strings will be ascii only and
 *  preceeded by the size (also in ascii).  Remember this is simple
 *  protocol for transferring commands and nothing fancy.  Even binary data
 *  can be sent with 64bit encoding without much loss.
 *
 *  Blocks till a complete string is read.
 *
 *  \return 0 if successfull, anything else if no more messages and/or
 *  channel is closed.
 *
 *  \version
 *      - S Panyam  04/11/2008
 *      Initial version.
 */
//*****************************************************************************
bool Debugger::ReadString(std::string &result)
{
    if (clientSocket >= 0)
    {
        // lock the read mutex
        CMutexLock socketReadLock(socketReadMutex);

        const unsigned MAX_PARAM_SIZE = 1024;
        char param[MAX_PARAM_SIZE + 1];
        unsigned paramsize;
        char paramsizebuff[8];

        ssize_t nread = recv(clientSocket, paramsizebuff, 4, MSG_WAITALL);
        if (nread <= 0)
            return false;

        paramsize = (((paramsizebuff[0]) & 0xff) |
                     (((paramsizebuff[1]) & 0xff) << 8) |
                     (((paramsizebuff[2]) & 0xff) << 16) |
                     (((paramsizebuff[3]) & 0xff) << 24));

        std::ostringstream strstream;
        while (paramsize > MAX_PARAM_SIZE)
        {
            if (recv(clientSocket, param, MAX_PARAM_SIZE, MSG_WAITALL) <= 0)
                return false;
            param[MAX_PARAM_SIZE] = 0;
            strstream << param;
        }

        if (paramsize > 0)
        {
            if (recv(clientSocket, param, paramsize, MSG_WAITALL) <= 0)
                return false;
            param[paramsize] = 0;
            strstream << param;
        }

        result = strstream.str();
        return true;
    }
    return false;
}


//*****************************************************************************
/*!
 *  \brief  Sends a string to the client.  
 *
 *  Writes a string to a socket - string is written as (len/data) pair All
 *  strings will be ascii only and preceeded by the size (also in ascii).
 *  Remember this is simple protocol for transferring commands and nothing
 *  fancy.  Even binary data can be sent with 64bit encoding without much
 *  loss of bandwidth.
 *
 *  \version
 *      - S Panyam  04/11/2008
 *      Initial version.
 */
//*****************************************************************************
int Debugger::WriteString(const char *data, unsigned datasize)
{
    char datasizebuff[8];
    if (clientSocket >= 0)
    {
        // lock the write mutex
        CMutexLock socketWriteLock(socketWriteMutex);

        datasizebuff[0] = ((datasize)       & 0xff);
        datasizebuff[1] = ((datasize >> 8)  & 0xff);
        datasizebuff[2] = ((datasize >> 16) & 0xff);
        datasizebuff[3] = ((datasize >> 24) & 0xff);

        if (send(clientSocket, datasizebuff, 4, 0) < 0)
            return -1;

        int result = send(clientSocket, data, datasize, 0);
        return result;
    }

    return -1;
}

//*****************************************************************************
/*!
 *  \brief  Gets the instance of the lua side of debugger.
 *
 *  \return The lua bindings for the debugger.
 *
 *  \version
 *      - S Panyam  12/11/2008
 *      Initial version.
 */
//*****************************************************************************
LuaBindings *Debugger::GetLuaBindings()
{
    if (pLuaBindings == NULL)
    {
        pLuaBindings = new LuaBindings(this);
    }
    return pLuaBindings;
}

LUNARPROBE_NS_END

