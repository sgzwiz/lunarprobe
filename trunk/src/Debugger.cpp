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

LUNARPROBE_CPP_NAMESPACE_BEGIN

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
Debugger::Debugger() :
    serverPort(LUA_DEBUG_PORT),
    serverSocket(-1),
    serverStopped(false),
    serverState(SERVER_CREATED),
    clientSocket(-1),
    serverRunningCond(serverStateMutex),
    serverDeadCond(serverStateMutex),
    pLuaBindings(NULL)
{
    // Start the lua server
    StartServer();
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
 *  \brief  Starts the debug server.
 *
 *  \version
 *      - S Panyam  27/10/2008
 *      Initial version.
 */
//*****************************************************************************
int Debugger::StartServer()
{
    CMutexLock stateMutexLock(serverStateMutex);
    if (serverState == SERVER_RUNNING)
    {
        return -1;
    }
    else if (serverState == SERVER_STARTED)
    {
        WaitForServerBegin();
        return -1;
    }
    else if (serverState == SERVER_STOPPED)
    {
        WaitForServerFinish();
    }

    serverState = SERVER_STARTED;

    pthread_create(&serverThread, NULL, ServerThreadFunc, this);

    pthread_detach(serverThread);

    WaitForServerBegin();

    return 0;
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
void Debugger::HandleConnection()
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




///////////////////////////////////////////////////////////////////////////////////
//
//
//                      SERVER THREAD SPECIFIC FUNCTIONS
//
//
///////////////////////////////////////////////////////////////////////////////////

//*****************************************************************************
/*!
 *  \brief  If a thread has been started but not yet in the running state,
 *  this blocks until the thread has got into the running state.
 *
 *  \version
 *      - S Panyam     27/10/2008
 *        Created.
 *
 *****************************************************************************/
int Debugger::WaitForServerBegin()
{
    return serverRunningCond.Wait();
}

//*****************************************************************************
/*!
 *  \brief  If a thread has been stopped but still running, this blocks
 *  until the thread has stopped running.
 *
 *  \version
 *      - S Panyam     27/10/2008
 *        Created.
 *
 *****************************************************************************/
int Debugger::WaitForServerFinish()
{
    return serverDeadCond.Wait();
}

//*****************************************************************************
/*!
 *  \brief  Signals that the thread has gone to the start state.
 *
 *  \version
 *      - S Panyam     27/10/2008
 *        Created.
 *
 *****************************************************************************/
int Debugger::SignalServerBegin()
{
    serverState = SERVER_RUNNING;

    return serverRunningCond.Signal();
}

//*****************************************************************************
/*!
 *  \brief  Signals that the thread has gone to the finished state.
 *
 *  \version
 *      - S Panyam     27/10/2008
 *        Created.
 *
 *****************************************************************************/
int Debugger::SignalServerFinish()
{
    CMutexLock stateMutexLock(serverStateMutex);

    serverState = SERVER_TERMINATED;

    return serverDeadCond.Signal();
}

//*****************************************************************************
/*!
 *  \brief  The actual run function of the Server thread.
 *
 *  \version
 *      - S Panyam     27/10/2008
 *        Created.
 *
 *****************************************************************************/
int Debugger::Run()
{
    // do it all here now

    int result = 0;

    using namespace std;

    // Create an internet socket using streaming (tcp/ip)
    // and save the handle for the server socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        cerr << "=======================================================" << endl;
        cerr << "ERROR: Cannot create server socket: [" << errno << "]: " << strerror(errno) << endl << endl;
        return errno;
    }

    // set it so we can reuse the socket immediately after closing it.
    int reuse = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) != 0)
    {
        cerr << "=======================================================" << endl;
        cerr << "ERROR: setsockopt failed: [" << errno << "]: " << strerror(errno) << endl << endl;
        result = errno;
    }
    else
    {
        int nodelay = 1;
        if (setsockopt(serverSocket, SOL_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) != 0)
        {
            cerr << "=======================================================" << endl;
            cerr << "ERROR: Could not set TCP_NODELAY [" << errno << "]: " << strerror(errno) << endl << endl;
            result = errno;
        }
        else
        {
            // Setup the structure that defines the IP-adress, port and protocol
            // family to use.
            sockaddr_in srv_sock_addr;

            // zero the srv sock addr structure
            bzero(&srv_sock_addr, sizeof(srv_sock_addr));

            // Use the internet domain.
            srv_sock_addr.sin_family = AF_INET;

            // Use this specific port.
            srv_sock_addr.sin_port = htons(serverPort);

            // Use any of the network cards IP addresses.
            srv_sock_addr.sin_addr.s_addr = INADDR_ANY;

            // Bind the socket to the port number specified by (serverPort).
            int retval = bind(serverSocket, (sockaddr*)(&srv_sock_addr), sizeof(sockaddr));
            if (retval != 0)
            {
                cerr << "=======================================================" << endl;
                cerr << "ERROR: Cannot bind to server on port: [" << errno << "]: " << strerror(errno) << endl << endl;
                result = errno;
            }
            else
            {
                // Setup a limit of maximum 10 pending connections.
                retval = listen(serverSocket, 10);
                if (retval < 0)
                {
                    cerr << "=======================================================" << endl;
                    cerr << "ERROR: Cannot listen to connections: [" << errno << "]: " << strerror(errno) << endl;
                    result = errno;
                }
                else
                {
                    // Ok, now loop here indefinitly to service our incoming requests
                    serverStopped = false;

                    // block/ignore SIGPIPE exceptions
                    signal(SIGPIPE, SIG_IGN);

                    while(!serverStopped)
                    {
                        // Accept will block until a request is detected on this socket
                        sockaddr_in client_sock_addr;
                        socklen_t addlen = sizeof(client_sock_addr);

                        // Handle for the socket taking the client request
                        clientSocket    = accept(serverSocket, (sockaddr*)(&client_sock_addr), &addlen);
                        int errnum      = errno;
                        result          = errnum;

                        if (!serverStopped)
                        {
                            if (clientSocket < 0)
                            {
                                cerr << "=======================================================" << endl;
                                cerr << "ERROR: Could not accept connection [" << errnum << "]: " << strerror(errnum) << "." << endl;
                                serverStopped = true;
                            }
                            else
                            {
                                HandleConnection();
                            }
                        }

                        // close the client after done!!
                        if (clientSocket >= 0)
                        {
                            // server being killed and we have a socket, so close the
                            // socket as well
                            shutdown(clientSocket, SHUT_RDWR);
                            close(clientSocket);
                            clientSocket = -1;
                        }
                    }
                }
            }
        }
    }

    if (serverSocket >= 0)
    {
        shutdown(serverSocket, SHUT_RDWR);
        close(serverSocket);
        serverSocket = -1;
    }

    SignalServerFinish();

    return result;
}

void *Debugger::ServerThreadFunc(void *pData)
{
    Debugger *pDebugger = (Debugger *)pData;

    {
        CMutexLock mutexLock(pDebugger->serverStateMutex);

        if (pDebugger->serverState == SERVER_STARTED)
            pDebugger->SignalServerBegin();
    }

    pDebugger->Run();

    pDebugger->SignalServerFinish();

    pthread_exit(NULL);

    return NULL;
}

LUNARPROBE_CPP_NAMESPACE_END

