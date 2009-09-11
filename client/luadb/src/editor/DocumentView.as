package editor
{
    /**
     * Renders a document.
     */
    public class DocumentView implements IDocumentView
    {
        // A stack of styles used in rendering.
        protected var styleStack:Array = [];
        
        // Document being rendered
        protected var currDocument:IDocument = null;
        
        /**
         * Constructor with a document parameter.
         */
        public function DocumentRenderer(document: IDocument)
        {
        }
    }
}