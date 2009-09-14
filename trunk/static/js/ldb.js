
var clientId        = null;
var numEvents       = -1;
var channelStarted  = false;
var msgCounter      = 0;

/**
 * Handshaking is the first part of connecting to a bayeux channel.
 * With a handshake, the server will return a clientId, to identify a
 * client connection to bayeux and the server will use this clientId to
 * send subsequent messages to the client
 *
 * Once we have a clientId, we can subscribe to any channel we want (in
 * this case we only subscribe to the ldb channel for listening to debugger
 * events).
 */
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

            SubscribeToChannel("/ldb");
            /*
            SubscribeToChannel("/channel1");
            SubscribeToChannel("/channel2");
            SubscribeToChannel("/channel3");
            SubscribeToChannel("/channel4");
            SubscribeToChannel("/channel5");
            */
        }
    }

    Log("Perform handshakes...");
    var data = {'channel': '/meta/handshake',
                'version': '1.0',
                'supportedConnectionTypes': ['long-polling',
                                             'callback-polling',
                                             'iframe']};
    var datastr = JSON.encode(data);
    MakeAjaxRequest("POST", "/bayeux/", callback, datastr, false);
}

/**
 * Generic function to subscribe to a particular named channel.
 */
function SubscribeToChannel(channel)
{
    Log("Subscribing to channel: " + channel);
    var data = {'channel': '/meta/subscribe',
                'clientId': clientId,
                'subscription': channel};

    var datastr = JSON.encode(data);
    MakeAjaxRequest("POST", "/bayeux/", ChannelEventHandler, datastr, true);
}

/**
 * Handles events sent by the server on a channel that we are subscribed to
 * (using the SubscribeToChannel method above).
 */
function ChannelEventHandler(request)
{
    var result  = JSON.decode(request.responseText);
    numEvents   = numEvents + 1;

    if (numEvents == 0)
    {
        // this is the first event so ignore it if need be
        channelStarted = true;

        // ListDir(".")
        OnConnected();
    }
    HandleEvent(result);
}

/**
 * Tells if we are connected.
 */
function IsConnected()
{
    return clientId != null;
}

