
var clientId        = null;
var numEvents       = -1;
var channelStarted  = false;
var msgCounter      = 0;

function ChannelEventHandler(request)
{
    var result      = JSON.decode(request.responseText);
    numEvents = numEvents + 1;

    if (numEvents == 0)
    {
        // this is the first event so ignore it if need be
        channelStarted = true;
    }
    else
    {
        alert("Received Event: " + request.responseText);
    }
}

function SubscribeToChannel()
{
    var data = {'channel': '/meta/subscribe',
                'clientId': clientId,
                'subscription': '/ldb'};

    var datastr = JSON.encode(data);
    MakeAjaxRequest("POST", "/bayeux/", ChannelEventHandler, datastr, true);
}

function DoHandshake()
{
    // subscribe to all channels
    function callback(request)
    {
        if (request.readyState == 4)
        {
            // woo whoo success
            var result = JSON.decode(request.responseText);
            clientId = result['clientId'];

            SubscribeToChannel();
        }
    }

    var data = {'channel': '/meta/handshake',
                'version': '1.0',
                'supportedConnectionTypes': ['long-polling',
                                             'callback-polling',
                                             'iframe']};
    var datastr = JSON.encode(data);
    MakeAjaxRequest("POST", "/bayeux/", callback, datastr);
}

function LuaDBLoaded()
{
    // now do the channel subscriptions
    DoHandshake();
}

function SendCommand(cmd, call_back, cmd_data)
{
    msgCounter  = msgCounter + 1;
    var command = {'id': msgCounter, 'cmd': cmd, 'data': cmd_data};
    var data    = {'channel': '/ldb',
                   'clientId': clientId,
                   'command': JSON.encode(command)}
    function handler(request)
    {
        // alert("SendCommand Result: " + request.responseText);
        if (call_back != null)
        {
            var result = JSON.decode(request.responseText);
            call_back(result);
        }
    }

    var datastr = JSON.encode(data);
    MakeAjaxRequest("POST", "/bayeux/", handler, datastr, true);
}

function GetClient()
{
    return ElementById("LDBApp");
}

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


