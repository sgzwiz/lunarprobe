// ActionScript file

import dialogs.*;
import editor.*;

import flash.display.*;
import flash.external.*;
import flash.media.*;
import flash.net.*;
import flash.utils.*;

import mx.collections.*;
import mx.containers.TitleWindow;
import mx.controls.ProgressBar;
import mx.events.*;

import utils.*;

// Dialog for showing which units can be built on a building.
// TODO: This will have to go into a separate SWF
private var progressBarDialog:TitleWindow     	= null;
private var theProgressBar:ProgressBar        	= null;
private var gameHost: String				 	= "localhost";
private var gamePort: int						= 9999;
private var connecting:Boolean 					= true;
private var cancelConnect:Boolean 				= false;
private var connected:Boolean					= false;
private var currClientId:String					= null;
private var progressMonitor:*					= null;
private var dataListeners:Array					= new Array();
private var jsonParser:JsonParser				= null;
private var commandCallbacks:*					= {};
private var numFuncCalls:Number					= 0;
private var connectDialog:OkCancelDialog		= null;

// Currently opened file
private var currDoc: IDocument				    = null;

private function onCreationComplete(event: Object): void
{
	filesView.theApp = this;
	contextsView.theApp = this;
	
    // flash.system.Security.allowDomain("localhost:8080");
    // will be called by JS when it has recieved the result of a command we invoked
    ExternalInterface.addCallback("CommandCallback", CommandCallback);
    ExternalInterface.addCallback("OnConnected", OnConnected);
    ExternalInterface.addCallback("Log", Log);
    ExternalInterface.addCallback("SetConnected", SetConnected);
    ExternalInterface.addCallback("HandleEvent", HandleEvent);
    
    // tell JS we are loaded
    ExternalInterface.call("LuaDBLoaded");
}

// Callback for all commands - called by JS when a command is 
// finished or it has recieved a result
private function CommandCallback(callback_id: *, result: *): void
{
	var callback_func:Function = commandCallbacks[callback_id];
	delete commandCallbacks[callback_id];
	if (callback_func != null)
		callback_func(result);
}

// Called by JS when an event is recieved
private function HandleEvent(event: *): void
{
	Log(JsonParser.JsonEncode(event));
}

// Sends a command via JS to the debugger.
public function SendCommand(cmd: String, cmd_data: *, callback: Function): void
{
	var currFuncIndex:Number = numFuncCalls++;
	commandCallbacks[currFuncIndex] = callback;
	ExternalInterface.call("SendCommand", cmd, cmd_data, currFuncIndex);
}


public function Log(str: String): void
{
	trace(str);
	logArea.text += "\n" + str;
}


/**
 * Sets the connected status of the debugger.
 */
public function SetConnected(conn:Boolean): void
{
	if (conn)
	{
		Log("Connected to debugger successfully");
	}
	else
	{
		Log("Disconnected to debugger");
	}
}

public function isConnected(): Boolean
{
	return ExternalInterface.call("IsConnected");
}

/**
 * Sets a given file as the current file in the text editor.
 */
public function setCurrentFile(fullPath: String, contents: String = null): void 
{
    var input:IDataInput = null; //contents;
    if (contents == null)
    {
		sourceFileArea.setDocument(null);
    }
    else
    {
		currDoc = new DefaultDocumentTokenizer().tokenize(fullPath, input);
		sourceFileArea.setDocument(currDoc);
    }
}

/**
 * Gets the currently open file in the editor.
 */
public function getDocument(): IDocument
{
	return currDoc;
}

private function OnConnected(): void
{
	connectDialog.setVisible(false);
	SetConnected(true);
}

private function onConnectMI(event: *): void
{
	connectDialog = new ConnectDialog();
	connectDialog.theApp = this;
	connectDialog.show(true, this);
	// connect("", 0, this);
}

public function connect(host: String, port: int, progress:*): Boolean
{
	// ExternalInterface.call("DoHandshake", host, port);
	ExternalInterface.call("DoHandshake");
	return true;
}

// List of commands we can send to the debugger
private function CommandEvaluate(context: String, expr:String): void
{
	function result_callback(result: *): void
	{
	}
	SendCommand("eval", {'context': context, 'expr_str': expr}, result_callback);
}


/*
public function SubscribeToChannel(channel:String): void
{
	if (!isConnected())
		return ;
		
	var uri:String = "http://" + gameHost + ":" + gamePort + "/bayeux/";
	var urlreq:URLRequest = new URLRequest(uri);
	var urldata:String =  JsonParser.JsonEncode({'channel': '/meta/subscribe', 
												 'subscription': channel,
				   		   						 'clientId': currClientId});
	urlreq.data =  urldata;
	urlreq.method = "POST";
	var urlstream:URLStream 	= new URLStream();
	var jsonParser:JsonParser 	= new JsonParser();
	
	function urlStreamEventHandler(evt: Event): void
	{
		Log("Event: " + evt.type + ", " + evt.toString());
		if (urlstream.bytesAvailable > 0)
		{
			jsonParser.processInput(urlstream);
			var nextObject:* = jsonParser.next();
			if (nextObject != null)
			{
				Log("Next Object: " + JsonParser.JsonEncode(nextObject));
			}
		}
	}
	
	urlstream.addEventListener(Event.COMPLETE, urlStreamEventHandler);
	urlstream.addEventListener(Event.OPEN, urlStreamEventHandler);
	urlstream.addEventListener(HTTPStatusEvent.HTTP_STATUS, urlStreamEventHandler);
	urlstream.addEventListener(IOErrorEvent.IO_ERROR, urlStreamEventHandler);
	urlstream.addEventListener(ProgressEvent.PROGRESS, urlStreamEventHandler);
	urlstream.addEventListener(SecurityErrorEvent.SECURITY_ERROR, urlStreamEventHandler);
	urlstream.load(urlreq);
}

public function connect(host: String, port: int, progress:*): Boolean
{
	if (isConnected())
		return true;
		
	connecting 		= true;
	cancelConnect 	= false;
	gameHost		= host;
	gamePort		= port;
	jsonParser		= new JsonParser();
	
	var uri:String = "http://" + host + ":" + port + "/bayeux/";
	var urlreq:URLRequest = new URLRequest(uri);
	var urldata:String =  JsonParser.JsonEncode({'channel': '/meta/handshake', 'version': '1.0',
				   		   						 'supportedConnectionTypes':
				   		   						  	['long-polling', 'callback-polling', 'iframe']});
	urlreq.data =  urldata;
	urlreq.method = "POST";
	var loader:URLLoader = new URLLoader();
	
	function loaderEventHandler(evt: Event): void
	{
		Log("Event: " + evt.type + ", " + evt.toString());
		if (evt.type == Event.COMPLETE)
		{
			var ldr: URLLoader = evt.currentTarget as URLLoader;
			var jp:JsonParser = new JsonParser();
			var ba:ByteArray = new ByteArray();
			ba.writeUTF(ldr.data);
			ba.position = 0;
			jp.processInput(ba);
			var response:* = jp.next();
			currClientId = response['clientId'];
			
			connected = true;
			
			// now subscribe to it!
			SubscribeToChannel("/ldb");
			SubscribeToChannel("/channel1");
			SubscribeToChannel("/channel2");
			SubscribeToChannel("/channel3");
			SubscribeToChannel("/channel4");
			SubscribeToChannel("/channel5");
		}
	}
	
	loader.addEventListener(Event.COMPLETE, loaderEventHandler);
	loader.addEventListener(Event.OPEN, loaderEventHandler);
	loader.addEventListener(HTTPStatusEvent.HTTP_STATUS, loaderEventHandler);
	loader.addEventListener(IOErrorEvent.IO_ERROR, loaderEventHandler);
	loader.addEventListener(ProgressEvent.PROGRESS, loaderEventHandler);
	loader.addEventListener(SecurityErrorEvent.SECURITY_ERROR, loaderEventHandler);
	loader.load(urlreq);
	return true;
}

public function disconnect(): void
{
}
*/