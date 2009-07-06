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

LUNARPROBE_NS_BEGIN

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
LuaStack LuaUtils::NewLuaStack(bool openlibs, bool debug, const char *name)
{
    LuaStack new_stack = lua_open();

    if (openlibs)
        luaL_openlibs(new_stack);

    // Force lua garbage collector to be non incremental
    lua_gc(new_stack, LUA_GCSETSTEPMUL, 100000000);

    if (debug)
        LunarProbe::GetInstance()->Attach(new_stack, name);

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
 *  \brief  Wrapper to push a json node smart ptr.
 *
 *  \param  L       Stack on which the json node is pushed.
 *  \param  node    The node to be pushed.
 *
 *  \version
 *      - S Panyam  26/06/2009
 *      Initial version.
 */
//*****************************************************************************
void LuaUtils::PushJson(lua_State  *L, const JsonNodePtr &node)
{
    PushJson(L, node.Data());
}

//*****************************************************************************
/*!
 *  \brief  Pushes a json object onto the lua stack.
 *
 *  \param  L       Stack on which the json node is pushed.
 *  \param  node    The node to be pushed.
 *
 *  \version
 *      - S Panyam  26/06/2009
 *      Initial version.
 */
//*****************************************************************************
void LuaUtils::PushJson(lua_State  *L, const JsonNode *node)
{
    if (!node)
    {
        lua_pushnil(L);
        return ;
    }

    switch (node->Type())
    {
        case JNT_NULL: lua_pushnil(L); break;
        case JNT_BOOL: lua_pushboolean(L, node->Value<bool>()); break;
        case JNT_INT: lua_pushinteger(L, node->Value<int>()); break;
        case JNT_DOUBLE: lua_pushnumber(L, node->Value<double>()); break;
        case JNT_STRING: lua_pushstring(L, node->Value<std::string>().c_str()); break;
        case JNT_LIST:
            lua_newtable(L);
            for (unsigned i = 0, size = node->Size();i < size;++i)
            {
                lua_pushinteger(L, i + 1);
                PushJson(L, node->Get(i));
                lua_settable(L, -3);
            } 
            break;
        case JNT_OBJECT: 
            const JsonObjectNode *objNode = static_cast<const JsonObjectNode *>(node);

            lua_newtable(L);
            for (JsonObjectNode::const_iterator iter = objNode->begin(); iter != objNode->end(); ++iter)
            {
                PushJson(L, iter->second);
                lua_setfield(L, -2, iter->first.c_str());
            }
            break;
    }
}

//*****************************************************************************
/*!
 *  \brief  Converts the top most item on the stack into a json node.
 *
 *  \param  L       Stack from which the json node is pulled.
 *  \param  input   The node to be saved as.
 *
 *  \version
 *      - S Panyam  26/06/2009
 *      Initial version.
 */
//*****************************************************************************
void LuaUtils::PopJson(lua_State  *L, JsonNodePtr &output)
{
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
            case 'j':   // table
                PushJson(L, va_arg(vl, const JsonNode *));
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
                    
                    *va_arg(vl, std::string *) = std::string(lua_tostring(L, nres));
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

//*****************************************************************************
/*!
 *  \brief  Transfers a value from a lua stack to another.
 *
 *  \param  inStack     Stack where the value resides.
 *  \param  outStack    Stack where the value is to be moved to.
 *  \param  ntop        Index on the first stack where the value exists
 *                      (default = -1 => top).
 *  \param  levels      How many levels to recurse to if the value is a
 *                      table (default = 1)..
 *  \param  varname     Name to assign to the value on the second stack, if
 *                      the value is being copied as a key in table.
 *
 *  \version
 *      - S Panyam  04/11/2008
 *      Initial version.
 */
//*****************************************************************************
bool LuaUtils::TransferValueToStack(LuaStack    inStack,
                                    LuaStack    outStack,
                                    int         ntop,
                                    int         levels,
                                    const char *varname)
{
    if (levels < 0)
        return false;

    // convert relative to absolute indexing if necessary
    if (ntop < 0)
        ntop = (lua_gettop(inStack) + 1 + ntop);

    lua_newtable(outStack);

    // push variable name
    if (varname != NULL)
    {
        lua_pushstring(outStack, varname);
        lua_setfield(outStack, -2, "name");
    }

    // push the variable type
    int top_type = lua_type(inStack, ntop);
    lua_pushstring(outStack, lua_typename(inStack, top_type));
    lua_setfield(outStack, -2, "type");

    // finally push the value
    if (lua_isnil(inStack, ntop))
    {
        lua_pushnil(outStack);
        lua_setfield(outStack, -2, "value");
    }
    else if (lua_isboolean(inStack, ntop))
    {
        lua_pushboolean(outStack, lua_toboolean(inStack, ntop));
        lua_setfield(outStack, -2, "value");
    }
    else if (lua_isnumber(inStack, ntop))
    {
        lua_pushnumber(outStack, lua_tonumber(inStack, ntop));
        lua_setfield(outStack, -2, "value");
    }
    else if (lua_isstring(inStack, ntop))
    {
        lua_pushstring(outStack, lua_tostring(inStack, ntop));
        lua_setfield(outStack, -2, "value");
    }
    else if (lua_islightuserdata(inStack, ntop))
    {
        lua_pushlightuserdata(outStack, lua_touserdata(inStack, ntop));
        lua_setfield(outStack, -2, "value");
    }
    else if (lua_isuserdata(inStack, ntop)  ||
             lua_iscfunction(inStack, ntop) ||
             lua_isfunction(inStack, ntop)  ||
             lua_isthread(inStack, ntop))
    {
        lua_pushfstring(outStack, "%p", lua_topointer(inStack, ntop));
        lua_setfield(outStack, -2, "value");
    }
    else if (lua_istable(inStack, ntop))
    {
        if (levels == 0)
        {
            lua_pushboolean(outStack, true);
            lua_setfield(outStack, -2, "raw");

            // whether to send only a summary!
            lua_pushfstring(outStack, "%p", lua_topointer(inStack, ntop));
        }
        else
        {
            lua_newtable(outStack);

            int index = 1;

            lua_pushnil(inStack);
            while (lua_next(inStack, ntop) != 0)
            {
                // printf("%s - %s\n", lua_typename(inStack, lua_type(inStack, -2)), lua_typename(inStack, lua_type(inStack, -1)));

                int newtop      = lua_gettop(inStack);
                int keyindex    = newtop - 1;
                int valindex    = newtop;

                lua_pushinteger(outStack, index++);
                lua_newtable(outStack);

                // uses 'key' at index top-1 and 'value' at index top
                if (LuaUtils::TransferValueToStack(inStack, outStack, keyindex, 0))
                {
                    lua_setfield(outStack, -2, "key");

                    if (LuaUtils::TransferValueToStack(inStack, outStack, valindex, levels - 1))
                        lua_setfield(outStack, -2, "value");
                }

                lua_settable(outStack, -3);

                // remove 'value', keeps 'key' for next iteration
                lua_pop(inStack, 1);
            }
        }

        lua_setfield(outStack, -2, "value");
    }
    else
    {
        lua_pushstring(outStack, "Unknown type.");
        lua_setfield(outStack, -2, "value");
    }

    return true;
}

LUNARPROBE_NS_END

