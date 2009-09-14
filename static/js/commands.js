
function EvalExpression(context, expr, token)
{
    function callback(result)
    {
        GetClient().GotEvalResult(result);
    }

    SendCommand("eval", callback, {'context': context, 'expr_str': expr, 'token': token});
}

function ResetDebugger()
{
    SendCommand("reset", null, null);
}

function GetContexts()
{
    function callback(result) { GetClient().SetContexts(result); }

    SendCommand("contexts", callback, null);
}

function GetBreakpoints()
{
    function callback(result) { GetClient().SetBreakpoints(result); }

    SendCommand("breaks", callback, null);
}

function SetBPAtFile(filename, linenum)
{
    var data = {'filename': filename, 'linenum': linenum};

    function callback(result) { GetClient().SetBPResult(data, result) };

    SendCommand("break", callback, data);
}

function SetBPAtFunction(funcname)
{
    var data = {'funcname': funcname};

    function callback(result) { GetClient().SetBPResult(data, result) };

    SendCommand("break", callback, data);
}

function ClearBPAtFile(filename, linenum)
{
    var data = {'filename': filename, 'linenum': linenum};

    function callback(result) { GetClient().ClearBPResult(data, result) };

    SendCommand("clear", callback, data);
}

function ClearBPAtFunction(funcname)
{
    var data = {'funcname': funcname};

    function callback(result) { GetClient().ClearBPResult(data, result) };

    SendCommand("clear", callback, data);
}

function ClearAllBreakpoints()
{
    function callback(result) { GetClient().SetBreakpoints([]); }

    SendCommand("clearall", callback, null);
}

function Step(context)
{
    SendCommand("step", null, {'context': context});
}

function Next(context)
{
    SendCommand("next", null, {'context': context});
}

function Finish(context)
{
    SendCommand("finish", null, {'context': context});
}

function Continue(context)
{
    SendCommand("continue", null, {'context': context});
}

function GetLocal(context, frame, lvindex, numlevels)
{
    function callback(result)
    {
        GetClient().GotLocalVariable(result);
    }

    SendCommand("local", callback, {'context': context,
                                    'frame': frame,
                                    'lv': lvindex,
                                    'nlevels': nlevels});
}

function GetAllLocals(context, frame)
{
    function callback(result)
    {
        GetClient().GotAllLocalVariables(result);
    }

    SendCommand("locals", callback, {'context': context, 'frame': frame });
}


function GetUpValue(context, frame, uvindex, nlevels, funcindex)
{
    function callback(result)
    {
        GetClient().GotUpValue(result);
    }

    SendCommand("upval", callback, {'context': context, 'frame': frame,
                                    'uv': uvindex, 'nlevels': nlevels,
                                    'func': funcindex});
}


function GetAllUpValues(context, frame)
{
    function callback(result)
    {
        GetClient().GotAllUpValues(result);
    }

    SendCommand("upvals", callback, {'context': context, 'frame': frame});
}

function GetFile(file, first, last)
{
    function callback(result)
    {
        GetClient().GotFile(result);
    }

    SendCommand("file", callback, {'file': file, 'first': first, 'last': last});
}


function ListDir(dir)
{
    function callback(result)
    {
        GetClient().GotDirListing(result);
    }

    SendCommand("files", callback, {'dir': dir});
}
