package editor
{
    public class NumericToken extends Token
    {
        public var value: String = "";

        public function StringToken(v: String = "")
        {
            value = v;
        }
        
        // Textual representation of the token
        public function get tokenText(): String
        {
            return value;
        }
        
        // Value of the token
        public function get tokenValue(): *
        {
            return value;
        }
    }
}