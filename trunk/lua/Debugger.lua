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

    \file       Debugger.lua

    \brief      Classes for maintaining and handling debugger state and
                breakpoints.

    \version
            S Panyam 07/Nov/08
            - Initial version

--------------------------------------------------------------------------------]]

require "Handlers"

--[[------------------------------------------------------------------------------
    \brief  Extracts the address portion from teh string version of a
    userdata (which is usually of the form "userdata: 0x........)

    \version
            S Panyam 12/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function userdataToString(udObj)
    local addr_str = tostring(udObj)

    addr_str = string.gsub(addr_str, "userdata: ", "")

    return addr_str
end


--[[------------------------------------------------------------------------------
    \brief  Converts a filename and linunum into a string to be used as a
    unique index.

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function bpToString(funcname, linenum)
    if linenum == nil then
        linenum = -1
    end
    return (funcname .. ":" .. linenum)
end

--[[------------------------------------------------------------------------------
    \brief  A class to maintain info about a breakpoint.

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
Breakpoint = {
    filename    = nil, 
    funcname    = nil,
    linenum     = -1,
    enabled     = true,
}

--[[------------------------------------------------------------------------------
    \brief  Holds info about the lua stack being debugged. 

    \version
            S Panyam 10/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
DebugContext = {
    -- The user data representing the cpp context object
    cppContext  = nil,

    -- The stringified version of the address of the cppContext
    address     = nil,

    -- the debugger in which this context belongs
    debugger    = nil,

    -- Whether the context is running or not
    running     = false,

    -- The location where the context is paused at
    location    = nil,

    -- Name of the context
    name        = "",

    -- The last "flow" related command called - used to handle when
    -- a debug hook function actually stops evenif there is no breakpoint
    -- on a line.
    lastCommand = nil,

    -- The stack trace of what got us here.
    stacktrace  = {},
}

--[[------------------------------------------------------------------------------
    \brief  A class to maintain the debugger state.

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
Debugger = {
    -- The cpp debugger object that we will be serving
    cppDebugger     = nil,

    -- The contexts that will be passed down to us.
    contexts        = {},

    -- All breakpoints by name
    bpsByName       = {},

    -- All breakpoints by index
    bpsByIndex      = {},

    -- Functions for handling the commands recieved by the client - 
    -- stored in Handlers.lua
    commandHandlers = {},
}

--[[------------------------------------------------------------------------------
    \brief  Constructor for a breakpoint.

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Breakpoint:New()
    local o = {}
    setmetatable(o, self)
    self.__index = self
    return o
end

--[[------------------------------------------------------------------------------
    \brief  Constructor for a DebugContext.

    \version
            S Panyam 10/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function DebugContext:New(context, name, debugger)
    local o = {}
    setmetatable(o, self)
    self.__index = self
    o.cppContext    = context
    o.debugger      = debugger
    o.address       = userdataToString(context)
    o.running       = false
    o.location      = nil
    o.lastCommand   = nil
    o.name          = name
    return o
end

--[[------------------------------------------------------------------------------
    \brief  Sets the last command.  Used to determine whether a breakpoint
    should halt execution or not.

    \param  cmd_type    -   Command type/name.
    \param  cmd_data    -   Command data.

    \version
            S Panyam 11/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function DebugContext:SetLastCommand(cmd_type, cmd_data)
    self.lastCommand = {["type"] = cmd_type, ["data"] = cmd_data}
end

--[[------------------------------------------------------------------------------
    \brief  Pauses the debug context and sets the last command to nil.

    \version
            S Panyam 11/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function DebugContext:Pause()
    self.running = false
end

--[[------------------------------------------------------------------------------
    \brief  Resumes the debug context.

    \version
            S Panyam 11/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function DebugContext:Resume()
    self.running = true

    DebugLib.Resume(self.cppContext)

    -- notify clients that the context has resumed
    -- self.debugger:SendEvent("ContextResumed", {["address"] = self.address, ["name"] = self.name})
end

--[[------------------------------------------------------------------------------
    \brief  Evaluate an expression and return the result.

    \version
            S Panyam 01/Dec/08
            - Initial version
--------------------------------------------------------------------------------]]
function DebugContext:EvaluateString(expr_str)
    return DebugLib.EvaluateString(self.cppContext, expr_str)
end

--[[------------------------------------------------------------------------------
    \brief  Get the local variables in a frame (default 0).

    \version
            S Panyam 25/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function DebugContext:GetLocals(frame)
    return DebugLib.GetLocals(self.cppContext, frame)
end

--[[------------------------------------------------------------------------------
    \brief  Get the value of a local variable in a frame (default 0).

    \version
            S Panyam 25/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function DebugContext:GetLocal(lvindex, nlevels, frame)
    return DebugLib.GetLocal(self.cppContext, lvindex, nlevels, frame)
end

--[[------------------------------------------------------------------------------
    \brief  Get the upvalues in a frame (default 0).

    \version
            S Panyam 08/Dec/08
            - Initial version
--------------------------------------------------------------------------------]]
function DebugContext:GetUpValues(frame)
    return DebugLib.GetUpValues(self.cppContext, frame)
end

--[[------------------------------------------------------------------------------
    \brief  Get the value of an upvalue in a frame (default 0).

    \version
            S Panyam 25/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function DebugContext:GetUpValue(funcindex, uvindex, nlevels, frame)
    return DebugLib.GetUpValue(self.cppContext, funcindex, uvindex, nlevels, frame)
end

--[[------------------------------------------------------------------------------
    \brief  Clears the last command.

    \version
            S Panyam 11/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function DebugContext:ClearLastCommand()
    self.lastCommand = nil
end

--[[------------------------------------------------------------------------------
    \brief  Called to create a new debugger state.

    \param  cpppDebugger    -   The cpp debugger object.

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:New(cppDebugger)
    local o = {}
    setmetatable(o, self)
    self.__index = self

    o.cppDebugger                   = cppDebugger
    
    -- set command handlers
    o.commandHandlers["reset"]      = MsgFunc_Reset
    o.commandHandlers["load"]       = MsgFunc_Load

    -- BP related messages
    o.commandHandlers["break"]      = MsgFunc_SetBP
    o.commandHandlers["breaks"]     = MsgFunc_GetBPs
    o.commandHandlers["clear"]      = MsgFunc_ClearBP
    o.commandHandlers["clearall"]   = MsgFunc_ClearBPs

    -- flow related messages
    o.commandHandlers["step"]       = MsgFunc_Step
    o.commandHandlers["next"]       = MsgFunc_Next
    o.commandHandlers["until"]      = MsgFunc_Until
    o.commandHandlers["finish"]     = MsgFunc_Finish
    o.commandHandlers["continue"]   = MsgFunc_Continue

    -- information related messages
    o.commandHandlers["print"]      = MsgFunc_Print
    o.commandHandlers["list"]       = MsgFunc_List
    o.commandHandlers["set"]        = MsgFunc_Set
    o.commandHandlers["eval"]       = MsgFunc_Eval
    o.commandHandlers["local"]      = MsgFunc_Local
    o.commandHandlers["locals"]     = MsgFunc_Locals
    o.commandHandlers["upval"]      = MsgFunc_UpValue
    o.commandHandlers["upvals"]     = MsgFunc_UpValues
    o.commandHandlers["frame"]      = MsgFunc_Frame
    o.commandHandlers["contexts"]   = MsgFunc_Contexts
    o.commandHandlers["file"]       = MsgFunc_File
    o.commandHandlers["files"]      = MsgFunc_Files

    return o
end

--[[------------------------------------------------------------------------------
    \brief  Removes a debug context if it exists.

    \return The debug context being removed.

    \version
            S Panyam 12/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:RemoveDebugContext(context)
    local addr_str      = userdataToString(context)
    local dbgContext    = self.contexts[addr_str]

    if dbgContext ~= nil then
        table.remove(self.contexts, addr_str)
    end

    return dbgContext
end

--[[------------------------------------------------------------------------------
    \brief  Gets the debug context matching the address.

    \return The debug context matching the given address.

    \version
            S Panyam 10/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:GetDebugContext(context, name)
    local addr_str      = userdataToString(context)
    local dbgContext    = self.contexts[addr_str]

    if dbgContext == nil then
        dbgContext = DebugContext:New(context, name, self)
        self.contexts[addr_str] = dbgContext
    end

    return dbgContext
end

--[[------------------------------------------------------------------------------
    \brief  Remove a context from our list.

    \return The removed context.

    \version
            S Panyam 10/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:RemoveDebugContext(context)
    local addr_str      = userdataToString(context)
    local dbgContext    = self.contexts[addr_str]

    if dbgContext ~= nil then
        table.remove(self.contexts, addr_str)
    end

    return dbgContext
end

--[[------------------------------------------------------------------------------
    \brief  Calls the actual cppDebugger object to send a message to the
    client.

    \param  msg -   Message to send

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:SendMessage(msg)
    print("Sending Message: " .. msg)
    DebugLib.WriteString(self.cppDebugger, msg)
end

--[[------------------------------------------------------------------------------
    \brief  Wrapper for sending a server initiated event.

    \param  evt_name    Name of the event to send.
    \param  evt_data    Data of the event to send.

    \version
            S Panyam 20/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:SendEvent(evt_name, evt_data)
    self:SendMessage(Json.Encode({["type"]      = "Event",
                                  ["event"]     = evt_name,
                                  ["data"]      = evt_data}))
end

--[[------------------------------------------------------------------------------
    \brief  Wrapper for sending a response/reply to a client initiated
    event/message.

    \param  code                (Return) Code of the event handled.
    \param  data                (Return) Value of the event handled.
    \param  original_message    Original message sent by the client.

    \version
            S Panyam 21/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:SendReply(code, value, original_message)
    self:SendMessage(Json.Encode({["type"]      = "Reply",
                                  ["code"]        = code,
                                  ["value"]       = value,
                                  ["original"]    = original_message}))
end

--[[------------------------------------------------------------------------------
    \brief  Reloads a script on a particular context.

    \param  msg -   Message to send

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:LoadFile(context, filename)
    DebugLib.LoadFile(self, context, filename)
end

--[[------------------------------------------------------------------------------
    \brief  Requests a reload of the debugger.

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:Reload()
    DebugLib.Reload(self.cppDebugger)
end

--[[------------------------------------------------------------------------------
    \brief  Gets a BP at a given function

    \param  funcname    -   Name of the function where the BP is to be searched for.

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:GetBPByFunction(funcname)
    return self.bpsByName[funcname]
end

--[[------------------------------------------------------------------------------
    \brief  Gets a BP at a given file and line.

    \param  filename    -   Name of the file where the BP is to be searched for.
    \param  linenum     -   Line on which the BP is located.

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:GetBPByFile(filename, linenum)
    return self.bpsByName[bpToString(filename, linenum)]
end

--[[------------------------------------------------------------------------------
    \brief  Gets a BP by its index.

    \param  index   -   Index of the BP.

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:GetBP(index)
    return self.bpsByIndex[index]
end

--[[------------------------------------------------------------------------------
    \brief  Return a list of all BPs

    \return A list of all breakpoints.

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:GetBPs()
    return self.bpsByIndex
end

--[[------------------------------------------------------------------------------
    \brief  Sets a BP at a given function

    \param  funcname    -   Name of the function where the BP is to be set.

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:SetBPAtFunction(funcname)
    local bp = self:GetBPByFunction(funcname)
    
    if bp == nil then
        bp = Breakpoint:New()
        bp.funcname = funcname
        self.bpsByName[funcname] = bp
        table.insert(self.bpsByIndex, bp)
    end
        
    return bp
end

--[[------------------------------------------------------------------------------
    \brief  Sets a BP at a given file and line.

    \param  filename    -   Name of the file where the BP is to be set.
    \param  linenum     -   Line on which the BP is to be set.

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:SetBPAtFile(filename, linenum)
    local bp = self:GetBPByFile(filename, linenum)
    
    if bp == nil then
        bp = Breakpoint:New()
        bp.filename = filename
        bp.linenum  = linenum
        if bp.linenum == nil then
            bp.linenum = -1
        end
        local bpString = bpToString(filename, linenum)

        self.bpsByName[bpString] = bp
        table.insert(self.bpsByIndex, bp)
    end
        
    return bp
end

--[[------------------------------------------------------------------------------
    \brief  Enable a BP at a given index.

    \param  index   -   Index of the bp to enable/disable
    \param  enable  -   Enable or disable the BP.

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:EnableBP(index, enable)
    self.bpsByIndex[index].enabled = enable
end

--[[------------------------------------------------------------------------------
    \brief  Remove a BP at a given index.

    \param  index   -   Index of the bp to remove.

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:RemoveBP(index)
    local bp = table.remove(self.bpsByIndex, index)

    if bp.filename ~= nil then
        table.remove(self.bpsByName, bpToString(bp.filename, bp.linenum))
    else
        table.remove(self.bpsByName, bp.funcname)
    end
end

--[[------------------------------------------------------------------------------
    \brief  Remove all BPs.

    \version
            S Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function Debugger:ClearBPs()
    self.bpsByName = {}
    self.bpsByIndex = {}
end

