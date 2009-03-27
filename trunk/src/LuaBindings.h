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
 *  \file   LuaBindings.h
 *
 *  \brief  Lua bindings for the Debugger.
 *
 *  \version
 *      - S Panyam   28/10/2008
 *      Initial version.
 */
//*****************************************************************************

#ifndef _LUABINDINGS_H_
#define _LUABINDINGS_H_

#include "LuaUtils.h"
#include "thread/mutex.h"

LUNARPROBE_NS_BEGIN

//*****************************************************************************
/*!
 *  \class  LuaBindings
 *
 *  \brief  Lua bindings for the debugger.
 *
 *****************************************************************************/
class LuaBindings
{
public:
    // The folder where all the lua files are - modified by 
    // the embedding program
    static std::string  LUA_SRC_LOCATION;

    // The prefix to be used on all global variables that the debugger
    // inserts into a lua stack that is being debugged (will be removed as
    // soon as debugging is stopped).
    static std::string  LUA_VAR_PREFIX;

public:
    // ctor
    LuaBindings(Debugger *pDebugger);

    // dtor
    virtual ~LuaBindings();

    // Called by the debugger to tell LUA that a context has been added
    virtual void ContextAdded(DebugContext *pContext);

    // Called by the debugger to tell LUA that a context has been removed
    virtual void ContextRemoved(DebugContext *pContext);

    // Called by the debugger to tell LUA to handle a break point.
    virtual void HandleBreakpoint(DebugContext *pContext, LuaDebug pDebug);

    // Called by the debugger to notify LUA to handle a client message
    virtual void HandleMessage(const std::string &message);

    // Requests a reload of the scripts.
    void RequestReload();

public: // Lua methods - called from LUA
    static int  Register(LuaStack stack);

    // Reload the debugger lua script
    static int  Reload(LuaStack stack);

    // Sends a string to the client
    static int  WriteString(LuaStack stack);

    // Evaluate a string and return the result.
    static int  EvaluateString(LuaStack stack);

    // Get local variables in a frame
    static int  GetLocals(LuaStack stack);

    // Get value of a local variable in a given frame.
    static int  GetLocal(LuaStack stack);

    // Set value of a local variable in a given frame.
    static int  SetLocal(LuaStack stack);

    // Get upvalues in a frame
    static int  GetUpValues(LuaStack stack);

    // Get value of an upvalue in a given frame.
    static int  GetUpValue(LuaStack stack);

    // Set value of an upvalue in a given frame.
    static int  SetUpValue(LuaStack stack);

    // Resumes a particular debug context
    static int  Resume(LuaStack stack);

    // Loads a file on a context
    static int  LoadFile(LuaStack stack);

    // Get the list of contexts being debugged
    static int GetContexts(LuaStack stack);

protected:
    // Gets the lua stack instance
    LuaStack    GetLuaStack();

    // Calls the function on the debugger lua files.
    int         CallLuaFunc(const char *funcname, const char *funcsig, ...);

protected:
    //! The actual debugger object we will be seving.
    Debugger *  pDebugger;

    //! The debugger lua stack - for running the debugger
    LuaStack    pStack;

    //! Whether a reload has been requested or not.
    bool        reloadRequested;

    // Mutex for the debugger lua stack
    SMutex      dbgStackMutex;
};

LUNARPROBE_NS_END

#endif

