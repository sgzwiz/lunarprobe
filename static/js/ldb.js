
var clientId        = null;
var numEvents       = -1;
var channelStarted  = false;

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

