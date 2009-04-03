// ActionScript file

import flash.display.*;
import flash.external.*;
import flash.media.*;

import mx.collections.*;
import mx.containers.TitleWindow;
import mx.controls.ProgressBar;
import mx.managers.PopUpManager;
import mx.events.*;
import core.*;
import views.*;

private var progressBarDialog:TitleWindow     = null;
private var theProgressBar:ProgressBar        = null;

private function onCreationComplete(event: Object): void
{
    ExternalInterface.addCallback("FileListFetched", null);
    ExternalInterface.addCallback("ContextsLoaded", null);
    ExternalInterface.addCallback("FileLoaded", null);
    
    
    // Tell the caller that we are ready to do the real bidding of our master!!!
    ExternalInterface.call("LuaDBLoaded");
}

public function Log(str:String): void
{
	trace(str);
}