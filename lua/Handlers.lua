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

    \file       Handlers.lua

    \brief      FUnctions for handling messages sent by the client.  All
    handlers either reutrn a code/value pair or nothing.  A return of
    nothing or code == 0, implies the command was successful and nothing
    needs to be returned.  Anything else is a failure.  This is handed over
    the client.

    \version
            Sri Panyam 07/Nov/08
            - Initial version

--------------------------------------------------------------------------------]]

require "Json"

function getContextFromMessageData(debugger, msg_data, not_running)
    local address   = msg_data["context"]
    if address == nil then
        return -1, "'context' parameter missing"
    end

    local debugContext =  debugger:GetDebugContext(address)
    if debugContext == nil then
        return -1, "Invalid context address specified."
    end

    if not_running == nil then
        not_running = true
    end

    if debugContext.running and not_running then
        return -1, "Command cannot be handled.  Context is still running."
    end

    if debugContext.location == nil then
        return -1, "ASSERT Failed: debutContext.location is null"
    end

    return 0, debugContext
end

--[[------------------------------------------------------------------------------
    \brief  Resets the debugger causing a reload of the scripts.

    \param  debugger    -   The debugger context to be modified.
    
    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_Reset(debugger, msg_data)
    debugger:Reload()
end

--[[------------------------------------------------------------------------------
    \brief  Loads a file and runs the script again.

    \param  debugger    -   The debugger context to be modified.
    \param  msg_data    -   {"file"     - Name of the file to load,
                             "context"  - Context to load it in
                            }
    
    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_Load(debugger, msg_data)
    local code, debugContext = getContextFromMessageData(debugger, msg_data)
    if code ~= 0 then
        return code, debugContext
    end

    local filename = msg_data["filename"]

    if filename == nil then
        return -1, "'filename' parameter missing."
    end

    debugger:LoadFile(debugContext.cppContext, filename)
end


--[[------------------------------------------------------------------------------
    \brief  Called to set a break point.

    \param  debugger    -   The debugger context to be modified.
    \param  msg_data    -   "filename"  if breakpoint is in a file
                            "linenum"   if brekapoint is in a file
                            "funcname"  if breakpoint is on a function
    
    \return (0, index) if breakpoint was set (or exists),
            otherwise (-1, error message) on error

    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_SetBP(debugger, msg_data)
    if type(msg_data) ~= "table" then
        return -1, "Message data must be a table!"
    end

    if msg_data["funcname"] ~= nil then
        local funcname  = msg_data["funcname"]

        bp = debugger:SetBPAtFunction(funcname)
    elseif msg_data["filename"] ~= nil then
        local filename  = msg_data["filename"]
        local linenum   = msg_data["linenum"]
        bp = debugger:SetBPAtFile(filename, linenum)
    else
        return -1, "Atleast one of funcname or filename/linenum must be specified"
    end

    -- TODO: this is wrong the index can be anywhere!
    return 0, #debugger.bpsByIndex
end

--[[------------------------------------------------------------------------------
    \brief  Called to get a list of all BPs.

    \param  debugger    -   The debugger context to be modified.
    \param  msg_data    -   None
    
    \return (0, bp_list)

    \version
            Sri Panyam 05/Dec/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_GetBPs(debugger, msg_data)
    return 0, debugger.bpsByIndex
end


--[[------------------------------------------------------------------------------
    \brief  Called to clear a particular BP.

    \param  debugger    -   The debugger context to be modified.
    \param  msg_data    -   The index of the bp to remove.
    
    \return (0) if breakpoint was removed.
            otherwise (-1, error message) on error

    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_ClearBP(debugger, msg_data)
    if type(msg_data) ~= "number" then
        return -1, "Message data must be an BP index!"
    end

    local index = msg_data

    if index > #debugger.bpsByIndex then
        return -1, "Invalid breakpoint index!"
    end

    debugger:RemoveBP(index)
end

--[[------------------------------------------------------------------------------
    \brief  Removes all break points.

    \param  debugger    -   The debugger context to be modified.
    
    \return (0) if breakpoint was removed.
            otherwise (-1, error message) on error

    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_ClearBPs(debugger, msg_data)
    debugger:ClearBPs()
end

--[[------------------------------------------------------------------------------
    \brief  Sets the debugger context to step into the next line (entering a
    function if necessary).  If a step is not possible, this is similar to
    a next.  Also if the next function is not a lua function, it wont be
    "break"ed at.

    \param  debugger    -   The debugger context to be modified.
    \param  msg_data    -   Context being controlled
    
    \return (0) if successful, otherwise (-1, error message) on error

    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_Step(debugger, msg_data)
    local code, debugContext = getContextFromMessageData(debugger, msg_data)
    if code ~= 0 then
        return code, debugContext
    end

    debugContext:SetLastCommand("step")
    debugContext:Resume(debugContext)
end

--[[------------------------------------------------------------------------------
    \brief  Sets the debugger context to step over the next line (entering a
    function if necessary).  Also if the next line is not a lua function or
    part of one, it wont be "break"ed at.

    \param  debugger    -   The debugger context to be modified.
    \param  msg_data    -   Context being controlled.
    
    \return (0) if successful, otherwise (-1, error message) on error

    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_Next(debugger, msg_data)
    local code, debugContext = getContextFromMessageData(debugger, msg_data)
    if code ~= 0 then
        return code, debugContext
    end

    debugContext:SetLastCommand("next",
                                {["line"] = debugContext.location.currentline,
                                 ["lastline"] = debugContext.location.lastlinedefined,
                                 ["function"] = debugContext.location.name,
                                 ["file"] = debugContext.location.source
                                })
    debugContext:Resume(debugContext)
end

--[[------------------------------------------------------------------------------
    \brief  Sets the debugger context to continue until the specified line or
    function has been hit, or the next breakpoint is hit (at which point
    this "until" flag is cleared).

    \param  debugger    -   The debugger context to be modified.
    \param  msg_data    -   Context being controlled followed by a 
                            line number or a function to run until.
    
    \return (0) if successful, otherwise (-1, error message) on error

    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_Until(debugger, msg_data)
    local code, debugContext = getContextFromMessageData(debugger, msg_data)
    if code ~= 0 then
        return code, debugContext
    end

    if msg_data["function"] == nil and msg_data["line"] == nil then
        return -1, [[Until command requires 'line' (with optional 'file' parameter) or 'function' parameter]]
    end

    if msg_data["file"] == nil then
        msg_data["file"] = debugContext.location.source
    end

    debugContext:SetLastCommand("until",
                                {["line"] = msg_data["line"],
                                 ["file"] = msg_data["file"],
                                 ["function"] = msg_data["function"]})
end

--[[------------------------------------------------------------------------------
    \brief  Sets the debugger context to continue till the current function
    has returned or the next breakpoint is hit (at which point this "until"
    flag is cleared).

    \param  debugger    -   The debugger context to be modified.
    \param  msg_data    -   Context being controlled
    
    \return (0) if successful, otherwise (-1, error message) on error

    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_Finish(debugger, msg_data)
    local code, debugContext = getContextFromMessageData(debugger, msg_data)
    if code ~= 0 then
        return code, debugContext
    end

    debugContext:SetLastCommand("finish", {["function"] = debugContext.location.name})
    debugContext:Resume(debugContext)
end

--[[------------------------------------------------------------------------------
    \brief  Continues to the next breakpoint.

    \param  debugger    -   The debugger context to be modified.
    \param  msg_data    -   Context being controlled
    
    \return (0) if successful, otherwise (-1, error message) on error

    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_Continue(debugger, msg_data)
    local code, debugContext = getContextFromMessageData(debugger, msg_data)
    if code ~= 0 then
        return code, debugContext
    end

    debugContext:ClearLastCommand()
    debugContext:Resume(debugContext)
end

--[[------------------------------------------------------------------------------
    \brief  Evaluates a command and prints the result.

    \param  debugger    -   The debugger context to be modified.
    \param  statement   -   The statement to execute.
    
    \return (0, result) if successful, otherwise (-1, error message) on error

    \version
            Sri Panyam 10/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_Print(debugger, msg_data)
end

--[[------------------------------------------------------------------------------
    \brief  List the code in a given file and an optional range.
    frame.

    \param  debugger    -   The debugger context to be modified.
    \param  msg_data    -   {"filename" - Name of the file to print
                             "start"    - Starting line (defaults to 0)
                             "end"      - Ending line (defaults to last line)
                            }
    
    \return (0, list of lines) if successful, otherwise (-1, error message) on error

    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_List(debugger, msg_data)
end

--[[------------------------------------------------------------------------------
    \brief  Set the value of a variable in a given stack frame.

    \param  debugger    -   The debugger context to be modified.
    \param  msg_data    -   {"frame     - defaults to 0,
                             "variable" - Name or index of the variable,
                             "value"    - Value - how to code this?
                            }
    
    \return (0, ) if successful, otherwise (-1, error message) on error

    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_Set(debugger, msg_data)
end

--[[------------------------------------------------------------------------------
    \brief  Evaluates an expression and returns its value.

    \param  debugger    -   The debugger context to be modified.
    \param  msg_data    -   {"context": The context on which we want to
                                        evaluate the expression.
                             "expr_str": The expression string.}
    
    \return (0, (var/value) pairs) if successful, otherwise (-1, error message) on error

    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_Eval(debugger, msg_data)
    local code, debugContext = getContextFromMessageData(debugger, msg_data)
    if code ~= 0 then
        return code, debugContext
    end

    local expr_str = msg_data["expr_str"]
    if expr_str == nil or expr_str == "" then
        return -1, "Invalid expression string."
    end

    return debugContext:EvaluateString(expr_str)
end

--[[------------------------------------------------------------------------------
    \brief  Return the value of a single local variable in a given stack
    frame.

    \param  debugger    -   The debugger context to be modified.
    \param  msg_data    -   The stack frame number.  If missing then
                            defaults to 0.
    
    \return (0, (var/value) pairs) if successful, otherwise (-1, error message) on error

    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_Local(debugger, msg_data)
    local code, debugContext = getContextFromMessageData(debugger, msg_data)
    if code ~= 0 then
        return code, debugContext
    end

    local frame     = msg_data["frame"]
    if frame == nil or frame < 0 then
        frame = 0
    end

    local lvindex   = msg_data["lv"]
    if lvindex == nil or lvindex < 0 then
        lvindex = 1
    end

    local nlevels = msg_data["nlevels"]
    if nlevels == nil or nlevels < 1 then
        nlevels = 1
    end

    return debugContext:GetLocal(lvindex, nlevels, frame)
end

--[[------------------------------------------------------------------------------
    \brief  Return the local variables and their values in a given stack
    frame.

    \param  debugger    -   The debugger context to be modified.
    \param  msg_data    -   {"context" - The context whose locals we are intersted in.
                             "frame" - The stack frame number.  If missing
                             then defaults to 0. }
    
    \return (0, (var/value) pairs) if successful, otherwise (-1, error message) on error

    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_Locals(debugger, msg_data)
    local code, debugContext = getContextFromMessageData(debugger, msg_data)
    if code ~= 0 then
        return code, debugContext
    end

    local frame     = msg_data["frame"]
    if frame == nil or frame < 0 then
        frame = 0
    end
     
    return debugContext:GetLocals(frame)
end

--[[------------------------------------------------------------------------------
    \brief  Return the value of a single upvalue in a given stack
    frame.

    \param  debugger    -   The debugger context to be modified.
    \param  msg_data    -   {"frame"    The stack frame,
                             "func"     Closure Function index,
                             "uv"       Index of the upvalue,
                             "nlevels"  Number of levels to recurse to.}
    
    \return (0, (var/value) pairs) if successful, otherwise (-1, error message) on error

    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_UpValue(debugger, msg_data)
    local code, debugContext = getContextFromMessageData(debugger, msg_data)
    if code ~= 0 then
        return code, debugContext
    end

    local frame     = msg_data["frame"]
    if frame == nil or frame < 0 then
        frame = 0
    end

    local funcindex = msg_data["func"]
    if funcindex == nil or funcindex < 0 then
        funcindex = 1
    end

    local uvindex   = msg_data["uv"]
    if uvindex == nil or uvindex < 0 then
        uvindex = 1
    end

    local nlevels = msg_data["nlevels"]
    if nlevels == nil or nlevels < 1 then
        nlevels = 1
    end

    return debugContext:GetUpValue(funcindex, uvindex, nlevels, frame)
end

--[[------------------------------------------------------------------------------
    \brief  Return the upvalues in a given stack frame.

    \param  debugger    -   The debugger context to be modified.
    \param  msg_data    -   {"context" - The context whose locals we are intersted in.
                             "frame" - The stack frame number.  If missing
                             then defaults to 0. }
    
    \return (0, (var/value) pairs) if successful, otherwise (-1, error message) on error

    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_UpValues(debugger, msg_data)
    local code, debugContext = getContextFromMessageData(debugger, msg_data)
    if code ~= 0 then
        return code, debugContext
    end

    local frame     = msg_data["frame"]
    if frame == nil or frame < 0 then
        frame = 0
    end
     
    return debugContext:GetUpValues(frame)
end

--[[------------------------------------------------------------------------------
    \brief  Return information about a given stack frame and set the given
    frame as the current frame.

    \param  debugger    -   The debugger context to be modified.
    \param  msg_data    -   The stack frame number.  If missing then
                            defaults to 0.
    
    \return (0) if successful, otherwise (-1, error message) on error

    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_Frame(debugger, msg_data)
end

--[[------------------------------------------------------------------------------
    \brief  Returns a list of all contexts being debugged.

    \param  debugger    -   The debugger context to be modified.
    
    \return (0, list of context/stack pairs) if successful,
            otherwise (-1, error message) on error

    \version
            Sri Panyam 07/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_Contexts(debugger, msg_data)
    -- local contexts = debugger.contexts
    local contexts = DebugLib.GetContexts(debugger.cppDebugger)
    local out = {}

    -- update existing context table
    for i, v in ipairs(contexts) do
        local context   = debugger:GetDebugContext(v["context"])
        context.name    = v["name"]
        context.running = v["running"]
    end

    -- send out the contexts
    for k, v in pairs(debugger.contexts) do
        table.insert(out, {["address"]  = v["address"],
                           ["name"]     = v["name"],
                           ["running"]  = v["running"],
                           ["location"] = v["location"]})
    end

    return 0, out
end

--[[------------------------------------------------------------------------------
    \brief  Returns a list of files in a directory.

    The file contents are returned within a given range.
    \param  debugger    -   The debugger context.
    \param  msg_data    -   {'dir' - List of packages to restrict files to.}
    
    \return (0, list of files) if successful,
            otherwise (-1, error message) on error

    \version
            Sri Panyam 03/Apr/09
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_Files(debugger, msg_data)
    local directory     = msg_data["dir"]

    if directory == nil then
        directory = "."
    end

    return DebugLib.ListDir(directory)
    -- local listing = DebugLib.ListDir(directory)
    -- return listing["code"], listing["value"]
end

--[[------------------------------------------------------------------------------
    \brief  Returns the contents of a file.

    The file contents are returned within a given range.
    \param  debugger    -   The debugger context.
    \param  msg_data    -   {'file'     -   Name of the file to inspect,
                             'raw'      -   Return raw file (default = false)
                             'first'    -   Starting line (default 0)
                             'last'     -   ending line   (default -1 => last)
                            }
    
    \return (0, list of lines) if successful,
            otherwise (-1, error message) on error

    \version
            Sri Panyam 12/Nov/08
            - Initial version
--------------------------------------------------------------------------------]]
function MsgFunc_File(debugger, msg_data)
    local filename      = msg_data["file"]
    local raw           = msg_data["raw"]
    local first_line    = msg_data["first"]
    local last_line     = msg_data["last"]

    if filename == nil then
        return -1, "Filename MUST be specified"
    end

    if raw == nil then
        raw = false
    end

    if first_line == nil or type(first_line) ~= "number" or first_line <= 0 then
        first_line = 1
    end

    if last_line == nil or type(last_line) ~= "number" or last_line <= 0 then
        last_line = math.huge
    end

    local lines = {}
    local fhandle = io.open(filename, "r")

    if fhandle == nil then
        return -1, "Could not open file."
    end

    for l = 1, first_line - 1 do
        local line = fhandle:read("*line")
        if line == nil then
            fhandle:close()
            return -1, "Invalid range."
        end
    end

    if first_line > last_line then
        local temp  = first_line
        first_line  = last_line
        last_line   = temp
    end

    for l = first_line, last_line do
        local line = fhandle:read("*line")
        if line == nil then
            break
        end
        table.insert(lines, line)
    end

    fhandle:close()

    return 0, lines
end

