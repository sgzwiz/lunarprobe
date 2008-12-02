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
 *  \file   LuaUtils.cpp
 *
 *  \brief  Lua utility definitions.
 *
 *  \version
 *      - S Panyam  06/11/2008
 *        Initial version
 *
 *****************************************************************************/

#include "lpmain.h"

LUNARPROBE_CPP_NAMESPACE_BEGIN

int traceback(lua_State *L)
{
    lua_getfield(L, LUA_GLOBALSINDEX, "debug");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return 1;
    }

    lua_getfield(L, -1, "traceback");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 2);
        return 1;
    }

    lua_pushvalue(L, 1);  /* pass error message */
    lua_pushinteger(L, 2);  /* skip this function and traceback */
    lua_call(L, 2, 1);  /* call debug.traceback */
    return 1;
}

//*****************************************************************************
/*!
 *  \brief  Writes a lua error to std error.
 *
 *  \version
 *      - S Panyam  04/11/2008
 *      Initial version.
 */
//*****************************************************************************
void LuaError(lua_State *stack, const char *fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    fprintf(stderr, "\n==============================\n");
    fprintf(stderr, "Error in Stack (%p): %s\n", stack, lua_tostring(stack, -1));
    vfprintf(stderr, fmt, vl);
    fprintf(stderr, "\n==============================\n");
    va_end(vl);
}

//*****************************************************************************
/*!
 *  \brief  convinience function creating a new lua stack and opening the
 *          standard libraries
 *
 *  \version
 *      - S Panyam  04/11/2008
 *      Initial version.
 */
//*****************************************************************************
LuaStack LuaUtils::NewLuaStack(bool openlibs, bool debug)
{
    LuaStack new_stack = lua_open();

    if (openlibs)
        luaL_openlibs(new_stack);

    // Force lua garbage collector to be non incremental
    lua_gc(new_stack, LUA_GCSETSTEPMUL, 100000000);

    if (debug)
        LunarProbe::Attach(new_stack);

    return new_stack;
}


//*****************************************************************************
/*!
 *  \brief  Runs a lua script (from file).
 *
 *  \param  L       The stack on which the script is to be executed.
 *  \param  file    The file to be executed.
 *
 *  \version
 *      - S Panyam  05/12/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaUtils::RunLuaScript(lua_State *L, const char *file)
{
    int retCode = luaL_dofile(L, file);
    if (retCode != 0)
    {
        LuaError(L, "Cannot run '%s'.", file);
    }
    return retCode;
}

//*****************************************************************************
/*!
 *  \brief  Runs a raw lua string on a specific stack.
 *
 *  \param  L       The stack on which the string is to be executed.
 *  \param  lua_str The raw lua string to execute.
 *  \param  len     Length of the raw string. if -ve, it is calculated with
 *                  a strlen.
 *
 *  \version
 *      - S Panyam  01/12/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaUtils::RunLuaString(lua_State *L, const char *lua_str, int len)
{
    int retCode;
    int errorFunc   = 0;

    if (true)
    {
        lua_pushcfunction(L, traceback);
        errorFunc   = lua_gettop(L);
    }

    if (len < 0)
        len = strlen(lua_str);

    if ((retCode = luaL_loadbuffer(L, lua_str, len, NULL)) != 0)
    {
        LuaError(L, "Error Loading String '%s'", lua_str);
        return retCode;
    }
    else if ((retCode = lua_pcall(L, 0, LUA_MULTRET, errorFunc)) != 0)
    {
        LuaError(L, "Error Executing String: '%s'\n", lua_str);
    }

    // Remove the error function
    if (errorFunc != 0)
        lua_remove(L, errorFunc);

    return retCode;
}

//*****************************************************************************
/*!
 *  \brief  Same as CallLuaFunc but takes a va_list instead of the variable
 *  arg list.
 *
 *  \param  funcname    Name of the lua function to call.
 *  \param  funcsig     Signature of the funtion.
 *  \param  vl          va_list representing the variable args.
 *
 *  \version
 *      - S Panyam  04/11/2008
 *      Initial version.
 */
//*****************************************************************************
int LuaUtils::VCallLuaFunc(lua_State  *L, const char *funcname, const char *funcsig, va_list vl)
{
    int narg, nres;     /* # of args and results */
    int top_before  = lua_gettop(L);
    int errorFunc   = 0;

    if (true)
    {
        lua_pushcfunction(L, traceback);
        errorFunc   = lua_gettop(L);
    }

    lua_getglobal(L, funcname);

    // push args onto the stack
    narg = 0;

    while (*funcsig)
    {
        switch (*funcsig++)
        {
            case 'u':   // light user data
                lua_pushlightuserdata(L, va_arg(vl, void*));
                break;
            case 'b':   // bool arg
                lua_pushboolean(L, va_arg(vl, int) != 0);
                break;
            case 'd':   // double arg
                lua_pushnumber(L, va_arg(vl, double));
                break;
            case 'i':   // int arg
                lua_pushinteger(L, va_arg(vl, int));
                break;
            case 's':   // long arg
                lua_pushstring(L, va_arg(vl, char *));
                break;
            case '>':
                goto endwhile;
            default:
                fprintf(stderr, "Stack %p - Invalid option (%c)", L, *(funcsig - 1));
        }
        narg++;
        luaL_checkstack(L, 1, "Too many arguments");
    }
endwhile:

    // do the call
    nres = strlen(funcsig);
    int result = 0;

    if ((result = lua_pcall(L, narg, nres, errorFunc)) != 0) // do the call
    {
        int top_after = lua_gettop(L);

        LuaError(L, "Error running function '%s'", funcname);

        printf("==============>  TopBefore: %d, TopAfter: %d, ErrorFunc: %d, nArgs: %d\n", top_before, top_after, errorFunc, narg);

        // lua_pop(L, 1);
    }
    else
    {
        // retrieve the results
        nres = -nres;

        while (*funcsig)    // get results
        {
            switch (*funcsig++)
            {
                case 'u':   // light user data
                    if (!lua_islightuserdata(L, nres))
                        fprintf(stderr, "Stack %p - Wrong result type.  Expected light user data.", L);
                    *va_arg(vl, void **) = lua_touserdata(L, nres);
                    break;

                case 'b':   // bool arg
                    if (lua_isboolean(L, nres))
                        *va_arg(vl, int *) = lua_toboolean(L, nres);
                    else if (lua_isnumber(L, nres))
                        *va_arg(vl, int *) = static_cast<int>(lua_tonumber(L, nres));
                    else
                        fprintf(stderr, "Stack %p - Wrong result type.  Expected boolean.", L);
                    break;

                case 'd': // double result 
                    if (lua_isnumber(L, nres))
                        *va_arg(vl, double *) = lua_tonumber(L, nres);
                    else
                        fprintf(stderr, "Stack %p - Wrong result type.  Expected double.", L);
                    break ;

                case 'i': // int result 
                    if (lua_isnumber(L, nres))
                        *va_arg(vl, int *) = static_cast<int>(lua_tonumber(L, nres));
                    else
                        fprintf(stderr, "Stack %p - Wrong result type.  Expected int.", L);
                    break ;

                case 's': // string result 
                    if (!lua_isstring(L, nres))
                        fprintf(stderr, "Stack %p - Wrong result type.  Expected string.", L);
                    *va_arg(vl, const char **) = lua_tostring(L, nres);
                    break ;

                default:
                   fprintf(stderr, "Stack %p - Invalid option (%c)", L, *(funcsig - 1));
            }
            nres ++;
        }
    }

    // Remove the error function
    if (errorFunc != 0)
        lua_remove(L, errorFunc);

    // force GC on errors
    // if (result != 0) lua_gc(L, LUA_GCCOLLECT, 0);

    return result;
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
int LuaUtils::CallLuaFunc(lua_State *L, const char *funcname, const char *funcsig, ...)
{
    va_list vl;
    va_start(vl, funcsig);
    int result = VCallLuaFunc(L, funcname, funcsig, vl);
    va_end(vl);
    return result;
}

LUNARPROBE_CPP_NAMESPACE_END

