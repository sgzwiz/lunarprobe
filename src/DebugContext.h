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
 *  \file   DebugContext.h
 *
 *  \brief  Holds all things to do with the debug info for a particular lua
 *  stack.
 *
 *  \version
 *        - S Panyam  23/10/2008
 *        Initial version.
 *
 *****************************************************************************/

#ifndef _DEBUGCONTEXT_H_
#define _DEBUGCONTEXT_H_

#include <string>
#include "halley.h"

LUNARPROBE_NS_BEGIN

//*****************************************************************************
/*!
 *  \class  DebugContext
 *
 *  \brief  The debug context/data for a particular lua context.
 *
 *****************************************************************************/
class DebugContext
{
public:
    // ctor
    DebugContext(LuaStack luaStack, const char *name = "");

    // Blocks while the debug context is being paused
    int         WaitWhilePaused();

    // Pauses further processing of a particular lua stack
    bool        Pause(LuaDebug pDebug);

    // Loads a file on a context
    bool        LoadFile(const char *filename);

    // Resumes lua stack processing
    bool        Resume();

    // Gets the lua debug object.
    LuaDebug    GetDebug() { return pDebug; }


public:
    //! Is the debugger for this stack currently running?
    bool        running;

    //! The lua stack this context is dealing with
    LuaStack    pStack;

    //! Name of the stack
    std::string name;

protected:
    SMutex          runStateMutex;
    SCondition      pausedCond;

private:
    //! More info about the current breakpoint where we are paused.
    LuaDebug  pDebug;
};

LUNARPROBE_NS_END

#endif

