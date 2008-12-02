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
 * \file   LunarProbe.h
 *
 * \brief  The hook between liblua and the debug interface.
 *
 * Version:
 *      Sri Panyam      05 Dec 2008
 *      Initial version
 *
 *****************************************************************************/

#ifndef _LUNAR_PROBE_H
#define _LUNAR_PROBE_H

#include "lpfwddefs.h"
#include <string>

LUNARPROBE_CPP_NAMESPACE_BEGIN

//*****************************************************************************
/*!
 *  \class  LunarProbe
 *
 *  \brief  Main functions to start and stop debugging of lua stacks.
 *
 *****************************************************************************/
class LunarProbe
{
public:
    // Starts the debugging of a lua stack
    static int Start(LuaStack lua_stack, const char *name = "");

    // Stops debugging of a lua stack
    static int Stop(LuaStack lua_stack);

    // gets the singleton debugger instance.
    static Debugger *GetInstance();

private:
    //! Singleton instance of the debugger.
    static std::auto_ptr< Debugger >    pDebugger;
};

LUNARPROBE_CPP_NAMESPACE_END

#endif

