
function GetClient()
{
    return ElementById("LDBApp");
}

/**
 * Called by a channel handler when we have connected (with successful
 * handshaking).
 */
function OnConnected()
{
    alert("Damn We cannot Override OnConnected");
    var ldbClient   = GetClient();
    ldbClient.OnConnected();
}

/**
 * Called by a channel handler when we recieve an even that needs to be
 * handled.
 */
function HandleEvent(result)
{
    alert("Damn We cannot Override HandleEvent");
    var ldbClient   = GetClient();
    ldbClient.HandleEvent(result);
}

function SendCommand(cmd, cmd_data, call_back_id)
{
    var command = {'id': call_back_id, 'cmd': cmd, 'data': cmd_data};
    var data    = {'channel': '/ldb',
                   'clientId': clientId,
                   'command': (command)}

    function handler(request)
    {
        // alert("SendCommand Result: " + request.responseText);
        if (request.readyState == 4 && call_back_id != null)
        {
            var result = JSON.decode(request.responseText);
            GetClient().CommandCallback(call_back_id, result);
            // call_back(result);
        }
    }

    var datastr = JSON.encode(data);
    MakeAjaxRequest("POST", "/bayeux/", handler, datastr, false);
}

