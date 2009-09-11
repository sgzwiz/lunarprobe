package editor
{
    /**
     * An interface to render a document as a text file.
     */
    public interface IDocumentView
    {
        // Get the size of a token
        function getTokenSize(token: int): Size;
        
        // Gets a line's dimensions
        function getLineSize(line: int): Size;
        
        // Gets document dimensions
        function getTotalSize(): Size;
        
        // Get the line at given coordinates
        function getLineAt(x: int, y: int): int;
        
        // Get the token at given coordinates
        function getTokenAt(x: int, y: int): int;

        // Get the scroll position
        function getScrollOffset(): Point;
        
        // Get the scroll position
        function scrollTo(x: int, y: int): void
    }
}