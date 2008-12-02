//***************************************************************************
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
 *  \file   LuaUtils.h
 *
 *  \brief  Lua Utilities.
 *
 *  \version
 *      - S Panyam  06/11/2008
 *        Initial version
 *
 *****************************************************************************/

#ifndef _LP_LUA_UTILS_H_
#define _LP_LUA_UTILS_H_

#include <iostream>
#include <stdarg.h>
#include "lpfwddefs.h"

LUNARPROBE_CPP_NAMESPACE_BEGIN

//*****************************************************************************
/*!
 *  \class  LuaUtils
 *
 *  \brief  A lua utilities class.
 *
 *****************************************************************************/
class LuaUtils
{
public:
    // Create a new lua state
    static LuaStack NewLuaStack();

    // Print errors for lua functions
    // static void LuaError(LuaStack stack, const char *fmt, ...);

    // Runs a lua script
    static int RunLuaScript(LuaStack stack, const char *file);

    // Runs a raw lua string.
    static int RunLuaString(LuaStack stack, const char *lua_str, int len = -1);

    // calls an arbitrary lua function
    static int CallLuaFunc(LuaStack stack, const char *funcname, const char *funcsig, ...);
    static int VCallLuaFunc(LuaStack stack, const char *funcname, const char *funcsig, va_list ap);
};

LUNARPROBE_CPP_NAMESPACE_END

#endif

