--[[------------------------------------------------------------------------------
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

            http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

----------------------------------------------------------------------------------

    \file       DebuggerMain.lua

    \brief      Main entry point from the CPP debugger and handles all
                debugger related actions.

    \version
            S Panyam 07/Nov/08
            - Initial version

--------------------------------------------------------------------------------]]

-- 
-- Set the package path to where all these files are going to be.
-- 
--[[------------------------------------------------------------------------------
if string.find(package.path, "libgameengine") == nil then
    package.path = package.path ..  ";lua/?.lua;shared/libgameengine/lua/debugger/?.lua"
end
--------------------------------------------------------------------------------]]

require "Json"
require "Debugger"

LUA_HOOKCALL	=   0
LUA_HOOKRET	    =   1
LUA_HOOKLINE	=   2
LUA_HOOKCOUNT	=   3
LUA_HOOKTAILRET =   4

--[[------------------------------------------------------------------------------
    \brief  Get the global debugger instance and creates one if it does not
    exist.
    
    \param  pDebugger   -   The cpp debugger object with which to creat the
    debugger if one does not exist.

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function GetDebugger(pDebugger)
    if THE_DEBUGGER == nil then
        THE_DEBUGGER = Debugger:New(pDebugger)
    end
    return THE_DEBUGGER
end

--[[------------------------------------------------------------------------------
    \brief  Called when a new context is added.

    \param  pDebugger   -   The debug server that invoked this script.
    \param  pContext    -   The debug context of a lua context being added.

    \version
            S Panyam 12/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function ContextAdded(pDebugger, pContext, name)
    local debugger          = GetDebugger(pDebugger)
    local debugContext      = debugger:GetDebugContext(pContext, name)
    debugContext.running    = true
    -- print("======================================================================")
    -- print("Adding Context: " .. name .. " - " .. tostring(pDebgugger) .. ", " ..  tostring(pContext))

    debugger:SendEvent("ContextAdded",
                        {["address"]  = debugContext["address"],
                         ["name"]     = debugContext["name"],
                         ["running"]  = debugContext["running"],
                         ["location"] = debugContext["location"]})
end

--[[------------------------------------------------------------------------------
    \brief  Called when a new context is removed.

    \param  pDebugger   -   The debug server that invoked this script.
    \param  pContext    -   The debug context of a lua context being removed.

    \version
            S Panyam 12/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function ContextRemoved(pDebugger, pContext)
    local debugger      = GetDebugger(pDebugger)
    local debugContext  = debugger:RemoveDebugContext(pContext)

    if debugContext ~= nil then
        -- print("Removed Context: " .. tostring(pDebguger) .. ", " ..  tostring(pContext))

        debugger:SendEvent("ContextRemoved",
                            {["address"]  = debugContext["address"],
                             ["name"]     = debugContext["name"],
                             ["running"]  = debugContext["running"],
                             ["location"] = debugContext["location"]})
    end
end

--[[------------------------------------------------------------------------------
    \brief  Called when the debug function is hit.

    \param  pDebugger    -   The debug server that invoked this script.
    \param  pDebug          -   The debug context of a lua context being debugged.

    \version
            S Panyam 04/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function HandleBreakpoint(pDebugger, pContextAddr, contextName, debug_event,
                          debug_name, debug_namewhat, debug_what,
                          debug_source, debug_currentline, debug_nups,
                          debug_linedefined, debug_lastlinedefined)
    -- initialise debugger if not already done
    local debugger      = GetDebugger(pDebugger)
    local debugContext  = debugger:GetDebugContext(pContextAddr, contextName)

    --[[ So waht do we do here?
        1. Essentially we need to check whether we should wait here or not.
        This is determined by whether there are breakpoints on this line.
        There are 2 kinds of breakpoint: Implicit and Explicit.

        Explicit ones are ones set by the user.
        Implicit ones are the ones resulted from user commands affecting
        the flow - eg step, next, continue, until, return and so on.

        So we have to check for implicit and explicit breakpoints.
    --]]

    local lastCommand   = debugContext.lastCommand
    local handled       = true
    local bp            = nil

    if debug_event == LUA_HOOKCALL then
        -- A function was entered so only stop here if:
        -- A BP exists in the function, or last command was step (but not next)

        bp = debugger:GetBPByFunction(debug_name)

        if bp ~= nil or (lastCommand ~= nil and lastCommand["type"] == "step") then
            handled = false
        end

    elseif debug_event == LUA_HOOKRET and lastCommand ~= nil then

        -- a return from a function only triggers a 
        -- break if the last command was a "finish" (in the same function)
        local cmd_type  = lastCommand["type"]
        local cmd_data  = lastCommand["data"]

        if cmd_type == "finish" and cmd_data["function"] == debug_name then
            handled = false
        end
    elseif debug_event == LUA_HOOKLINE then

        -- is there an explicit breakpoint?
        bp = debugger:GetBPByFile(debug_source, debug_currentline)

        -- print("Debugger, Context, BP: " .. tostring(debugger) .. ", " ..  tostring(debugContext) .. ", " .. tostring(bp))
        if bp ~= nil then

            handled = false

        elseif lastCommand ~= nil then
            -- no look for the implicit breakpoitns - these are tricky!!
            -- line changes are the trickiest because they have to take into
            -- account "next", "step" and "until" commands which may infact
            -- have been set in "other" functions

            local cmd_type  = lastCommand["type"]
            local cmd_data  = lastCommand["data"]

            if cmd_type == "step" or 
               (cmd_type == "until" and cmd_data["line"] == debug_currentline and cmd_data["file"] == debug_source) or
               (cmd_type == "next" and cmd_data["function"] == debug_name) then
                handled = false
            end
        end
    end

    if not handled then
        --[[--
        print("===============================================================")
        print("=======     BreakPoint: " .. tostring(bp))
        if bp ~= nil then
            print("=======         FileName: " .. tostring(bp.filename))
            print("=======         Linenum: " .. tostring(bp.linenum))
            print("=======         Function: " .. tostring(bp.funcname))
            print("=======         Enabled: " .. tostring(bp.enabled))
        end
        print("===============================================================")
        --]]--
        debugContext:Pause()
        debugContext:ClearLastCommand()
        debugContext.location  = {
            ["event"]           = debug_event,
            ["name"]            = debug_name,
            ["namewhat"]        = debug_namewhat,
            ["what"]            = debug_what,
            ["source"]          = debug_source,
            ["currentline"]     = debug_currentline,
            ["nups"]            = debug_nups,
            ["linedefined"]     = debug_linedefined,
            ["lastlinedefined"] = debug_lastlinedefined,
        }

        -- tell the client we have stopped!
        debugger:SendEvent("ContextPaused", debugContext);
    else
        debugContext.location = nil
    end

    -- nope no breakpoints found, 
    return handled
end

--[[------------------------------------------------------------------------------
    \brief  Called when the debug client sends the server a message 
            (ie run, break, step etc).

    \param  pMessage    -   The string representing a message.  It is
                            completely our (this script's) responsibility
                            to decode the message.

    \version
            S Panyam 04/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function HandleMessage(pDebugger, pMessage)
    local debugger      = GetDebugger(pDebugger)
    -- local message       = Json.Decode(pMessage)
    local message       = pMessage
    local msg_id        = message["id"]
    local msg_cmd       = message["cmd"]
    local msg_data      = message["data"]
    local code, value   = -1, "Unknown Error"

    if debugger.commandHandlers[msg_cmd] == nil then
        print("Invalid message: " .. msg_cmd)
        value = "Invalid command"
    else
        if msg_data == nil then
            msg_data = ""
        end
        code, value = debugger.commandHandlers[msg_cmd](debugger, msg_data)
    end

    -- simply send back the message along with the type, 
    -- code and value of the response
    -- debugger:SendReply(code, value, message)
    return Json.Encode({["type"]        = "Reply",
                        ["code"]        = code,
                        ["value"]       = value,
                        ["original"]    = pMessage})
    ---[[
    --]]
end

