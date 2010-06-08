package editor
{
    import flash.events.*;
    import flash.text.*;
    
    import mx.core.UIComponent;

    public class DocumentEditor extends UIComponent
    {
        // Current code file being edited/displayed
        protected var currDoc:IDocument = null;

        // The selection model.
        protected var selectionModel:SelectionModel = new SelectionModel();

        // The document renderer
        protected var renderer:IDocumentView = null;

        /**
         * Constructor
         */
        public function DocumentEditor()
        {
            super();
            addEventListener(MouseEvent.CLICK, handleMouseEvent);
            addEventListener(MouseEvent.DOUBLE_CLICK, handleMouseEvent);
            addEventListener(MouseEvent.MOUSE_DOWN, handleMouseEvent);
            addEventListener(MouseEvent.MOUSE_MOVE, handleMouseEvent);
            addEventListener(MouseEvent.MOUSE_OUT, handleMouseEvent);
            addEventListener(MouseEvent.MOUSE_OVER, handleMouseEvent);
            addEventListener(MouseEvent.MOUSE_UP, handleMouseEvent);
            addEventListener(MouseEvent.MOUSE_WHEEL, handleMouseEvent);
            addEventListener(MouseEvent.ROLL_OUT, handleMouseEvent);
            addEventListener(MouseEvent.ROLL_OVER, handleMouseEvent);
        }
        
        /**
         * Sets the code file to display.
         */
        public function setDocument(doc: IDocument): void
        {
            if (currDoc != doc)
            {
                currDoc = doc;
                invalidateDisplayList();
            }
        }

        protected function handleMouseEvent(event:MouseEvent): void
        {
        }
    }
}