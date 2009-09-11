package editor
{
    import flash.utils.*;

    /**
     * A tokenizer that simply returns tokens split by spaces (and newlines).
     */
    public class DefaultDocumentTokenizer implements IDocumentTokenizer
    {
        public function tokenize(docName: String, input: IDataInput): IDocument
        {
            while (input.bytesAvailable > 0)
            {
            }
            
            return null;
        }
    }
}