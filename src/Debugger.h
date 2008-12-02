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

#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#include <map>
#include "LuaUtils.h"
#include "MutexUtils.h"

LUNARPROBE_CPP_NAMESPACE_BEGIN

typedef std::map<LuaStack , DebugContext *> DebugContextMap;

#ifndef LUA_DEBUG_PORT
#define LUA_DEBUG_PORT  9999
#endif

//*****************************************************************************
/*!
 *  \class  Debugger
 *
 *  \brief  The actual remote lua debugger instance.
 *
 *****************************************************************************/
class Debugger
{
    // Make lua debugger our friend so it 
    // can access context maps and so on.
    //
    // TODO: have to rething this - 
    // exposing this so openly is not a great idea.
    friend class LuaBindings;

public:
    // ctor
    Debugger();

    // dtor
    virtual ~Debugger();

    // Sets the server port if the server is NOT running
    bool            SetPort(int port = LUA_DEBUG_PORT);

    // Functions called by the Interface on startup
    bool            StartDebugging(LuaStack lua_stack, const char *name = "");
    bool            StopDebugging(LuaStack lua_stack);
    void            HandleDebugHook(LuaStack pStack, LuaDebug pDebug);


    DebugContext *  GetDebugContext(LuaStack stack);

protected:
    // Generic functions
    DebugContext *  AddDebugContext(LuaStack stack, const char *name = "");

protected:
    DebugContextMap   debugContexts;


    //////////////////  All things to the with the Server   ///////////////////
public:
    // Starts the server
    virtual int StartServer();

    // Tells if the server is running or not
    virtual bool ServerRunning() { return serverState == SERVER_RUNNING; }

    // stops the server
    virtual void StopServer();

    // Sends a string on the socket
    virtual int WriteString(const char *data, unsigned datasize);

protected:
    // Handle a new client connection
    void HandleConnection();

    // Reads a string from the socket
    virtual bool ReadString(std::string &result);

protected:
    typedef enum
    {
        SERVER_CREATED,
        SERVER_STARTED,
        SERVER_RUNNING,
        SERVER_STOPPED,
        SERVER_TERMINATED,
    } ServerState;

    int         serverPort;
    int         serverSocket;
    bool        serverStopped;
    ServerState serverState;
    int         clientSocket;
    pthread_t   serverThread;

private:
    // the real server function
    int                 Run();

    // Get the lua bindings for the debugger
    LuaBindings *       GetLuaBindings();

    // server thread callback on startup
    static void *       ServerThreadFunc(void *pData);

    int                 WaitForServerBegin();
    int                 WaitForServerFinish();
    int                 SignalServerBegin();
    int                 SignalServerFinish();

private:
    CMutex              serverStateMutex;
    CMutex              socketReadMutex;
    CMutex              socketWriteMutex;
    CCondition          serverRunningCond;
    CCondition          serverDeadCond;

    //! the actual lua binding for the debugger exposed to LUA
    LuaBindings *       pLuaBindings;
};

LUNARPROBE_CPP_NAMESPACE_END

#endif

