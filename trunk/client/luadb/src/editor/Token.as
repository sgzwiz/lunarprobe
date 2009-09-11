package editor
{
    /**
     * A token is the most generic thing you can have.
     * It is the smallest logical unit in a document.
     */
    public class Token
    {
        public var tokenType: int = 0;
        
        /**
         * Creates a new token.
         */
        public function Token()
        {
        }
        
        // Textual representation of the token
        public function get tokenText(): String
        {
            return "";
        }
        
        // Value of the token
        public function get tokenValue(): *
        {
            return null;
        }
        
        // The previous token
        public function get prevToken(): Token
        {
            return null;
        }
        
        // The next token
        public function get nextToken(): Token
        {
            return null;
        }
    }
}