package editor
{
    import flash.utils.IDataInput;

    // An interface that tokenizes an input text to return a document
    public interface IDocumentTokenizer
    {
        function tokenize(docName: String, input: IDataInput): IDocument
    }
}