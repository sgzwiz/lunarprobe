package dialogs
{
	import flash.display.DisplayObject;
	import flash.events.Event;
	import flash.events.MouseEvent;
	
	import mx.containers.HBox;
	import mx.containers.TitleWindow;
	import mx.containers.VBox;
	import mx.controls.Button;
	import mx.core.Container;
	import mx.core.UIComponent;
	import mx.events.CloseEvent;
	import mx.managers.PopUpManager;

	/**
	 * Super class of all OkCancel dialogs
	 */
	public class OkCancelDialog extends TitleWindow
	{
		public var theApp:LDBApp			= null;
		protected var vbox:VBox				= new VBox();
		protected var holder:UIComponent	= null;
		protected var okButton:Button 		= new Button();
		protected var cancelButton:Button 	= new Button();
		protected var cancelled:Boolean 	= false;
		 
		public function OkCancelDialog()
		{
			super();
			this.showCloseButton = true;
			super.addChildAt(vbox, 0);
			//vbox.addChild(holder);
			var hbox:HBox = new HBox();
			vbox.addChild(hbox);
			hbox.setStyle("align", "center");
			setConstraint(vbox, "width", "100%");
			setConstraint(vbox, "height", "100%");
			setConstraint(hbox, "width", "100%");
			setConstraint(hbox, "height", "100%");
			hbox.addChild(okButton);
			hbox.addChild(cancelButton);
			okButton.label = "Ok";
			cancelButton.label = "Cancel";
			setConstraint(okButton, "width", "50%");
			setConstraint(cancelButton, "width", "50%");
			
			addEventListener(CloseEvent.CLOSE, cancel);
			okButton.addEventListener(MouseEvent.CLICK, okHandler);
			cancelButton.addEventListener(MouseEvent.CLICK, cancel);
		}
		
		public function setConstraint(item: *, constraint: String, value: String): void
		{
			// item.setConstraintValue(constraint, value);
			item.setStyle(constraint, value);
		}
		
		override public function addChild(child:DisplayObject):DisplayObject
		{
			if (holder == null)
			{
				holder = UIComponent(child);
				return super.addChildAt(child, 0);
			}
			
			if (holder is Container)
				return Container(holder).addChild(child);
				
			return null;
		}
		
		public function Cancelled():Boolean
		{
			return cancelled;
		}
		
		override public function addChildAt(child:DisplayObject, index:int):DisplayObject
		{
			if (holder == null)
			{
				holder = UIComponent(child);
				return super.addChildAt(child, 0);
			}
			
			if (holder is Container)
				return Container(holder).addChildAt(child, index);
				
			return null;
		}
		
		public function show(visible:Boolean = true, parent:DisplayObject = null):void
		{
			if (visible)
			{
				setVisible(true);
				PopUpManager.addPopUp(this, parent, true);
				PopUpManager.centerPopUp(this);
				cancelled = false;
			}
			else
			{
				PopUpManager.removePopUp(this);	
				this.setVisible(false);
			}
		}
		
		protected function cancel(event:Event = null):void
		{
			cancelled = true;
			show(false);
		}
		
		protected function okHandler(event:Event):void
		{
			if (event)
				event.stopPropagation();
			success();
		}
		
		protected function success():void
		{
			cancelled = false;
			show(false);
		}
	}
}
