package editor
{
    /**
     * A code file is any thing that is source code (including config files) 
     * that is displayed as text.
     * 
     * At its essence, it consists of tokens and it is upto the renderer to
     * show them 
     */
    public interface IDocument
    {
        // Name of the file
        function get name(): String;
        
        // Get number of tokens in the file.
        function get tokenCount(): int;
        
        // Get number of lines in the file.
        function get lineCount(): int;
        
        // Adds a new token to a given line at a given position
        function addToken(token:Token, line: int = -1, position: int = -1): void;
        
        // Adds a new line before a given line
        function addNewLine(position: int = -1): void;
        
        // Get the line number the token is in.
        function getTokenLinePosition(token: int): int;
        
        // Get the character position the token is in.
        function getTokenCharacterPosition(token: int): int;
        
        // Get the nth token in a given line
        function getTokenInLine(line: int, token: int): Token;
        
        // Get the nth token in the entire file
        function getToken(token: int): Token;
        
        // The root style that applies to the document.
        function get rootStyle(): Style;
    }
}