package editor
{
    import flash.utils.IDataInput;
    
    /**
     * A tokenizer that returns tokens for a lua document.
     */
    public class LuaDocumentTokenizer implements IDocumentTokenizer
    {
        /**
         * Tokenizes an input stream and returns the document.
         */
        public function tokenize(docName: String, input: IDataInput): IDocument
        {
            return null;
        }
    }
}