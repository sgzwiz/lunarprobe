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

// Sends a message to the client
int Debugger::SendMessage(const char *data, unsigned datasize)
{
    return 0;
}

//*****************************************************************************
/*!
 *  \brief  Gets the debug contexts
 *
 *  \version
 *      - S Panyam  23/10/2008
 *      Initial version.
 */
//*****************************************************************************
const DebugContextMap &Debugger::GetContexts() const
{
    return debugContexts;
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

