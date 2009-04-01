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
 *  \file   DebugServer
 *
 *  \brief  The servers that handle connections and use client interfaces
 *  to process the messages.
 *
 *  \version
 *        - S Panyam  23/10/2008
 *        Initial version.
 *
 *****************************************************************************/

#ifndef _DEBUG_SERVER_H_
#define _DEBUG_SERVER_H_

#include <map>
#include "halley.h"
#include "LuaUtils.h"
#include "TcpClientIface.h"
#include "BayeuxClientIface.h"

#ifndef LUA_DEBUG_PORT
#define LUA_DEBUG_PORT  9999
#endif

LUNARPROBE_NS_BEGIN

class TcpDebugServer :
        public virtual SServer,
        public virtual SConnFactory
{

public:
                            TcpDebugServer(int port, TcpClientIface *pIface);
    virtual                 ~TcpDebugServer();
    SConnHandler *          NewHandler();
    void                    ReleaseHandler(SConnHandler *);
    inline TcpClientIface * GetClientIface()    { return pClientIface; }

protected:
    TcpClientIface  *   pClientIface;
};

class HttpDebugServer : public virtual SEvServer
{
public:
    HttpDebugServer(int                 port,
                    BayeuxClientIface * pIface      = NULL,
                    const std::string & msgBoundary = "LUNARPROBE_MESSAGE_BOUNDARY");
    virtual ~HttpDebugServer();

    inline SHttpReaderStage *  GetReaderStage()    { return &requestReader; }
    inline SHttpHandlerStage * GetHandlerStage()   { return &requestHandler; }
    inline SWriterModule *     GetWriterModule()   { return &writerModule; }
    inline SContentModule *    GetContentModule()  { return &contentModule; }
    inline SBayeuxModule *     GetBayeuxModule()   { return &bayeuxModule; }
    inline SFileModule *       GetFileModule()     { return &fileModule; }
    inline SUrlRouter *        GetUrlRouter()      { return &urlRouter; }
    inline BayeuxClientIface * GetClientIface()    { return pClientIface; }

protected:
    BayeuxClientIface  *pClientIface;

    SHttpReaderStage    requestReader;
    SHttpHandlerStage   requestHandler;
    SWriterModule       writerModule;
    SContentModule      contentModule;
    SBayeuxModule       bayeuxModule;
    SFileModule         fileModule;
    SUrlRouter          urlRouter;
};

LUNARPROBE_NS_END

#endif

