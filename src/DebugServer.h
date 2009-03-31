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
 *  \brief  The interface between lunar probe and the client.
 *
 *  \version
 *        - S Panyam  23/10/2008
 *        Initial version.
 *
 *****************************************************************************/

#ifndef _DEBUG_SERVER_H_
#define _DEBUG_SERVER_H_

#include <map>
#include "LuaUtils.h"

LUNARPROBE_NS_BEGIN

typedef std::map<LuaStack , DebugContext *> DebugContextMap;

//*****************************************************************************
/*!
 *  \class  DebugServer
 *
 *  \brief  The interface between lunar probe and the client.
 *
 *****************************************************************************/
class DebugServer
{
public:
    // ctor
    DebugServer();

    // dtor
    virtual ~DebugServer();

    // Functions called by the Interface on startup
    virtual bool    StartDebugging(LuaStack lua_stack, const char *name = "");
    virtual bool    StopDebugging(LuaStack lua_stack);
    virtual void    HandleDebugHook(LuaStack pStack, LuaDebug pDebug);

    DebugContext *  GetDebugContext(LuaStack stack);

    //! Sends a message to the connected client
    virtual int     SendMessage(const char *data, unsigned datasize) = 0;

    //! Get a list of debug contexts
    const DebugContextMap &GetContexts() const;

protected:
    // Generic functions
    DebugContext *  AddDebugContext(LuaStack stack, const char *name = "");

    // Get the lua bindings for the debugger
    LuaBindings *       GetLuaBindings();

protected:
    //! List of debug contexts
    DebugContextMap   debugContexts;

    //! the actual lua binding for the debugger exposed to LUA
    LuaBindings *       pLuaBindings;
};

LUNARPROBE_NS_END

#endif

