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
 *  \file   LuaBindings.cpp
 *
 *  \brief  Lua bindings implementation for the ClientIface.
 *
 *  \version
 *      - S Panyam   28/10/2008
 *      Initial version.
 */
//*****************************************************************************

#include <string>
#include <sstream>
#include <iostream>

#include "lpmain.h"
#include "halley.h"

LUNARPROBE_NS_BEGIN

std::string  LuaBindings::LUA_SRC_LOCATION  = "shared/libgameengine/lua/debugger/";
std::string  LuaBindings::LUA_VAR_PREFIX    = "__LDB_";

DebugContext *GetContextIfPaused(LuaStack stack)
{
    DebugContext *  pDebugContext   = (DebugContext *)lua_touserdata(stack, 1);
    if (pDebugContext->running)
    {
        lua_pushinteger(stack, -1);
        lua_pushstring(stack, "Stack is not paused.");

        return NULL;
    }

    return pDebugContext;
}

//*****************************************************************************
/*!
 *  \brief  Constructor
 *
 *  Creates the lua bindings for the debugger.
 *
 *  \param  debugger    The debugger object we will be serving.
 *
 *  \version
 *      - S Panyam  27/10/2008
 *      Initial version.
 */
//*****************************************************************************
LuaBindings::LuaBindings(ClientIface *debugger) :
    pClientIface(debugger),
    pStack(NULL),
    reloadRequested(true),
    dbgStackMutex(PTHREAD_MUTEX_RECURSIVE_NP)
{
}

//*****************************************************************************
/*!
 *  \brief  Destructor.
 *
 *  Closes the debugger lua stack and finishes up.
 *
 *  \version
 *      - S Panyam  27/10/2008
 *      Initial version.
 */
//*****************************************************************************
LuaBindings::~LuaBindings()
{
    // close the lua stack
    if (pStack != NULL)
    {
        LunarProbe::GetInstance()->Detach(pStack);
        lua_close(pStack);
    }
}

//*****************************************************************************
/*!
 *  \brief  Registers and exposes the necessary methods to LUA.
 *
 *  \param  stack   -   The stack to which the methods are to be exposed.
 *
 *  \version
 *      - S Panyam  27/10/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaBindings::Register(LuaStack stack)
{
    // register the functions
    static const struct luaL_reg lib[] = 
    {
        // Sends a message on the socket
        { "WriteString", LuaBindings::WriteString },
        { "Resume", LuaBindings::Resume },
        { "Reload", LuaBindings::Reload },
        { "ListDir", LuaBindings::ListDir},
        { "LoadFile", LuaBindings::LoadFile },
        { "GetContexts", LuaBindings::GetContexts },
        { "EvaluateString", LuaBindings::EvaluateString },
        { "GetLocals", LuaBindings::GetLocals },
        { "GetLocal", LuaBindings::GetLocal},
        { "GetUpValues", LuaBindings::GetUpValues },
        { "GetUpValue", LuaBindings::GetUpValue},
        { "SetLocal", LuaBindings::SetLocal},
        { "SetUpValue", LuaBindings::SetUpValue},
        { NULL, NULL }
    };
    luaL_openlib(stack, "DebugLib", lib, 0);

    return 1;
}

//*****************************************************************************
/*!
 *  \brief  Gets the lua stack instance in the debugger.  Creates one if it
 *  does not exist.
 *
 *  \version
 *      - S Panyam  12/11/2008
 *      Initial version.
 */
//*****************************************************************************
LuaStack LuaBindings::GetLuaStack()
{
    if (reloadRequested || pStack == NULL)
    {
        if (pStack == NULL)
        {
            pStack = LuaUtils::NewLuaStack(true, false);

            // add the location of the lua files into the path!
            std::string strLuaPackagePath
                = std::string("package.path = package.path .. \";lua/?.lua;") + LUA_SRC_LOCATION + std::string("/?.lua\"");

            if (LuaUtils::RunLuaString(pStack, strLuaPackagePath.c_str(), strLuaPackagePath.size()) == 0)
            {
                Register(pStack);
            }
        }

        std::string mainlua = LUA_SRC_LOCATION + "/Main.lua";
        if (LuaUtils::RunLuaScript(pStack, mainlua.c_str()) == 0)
        {
            reloadRequested = false;

            // now register this one for debugging!
            // we have to do it here after all the above to avoid getting 
            // infinite recursive calls.
            // LunarProbe::GetInstance()->Attach(pStack, "debugger");
        }
    }

    return pStack;
}

//*****************************************************************************
/*!
 *  \brief  Called to request a reload of the lua scripts.
 *
 *  \version
 *      - S Panyam  27/10/2008
 *      Initial version.
 */
//*****************************************************************************
void LuaBindings::RequestReload()
{
    reloadRequested = true;
}

//*****************************************************************************
/*!
 *  \brief  Calls a lua function with arbitrary parameters.
 *
 *  \param  funcname    Name of the lua function to call.
 *  \param  funcsig     Signature of the funtion.
 *  \param  Input parameters followed by pointers to output result holders.
 *
 *  \version
 *      - S Panyam  04/11/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaBindings::CallLuaFunc(const char *funcname, const char *funcsig, ...)
{
    SMutexLock mutexLock(dbgStackMutex);

    va_list vl;
    va_start(vl, funcsig);
    int result = LuaUtils::VCallLuaFunc(GetLuaStack(), funcname, funcsig, vl);
    va_end(vl);
    return result;
}


//*****************************************************************************
/*!
 *  \brief  Called by the debugger to tell LUA that a context has been added.
 *
 *  \param  The context being added.
 *
 *  \version
 *      - S Panyam  12/11/2008
 *      Initial version.
 */
//*****************************************************************************
void LuaBindings::ContextAdded(DebugContext *pContext)
{
    if (CallLuaFunc("ContextAdded", "uus", this, pContext, pContext->name.c_str()) != 0)
    {
        // request a reload so that we give the user an opportunity to fix
        // any issues that have arisen from the source file.
        RequestReload();

        // and send out a "failure" message
        std::string error_msg = "{'code': -1, 'value': 'Unknown error in lua file.'}";
        pClientIface->SendMessage(error_msg.c_str(), error_msg.size());
    }
}

//*****************************************************************************
/*!
 *  \brief  Called by the debugger to tell LUA that a context has been removed.
 *
 *  \param  The context being removed.
 *
 *  \version
 *      - S Panyam  12/11/2008
 *      Initial version.
 */
//*****************************************************************************
void LuaBindings::ContextRemoved(DebugContext *pContext)
{
    if (CallLuaFunc("ContextRemoved", "uu", this, pContext) != 0)
    {
        // request a reload so that we give the user an opportunity to fix
        // any issues that have arisen from the source file.
        RequestReload();

        // and send out a "failure" message
        std::string error_msg = "{'code': -1, 'value': 'Unknown error in lua file.'}";
        pClientIface->SendMessage(error_msg.c_str(), error_msg.size());
    }
}

//*****************************************************************************
/*!
 *  \brief  Called (by the debugger) when a breakpoint is hit.  
 *
 *  At this point, the server notifies the client of this hit and process
 *  further commands from the client.  The breakpoint servicing would only
 *  finish, when the client performs one of the program flow actions (eg
 *  "continue", "step", "next" etc).
 *
 *  Note that when this function is called (by the ClientIface), the
 *  LuaStack associated with that Context (or thread) will be paused, until
 *  this function is returned from and hence the wait loop in the function.
 *
 *  It is upto the debug server (by itself or via the client) to resume the
 *  pContext for the context to proceed otherwise, the ClientIface will
 *  wait indefinitely until pContext->Resume() is invoked.  Note that this
 *  can happen in this function or anywhere else.  It must happen sometime
 *  that is all.
 *
 *  \version
 *      - S Panyam  27/10/2008
 *      Initial version.
 */
//*****************************************************************************
void LuaBindings::HandleBreakpoint(DebugContext *pContext, lua_Debug *pDebug)
{
    int handled;
    if (CallLuaFunc("HandleBreakpoint", "uusissssiiii>b",
                    this, pContext, pContext->name.c_str(),
                    pDebug->event, pDebug->name ? pDebug->name : "",
                    pDebug->namewhat ? pDebug->namewhat : "",
                    pDebug->what ? pDebug->what : "",
                    pDebug->source ? pDebug->source : "",
                    pDebug->currentline, pDebug->nups,
                    pDebug->linedefined, pDebug->lastlinedefined, &handled) != 0)
    {
        // request a reload so that we give the user an opportunity to fix
        // any issues that have arisen from the source file.
        RequestReload();
        return ;
    }

    if (! handled)
    {
        // pause the context since LUA has indicated so
        pContext->Pause(pDebug);

        // wait till this context is resumed (as a result of a client action)
        pContext->WaitWhilePaused();
    }
}

//*****************************************************************************
/*!
 *  \brief  Called by the debugger to notify LUA to handle a client message.
 *
 *  \version
 *      - S Panyam  27/10/2008
 *      Initial version.
 */
//*****************************************************************************
void LuaBindings::HandleMessage(const std::string &message)
{
    if (CallLuaFunc("HandleMessage", "us", this, message.c_str()) != 0)
    {
        // request a reload so that we give the user an opportunity to fix
        // any issues that have arisen from the source file.
        RequestReload();

        // and send out a "failure" message
        std::string error_msg = "{'code': -1, 'value': 'Unknown error in lua file.'}";
        pClientIface->SendMessage(error_msg.c_str(), error_msg.size());
    }
}

//*****************************************************************************
/*!
 *  \brief  Called by the debugger to notify LUA to handle a client message
 *  in pure json.  
 *
 *  \version
 *      - S Panyam  26/06/2009
 *      Initial version.
 */
//*****************************************************************************
void LuaBindings::HandleMessage(const JsonNodePtr &message, std::string &output)
{
    output = "null";
    if (CallLuaFunc("HandleMessage", "uj>s", this, message.Data(), &output) != 0)
    {
        // request a reload so that we give the user an opportunity to fix
        // any issues that have arisen from the source file.
        RequestReload();

        // and send out a "failure" message
        std::string error_msg = "{'code': -1, 'value': 'Unknown error in lua file.'}";
        pClientIface->SendMessage(error_msg.c_str(), error_msg.size());
    }
}


//*****************************************************************************
/*!
 *  \brief  Called by LUA to resume a particular debug context.
 *
 *  \luaparam   context -   The context to be resumed.
 *
 *  \version
 *      - S Panyam  06/11/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaBindings::Resume(LuaStack stack)
{
    DebugContext *  pDebugContext   = (DebugContext *)lua_touserdata(stack, 1);
    pDebugContext->Resume();
    return 0;
}

//*****************************************************************************
/*!
 *  \brief  Called by LUA to force a reload of the scripts.
 *
 *  \luaparam   debugger    -   The lua debugger whose client is to be notified.
 *
 *  \version
 *      - S Panyam  27/10/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaBindings::Reload(LuaStack stack)
{
    LuaBindings *   pLuaBindings    = (LuaBindings *)lua_touserdata(stack, 1);
    pLuaBindings->RequestReload();
    return 0;
}

//*****************************************************************************
/*!
 *  \brief  Called by LUA to sends a string on the socket to the client
 *
 *  \luaparam   debugger    -   The lua debugger whose client is to be notified.
 *  \luaparam   message     -   The message to send to the client.
 *
 *  \version
 *      - S Panyam  06/11/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaBindings::WriteString(LuaStack stack)
{
    size_t          length;
    LuaBindings *   pLuaBindings    = (LuaBindings *)lua_touserdata(stack, 1);
    const char *    msg             = lua_tolstring(stack, 2, &length);
    pLuaBindings->pClientIface->SendMessage(msg, length);
    return 0;
}

//*****************************************************************************
/*!
 *  \brief  Get a file listing of a folder.
 *
 *  \luaparam   debugger    -   The lua debugger whose contexts are to be retrieved.
 *  \luaparam   directory   -   The directory to be listed.
 *
 *  \version
 *      - S Panyam  12/11/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaBindings::ListDir(LuaStack stack)
{
    const char *    directory   = (const char *)lua_tostring(stack, 1);
    std::vector<DirEnt> entries;

    if (SFileModule::ReadDirectory(directory, entries))
    {
        // lua_newtable(stack);
        lua_pushinteger(stack, 0);
        // lua_setfield(stack, -2, "code");
        lua_newtable(stack);

        std::vector<DirEnt>::iterator iter = entries.begin();
        int nitems = 0;
        for (;iter != entries.end(); ++iter)
        {
            bool isdir = (iter->entStat.st_mode & S_IFDIR) != 0;

            // push the new item index
            lua_pushinteger(stack, ++nitems);

            // push a table to hold the entry info
            lua_newtable(stack);

            lua_pushstring(stack, iter->entName.c_str());
            lua_setfield(stack, -2, "name");

            lua_pushboolean(stack, isdir);
            lua_setfield(stack, -2, "isdir");

            lua_pushinteger(stack, iter->entStat.st_size);
            lua_setfield(stack, -2, "size");

            lua_settable(stack, -3);
        }
        // lua_setfield(stack, -2, "value");
    }
    else
    {
        lua_pushinteger(stack, 0);
        // lua_setfield(stack, -2, "code");
        lua_pushstring(stack, "Could not read directory.");
        // lua_setfield(stack, -2, "value");
    }

    return 2;
}

//*****************************************************************************
/*!
 *  \brief  Get a list of contexts currently being debugged.
 *
 *  \luaparam   debugger    -   The lua debugger whose contexts are to be retrieved.
 *  \luaparam   context     -   The context on which the file is to be loaded.
 *  \luaparam   filename    -   Name of the file to be loaded.
 *
 *  \version
 *      - S Panyam  12/11/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaBindings::LoadFile(LuaStack stack)
{
    DebugContext *  pContext    = (DebugContext *)lua_touserdata(stack, 1);
    const char *    filename    = (const char *)lua_tostring(stack, 2);

    pContext->LoadFile(filename);

    return 0;
}


//*****************************************************************************
/*!
 *  \brief  Loads a file on a context
 *
 *  \luaparam   debugger    -   The lua debugger whose client is to be notified.
 *  \luaparam   context     -   The context on which the file is to be loaded.
 *  \luaparam   filename    -   Name of the file to be loaded.
 *
 *  \version
 *      - S Panyam  12/11/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaBindings::GetContexts(LuaStack stack)
{
    LuaBindings *   pLuaBindings    = (LuaBindings *)lua_touserdata(stack, 1);
    ClientIface *   pClientIface    = pLuaBindings->pClientIface;
    int nitems = 1;

    lua_newtable(stack);

    for (DebugContextMap::const_iterator iter = pClientIface->GetContexts().begin();
         iter != pClientIface->GetContexts().end();
         ++iter)
    {
        DebugContext *pContext = iter->second;
        if (pContext)
        {
            lua_pushinteger(stack, nitems++);

            lua_newtable(stack);
            lua_pushlightuserdata(stack, pContext);
            lua_setfield(stack, -2, "context");

            lua_pushstring(stack, pContext->name.c_str());
            lua_setfield(stack, -2, "name");

            lua_pushboolean(stack, pContext->running);
            lua_setfield(stack, -2, "running");

            lua_settable(stack, -3);
        }
    }

    return 1;   // the table
}


//*****************************************************************************
/*!
 *  \brief  Evaluate a string and return the result.
 *
 *  \luaparam   context     -   The context to be resumed.
 *  \luaparam   expr_str    -   The string to evaluate.
 *
 *  \version
 *      - S Panyam  01/12/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaBindings::EvaluateString(LuaStack stack)
{
    DebugContext *  pDebugContext   = GetContextIfPaused(stack);
    if (pDebugContext != NULL)
    {
        const char *    expr_str        = lua_tostring(stack, 2);

        // we essentially do the statement:
        // LUA_VAR_PREFIX_tempResult = expr_str
        // then push the value of getglobal(LUA_VAR_PREFIX_tempResult)
        // on to the debug stack.

        std::string temp_result_var = LUA_VAR_PREFIX + "_temp_result";
        std::string global_name     = std::string("_G[\"") + temp_result_var + "\"]";
        std::stringstream realexprstr;
        realexprstr << global_name << " = " << expr_str;

        // TODO: should this be mutexed??
        int retCode = LuaUtils::RunLuaString(pDebugContext->pStack, realexprstr.str().c_str());

        lua_pushinteger(stack, retCode);

        if (retCode == 0)
        {
            lua_getglobal(pDebugContext->pStack, temp_result_var.c_str());

            LuaUtils::TransferValueToStack(pDebugContext->pStack, stack);

            // pop the global variable name off the stack now that we are
            // done copying it.
            lua_pop(pDebugContext->pStack, 1);
        }
        else
        {
            lua_pushstring(stack, lua_tostring(pDebugContext->pStack, -1));
        }
    }

    return 2;
}



//*****************************************************************************
/*!
 *  \brief  Get local variables in a stack frame.
 *
 *  \luaparam   context -   The context to be resumed.
 *  \luaparam   frame   -   The frame in which the locals are to be extracted.
 *
 *  \version
 *      - S Panyam  24/11/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaBindings::GetLocals(LuaStack stack)
{
    DebugContext *  pDebugContext   = GetContextIfPaused(stack);
    if (pDebugContext != NULL)
    {
        int             frame           = lua_tointeger(stack, 2);
        lua_Debug debug;
        LuaDebug pFrameDebug = pDebugContext->GetDebug();
        if (frame > 0)
        {
            pFrameDebug = &debug;
            lua_getstack(pDebugContext->pStack, frame, pFrameDebug);
        }

        lua_pushinteger(stack, 0);
        lua_newtable(stack);

        // and get all the locals now!!!
        for (int i = 1, nitems = 0;;i++)
        {
            const char *varname = lua_getlocal(pDebugContext->pStack, pFrameDebug, i);

            if (varname == NULL)
                break ;

            // pop the variable value off - we do not return this (or
            // should we?)
            lua_pop(pDebugContext->pStack, 1);

            // ignore temporaries?
            // if (strncmp("(*", varname, 2) != 0)
            {
                // push the new variable index
                lua_pushinteger(stack, ++nitems);

                // push a table to hold the variable info
                lua_newtable(stack);

                // push the index and the variable name
                lua_pushinteger(stack, i);
                lua_setfield(stack, -2, "index");

                lua_pushstring(stack, varname);
                lua_setfield(stack, -2, "name");

                lua_settable(stack, -3);
            }
        }
    }

    return 2;
}


//*****************************************************************************
/*!
 *  \brief  Get value of a local variable in a stack frame.
 *
 *  \luaparam   context -   The context to be resumed.
 *  \luaparam   lv      -   Index of the local variable.
 *  \luaparam   nlevels -   Number of leves to recurse into the value 
 *                          (default = 1)
 *  \luaparam   frame   -   The frame in which the locals are to be extracted.
 *                          (default = 0)
 *
 *  \version
 *      - S Panyam  24/11/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaBindings::GetLocal(LuaStack stack)
{
    DebugContext *  pDebugContext   = GetContextIfPaused(stack);
    if (pDebugContext != NULL)
    {
        int             lvindex         = lua_tointeger(stack, 2);
        int             nlevels         = lua_tointeger(stack, 3);
        int             frame           = lua_tointeger(stack, 4);

        lua_Debug debug;
        LuaDebug pFrameDebug = pDebugContext->GetDebug();
        if (frame > 0)
        {
            pFrameDebug = &debug;
            lua_getstack(pDebugContext->pStack, frame, pFrameDebug);
        }

        const char *varname = lua_getlocal(pDebugContext->pStack, pFrameDebug, lvindex);
        if (varname == NULL)
        {
            lua_pushinteger(stack, -1);
            lua_pushstring(stack, "Invalid variable index.");
        }
        else
        {
            // output code - 0 => success
            lua_pushinteger(stack, 0);

            // local variable value
            LuaUtils::TransferValueToStack(pDebugContext->pStack, stack, -1, nlevels, varname);

            // pop the value of the local variable 
            // of the stack being debugged!!
            lua_pop(pDebugContext->pStack, 1);
        }
    }

    return 2;
}

//*****************************************************************************
/*!
 *  \brief  Set value of a local variable in a stack frame.
 *
 *  \luaparam   context -   The context to be resumed.
 *  \luaparam   frame   -   The frame in which the locals are to be extracted.
 *  \luaparam   lv      -   Index of the local variable.
 *  \luaparam   type    -   Type of the new value.
 *  \luaparam   value   -   The new value
 *
 *  \version
 *      - S Panyam  24/11/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaBindings::SetLocal(LuaStack stack)
{
    DebugContext *  pDebugContext   = GetContextIfPaused(stack);
    if (pDebugContext != NULL)
    {
        // int             frame           = lua_tointeger(stack, 2);
        // int             lv              = lua_tointeger(stack, 3);
        // std::string     varType(lua_tostring(stack, 4));

        lua_pushinteger(stack, 0);
        lua_pushstring(stack, "Not yet implemented.");
    }

    return 2;
}


//*****************************************************************************
/*!
 *  \brief  Get all the up values in a stack frame.
 *
 *  \luaparam   context -   The context to be resumed.
 *  \luaparam   frame   -   The frame in which the locals are to be extracted.
 *
 *  \version
 *      - S Panyam  08/12/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaBindings::GetUpValues(LuaStack stack)
{
    DebugContext *  pDebugContext   = GetContextIfPaused(stack);
    if (pDebugContext != NULL)
    {
        int             frame           = lua_tointeger(stack, 2);
        lua_Debug debug;
        LuaDebug pFrameDebug = pDebugContext->GetDebug();
        if (frame > 0)
        {
            pFrameDebug = &debug;
            lua_getstack(pDebugContext->pStack, frame, pFrameDebug);
        }

        // push message result
        lua_pushinteger(stack, 0);

        // add a table to hold each closure
        lua_newtable(stack);

        // and get all the upvalues now!!!
        for (int funcindex = 1; ;funcindex++)
        {
            int uv = 1;
            for (; ;uv++)
            {
                const char *uvname = lua_getupvalue(pDebugContext->pStack, funcindex, uv);

                if (uvname == NULL)
                    break ;

                // pop the uv name off - we do not return this (or
                // should we?)
                lua_pop(pDebugContext->pStack, 1);

                if (uv == 1)
                {
                    // add a table to hold UVs for each closure
                    lua_pushinteger(stack, funcindex);
                    lua_newtable(stack);
                }

                // if (strncmp("(*", uvname, 2) != 0 && 
                //     strncmp("--", uvname, 2) != 0)
                {
                    // push the new variable index
                    lua_pushinteger(stack, uv);

                    // push a table to hold the variable info
                    lua_newtable(stack);

                    // push the index and the UV name
                    lua_pushinteger(stack, uv);
                    lua_setfield(stack, -2, "index");

                    lua_pushstring(stack, uvname);
                    lua_setfield(stack, -2, "name");

                    lua_settable(stack, -3);
                }
            }
            if (uv == 1) break ;
            else lua_settable(stack, -3);
        }
    }

    return 2;
}


//*****************************************************************************
/*!
 *  \brief  Get value of an upvalue in a stack frame.
 *
 *  \luaparam   context -   The context to be resumed.
 *  \luaparam   lv      -   Index of the upvalue.
 *  \luaparam   nlevels -   Number of leves to recurse into the value 
 *                          (default = 1)
 *  \luaparam   frame   -   The frame in which the upvalues are to be extracted.
 *                          (default = 0)
 *
 *  \version
 *      - S Panyam  08/12/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaBindings::GetUpValue(LuaStack stack)
{
    DebugContext *  pDebugContext   = GetContextIfPaused(stack);
    if (pDebugContext != NULL)
    {
        int             funcindex       = lua_tointeger(stack, 2);
        int             uvindex         = lua_tointeger(stack, 3);
        int             nlevels         = lua_tointeger(stack, 4);
        int             frame           = lua_tointeger(stack, 5);

        lua_Debug debug;
        LuaDebug pFrameDebug = pDebugContext->GetDebug();
        if (frame > 0)
        {
            pFrameDebug = &debug;
            lua_getstack(pDebugContext->pStack, frame, pFrameDebug);
        }

        const char *varname = lua_getupvalue(pDebugContext->pStack, funcindex, uvindex);
        if (varname == NULL)
        {
            lua_pushinteger(stack, -1);
            lua_pushstring(stack, "Invalid variable index.");
        }
        else
        {
            // output code - 0 => success
            lua_pushinteger(stack, 0);

            // upvalue value
            LuaUtils::TransferValueToStack(pDebugContext->pStack, stack, -1, nlevels, varname);

            // pop the value of the upvalue
            // of the stack being debugged!!
            lua_pop(pDebugContext->pStack, 1);
        }
    }

    return 2;
}

//*****************************************************************************
/*!
 *  \brief  Set value of an upvalue in a stack frame.
 *
 *  \luaparam   context -   The context to be resumed.
 *  \luaparam   frame   -   The frame in which the upvalues are to be extracted.
 *  \luaparam   uv      -   Index of the upvalue.
 *  \luaparam   type    -   Type of the new value.
 *  \luaparam   value   -   The new value
 *
 *  \version
 *      - S Panyam  08/12/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaBindings::SetUpValue(LuaStack stack)
{
    DebugContext *  pDebugContext   = GetContextIfPaused(stack);
    if (pDebugContext)
    {
        // int             frame           = lua_tointeger(stack, 2);
        // int             uv              = lua_tointeger(stack, 3);
        // std::string     varType(lua_tostring(stack, 4));

        lua_pushinteger(stack, 0);
        lua_pushstring(stack, "Not yet implemented.");
    }

    return 2;
}

LUNARPROBE_NS_END

