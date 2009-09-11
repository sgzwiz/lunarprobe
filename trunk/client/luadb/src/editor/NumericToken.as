package editor
{
    public class StringToken extends Token
    {
        public var value: Number = 0;

        public function NumericToken(v: Number = 0)
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