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
#include "net/server.h"
#include "net/connfactory.h"
#include "LuaUtils.h"
#include "TcpClientIface.h"

LUNARPROBE_NS_BEGIN

class TcpDebugServer :
        public virtual SServer,
        public virtual SConnFactory
{

public:
    TcpDebugServer(int port, TcpClientIface *pIface);
    virtual ~TcpDebugServer();
    SConnHandler *  NewHandler();
    void            ReleaseHandler(SConnHandler *);

protected:
    TcpClientIface  *   pClientIface;
};

LUNARPROBE_NS_END

#endif

