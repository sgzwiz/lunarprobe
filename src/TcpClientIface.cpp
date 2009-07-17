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
 *  \file   TcpClientIface.cpp
 *
 *  \brief  Custom TCP Implementation of the ClientIface.
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
#include "TcpClientIface.h"
#include "DebugContext.h"
#include "LuaBindings.h"

LUNARPROBE_NS_BEGIN

//*****************************************************************************
/*!
 *  \brief  Create a TCP based debug server.
 *
 *  \version
 *      - S Panyam  01/04/2009
 *      Initial version.
 */
//*****************************************************************************
TcpClientIface::TcpClientIface(int port) : SServer(port), clientSocket(-1)
{
}

//*****************************************************************************
/*!
 *  \brief  No-op destructor
 *
 *  \version
 *      - S Panyam  01/04/2009
 *      Initial version.
 */
//*****************************************************************************
TcpClientIface::~TcpClientIface()
{
}

//*****************************************************************************
/*!
 *  \brief  Handle debug connections.
 *
 *  \version
 *      - S Panyam  17/07/2009
 *      Initial version.
 */
//*****************************************************************************
void TcpClientIface::HandleConnection(int sock)
{
    // read and handle messages one by one
    std::string message;

    clientSocket = sock;

    // reload lua scripts at the start of each connection
    // GetLuaBindings()->RequestReload();

    while (Running() && ReadString(message))
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

    SServer::HandleConnection(clientSocket);
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
void TcpClientIface::HandleDebugHook(LuaStack pStack, LuaDebug pDebug)
{
    if (clientSocket < 0)
        return ;

    ClientIface::HandleDebugHook(pStack, pDebug);
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
bool TcpClientIface::ReadString(std::string &result)
{
    if (clientSocket >= 0)
    {
        // lock the read mutex
        SMutexLock socketReadLock(socketReadMutex);

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
int TcpClientIface::SendMessage(const char *data, unsigned datasize)
{
    char datasizebuff[8];
    if (clientSocket >= 0)
    {
        // lock the write mutex
        SMutexLock socketWriteLock(socketWriteMutex);

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

LUNARPROBE_NS_END

