
package utils
{
    import flash.utils.IDataInput;
    
    /*
     * On-demand parser for parsing json from a stream
     */
    public class JsonParser
    {
        private static var STATE_EOF:int                  = -1;
        private static var STATE_START:int                = 0;
        private static var STATE_READING_IDENTIFIER:int    = 1;
        private static var STATE_READING_NUMBER:int       = 2;
        private static var STATE_READING_STRING:int       = 3;
    
        private static var OBJ_NULL:int         = 0;
        private static var OBJ_BOOL:int         = 1;
        private static var OBJ_NUMBER:int       = 2;
        private static var OBJ_STRING:int       = 3;
        private static var OBJ_LIST:int         = 4;
        private static var OBJ_STRUCT:int       = 5;
        private static var OBJ_COMA:int         = 6;
        private static var OBJ_COLON:int        = 7;
        private static var OBJ_OPEN_BRACE:int   = 8;
        private static var OBJ_OPEN_SQUARE:int  = 9;
        private static var OBJ_CLOSE_BRACE:int  = 10;
        private static var OBJ_CLOSE_SQUARE:int = 11;
        
        /*
         * Current string delimiter only valid if we are
         * currently reading a string
         */
        private var currDelim:int;
        
        /*
         * Current string - only if we are reading a string
         */
        private var currString:String;
        
        /*
         * State in reading current string
         */
        private var currStringState:int;

        /*
         * The byte buffer
         */
        private var byteBuffers: Array = new Array();

        /*
         * 
         */
        private var currBuffer: IDataInput;
        
        /*
         * Number of bytes read since start of object
         */
        private var bytesRead: int;
        
        /*
         * Current parser state
         */
        private var currState: int;
        
        /*
         * How many unmatched open square brackets have we encountered?
         */
        private var squareCount: int = 0;
                
        /*
         * How many unmatched open braces have we encountered?
         */
        private var braceCount: int = 0;
        
        /*
         * Current parent object
         */
        private var objStack: Array = null;
        
        
        public static const CHAR_BSLASH:int 	= '\/'.charCodeAt(0);
        public static const CHAR_DQUOTE:int 	= '"'.charCodeAt(0);
        public static const CHAR_SLASH:int 		= '\\'.charCodeAt(0);
        
        public static const CHAR_SLASH_B:int 	= '\b'.charCodeAt(0);
        public static const CHAR_SLASH_F:int 	= '\f'.charCodeAt(0);
        public static const CHAR_SLASH_N:int 	= '\n'.charCodeAt(0);
        public static const CHAR_SLASH_R:int 	= '\r'.charCodeAt(0);
        public static const CHAR_SLASH_V:int 	= '\v'.charCodeAt(0);
        public static const CHAR_SLASH_T:int 	= '\t'.charCodeAt(0);
        
        public static const CHAR_SQUOTE:int = '\''.charCodeAt(0);
        public static const CHAR_PLUS:int = '+'.charCodeAt(0);
        public static const CHAR_DOT:int = '.'.charCodeAt(0);
        public static const CHAR_a:int = 'a'.charCodeAt(0);
        public static const CHAR_A:int = 'A'.charCodeAt(0);
        public static const CHAR_e:int = 'e'.charCodeAt(0);
        public static const CHAR_E:int = 'E'.charCodeAt(0);
        public static const CHAR_t:int = 't'.charCodeAt(0);
        public static const CHAR_v:int = 'v'.charCodeAt(0);
        public static const CHAR_b:int = 'b'.charCodeAt(0);
        public static const CHAR_f:int = 'f'.charCodeAt(0);
        public static const CHAR_r:int = 'r'.charCodeAt(0);
        public static const CHAR_n:int = 'n'.charCodeAt(0);
        public static const CHAR_z:int = 'z'.charCodeAt(0);
        public static const CHAR_Z:int = 'Z'.charCodeAt(0);
        public static const CHAR_0:int = '0'.charCodeAt(0);
        public static const CHAR_9:int = '9'.charCodeAt(0);
        public static const CHAR_SPACE:int = ' '.charCodeAt(0);
        public static const CHAR_MINUS:int = '-'.charCodeAt(0);
        public static const CHAR_UND:int = '_'.charCodeAt(0);
        public static const CHAR_TAB:int = '\t'.charCodeAt(0);
        public static const CHAR_CR:int = '\r'.charCodeAt(0);
        public static const CHAR_LF:int = '\n'.charCodeAt(0);
        public static const CHAR_COMA:int = ','.charCodeAt(0);
        public static const CHAR_COLON:int = ':'.charCodeAt(0);
        public static const CHAR_OSQ:int = '['.charCodeAt(0);
        public static const CHAR_CSQ:int = ']'.charCodeAt(0);
        public static const CHAR_OBRACE:int = '{'.charCodeAt(0);
        public static const CHAR_CBRACE:int = '}'.charCodeAt(0);
                
        /*
         * Constructor
         */
        public function JsonParser()
        {
            reset();
        }
		        
		public static function JsonEncode(object: *): String
		{
			var out: String = "";
			if (object is Number)
			{
				out = (object as Number).toString();
			}
			else if (object is Boolean)
			{
				out = (object as Boolean).toString();
			}
			else if (object is String)
			{
				var str:String = object as String;
				out = "\"";
				for (var i:int = 0;i < str.length;i++)
				{
					var ch:int = str.charCodeAt(i);
					if (ch == CHAR_BSLASH) out += "\\/";
					else if (ch == CHAR_DQUOTE) out += "\\\"";
					else if (ch == CHAR_SLASH) out += "\\\\";
					else if (ch == CHAR_SLASH_B) out += "\\b";
					else if (ch == CHAR_SLASH_F) out += "\\f";
					else if (ch == CHAR_SLASH_N) out += "\\n";
					else if (ch == CHAR_SLASH_R) out += "\\r";
					else if (ch == CHAR_SLASH_V) out += "\\v";
					else if (ch == CHAR_SLASH_T) out += "\\t";
                   	else out += str.charAt(i);
				}
				out += "\"";
			}
			else if (object is Array)
			{
				var childArray:Array = object as Array;
				out = "[ ";
				for (var index:int = 0;index < childArray.length;index++)
				{
					if (index > 0) out += ", ";
					out += JsonEncode(childArray[index]);
				}
				out += " ]";
			}
			else if (object is Object)
			{
				out = "{ ";
				for (var key:String in object)
				{
					out += JsonEncode(key) + ": ";
					out += JsonEncode(object[key]);
				}
				out += " }";
			}
			else
			{
				throw new Error("Invalid object type in json encoding: " + object.toString());
			}
			return out;
		}
		

        /*
         * Resets the parser
         */
        public function reset(resetBytes:Boolean = true): void
        {
            objStack        = [];
            currState       = STATE_START;
            bytesRead       = 0;
            currString      = "";
            currStringState = 0;
            squareCount     = 0;
            braceCount      = 0;
            if (resetBytes)
            {
                byteBuffers = new Array();
                currBuffer  = null;
            }
        }

        /*
         * Process more bytes.  
         *
         * Returns nothing.  Call next() to retrieve the next object
         * from the stream (if any).
         */
        public function processInput(data:IDataInput): void
        {
            byteBuffers.push(data);
        }

        /*
         * Return the next object from the stream.
         * null if none (this is not necessarily an error, just means we do
         * not have enough bytes given to us to extract the next object).
         */
        public function next(): *
        {
            var currCh:*;
            var readChar:Boolean = true;
            
            while ( ! (objStack.length == 1 && squareCount == 0 && braceCount == 0))
            {
                if (currBuffer == null || currBuffer.bytesAvailable == 0)
                {
                    if (byteBuffers.length == 0)
                    {
                        return null;
                    }
                    currBuffer = byteBuffers.pop();
                }
                
                if (readChar)
                {
                    if (currBuffer.bytesAvailable == 0)
                        return null;
                    bytesRead++;
                    currCh = currBuffer.readByte();
                }
                else
                {
                    readChar = true;
                }

                if (currState == STATE_READING_STRING)
                {
                    processStringBytes(currCh);
                }   
                else if (currState == STATE_READING_IDENTIFIER)
                {
                    if ((currCh >= CHAR_a && currCh <= CHAR_z)         || 
                             (currCh >= CHAR_A && currCh <= CHAR_Z)     ||
                             (currCh >= CHAR_0 && currCh <= CHAR_9)     || 
                             currCh == CHAR_UND)
                    {
                        currString += String.fromCharCode(currCh);                        
                    }
                    else
                    {
                        if (currString == "true")
                        {
                            shift(OBJ_BOOL, true);
                        }
                        else if (currString == "false")
                        {
                            shift(OBJ_BOOL, false);
                        } 
                        else if (currString == "null")
                        {
                            shift(OBJ_NULL, null);
                        } 
                        else
                        {
                            shift(OBJ_STRING, currString);
                            //throw new Error("Invalid identifier: " + currString);
                        }
                        
                        currState = STATE_START;
                        readChar = false;
                    }
                }
                else if (currState == STATE_READING_NUMBER)
                {
                    if ( ! processNumberBytes(currCh))
                    {
                        readChar = false;
                    }
                }             
                else if (currState == STATE_START)
                {
                    if (currCh == CHAR_SPACE || currCh == CHAR_TAB || 
                        currCh == CHAR_CR || currCh == CHAR_LF)
                    {
                        // ignore white spaces
                    }
                    else if ((currCh >= CHAR_a && currCh <= CHAR_z)         || 
                             (currCh >= CHAR_A && currCh <= CHAR_Z)     ||
                             currCh == CHAR_UND)
                    {
                        currState = STATE_READING_IDENTIFIER;
                        currString = String.fromCharCode(currCh);
                    }
                    else if (currCh == CHAR_MINUS || (currCh >= CHAR_0 && currCh <= CHAR_9))
                    {
                        currState = STATE_READING_NUMBER;
                        currStringState = 0;
                        processNumberBytes(currCh);
                    }
                    else if (currCh == CHAR_SQUOTE || currCh == CHAR_DQUOTE)
                    {
                        currDelim = currCh;
                        currString = "";
                        currState = STATE_READING_STRING;
                    }
                    else if (currCh == CHAR_COMA)
                    {
                        shift(OBJ_COMA);
                    }
                    else if (currCh == CHAR_COLON)
                    {
                        shift(OBJ_COLON);
                    }
                    else if (currCh == CHAR_OSQ)
                    {
                        squareCount++;
                        shift(OBJ_OPEN_SQUARE);
                    }
                    else if (currCh == CHAR_OBRACE)
                    {
                        braceCount++;
                        // shift(OBJ_STRUCT, new Object());
                        shift(OBJ_OPEN_BRACE);
                    }
                    else if (currCh == CHAR_CSQ)
                    {
                        squareCount--;
                        shift(OBJ_CLOSE_SQUARE);
                    }
                    else if (currCh == CHAR_CBRACE)
                    {
                        braceCount--;
                        shift(OBJ_CLOSE_BRACE);
                    }
                }
            }

            if (objStack.length == 1)
            {
                var obj:Object = objStack.pop();
                reset(false);
                return obj['value'];
            }

            return null;
        }
        
        /*
         * Shifts a token onto the stack and immediately reduces it (the
         * stack)
         */
        private function shift(tok:int, value: * = null): void
        {
            if (tok == OBJ_CLOSE_SQUARE || tok == OBJ_CLOSE_BRACE)
            {
                reduce(tok);
            }
            else
            {
                objStack.push({'type': tok, 'value': value});
            }
        }

        /* 
         * Reduces the parse stackby utilising the next token.
         * Reduction can happen more than once.
         */
        private function reduce(topType:int): void
        {
            var t1: *;
            var t2: *;

            if (topType == OBJ_CLOSE_SQUARE)
            {
                // we are building a list
                var newArray: Array = new Array();
                while (objStack.length > 0 &&
                        objStack[objStack.length - 1]['type'] != OBJ_OPEN_SQUARE)
                {
                    t1 = objStack.pop();
                    if (t1['type'] == OBJ_COMA)
                        t1 = objStack.pop();
                    newArray.push(t1['value']);
                }
                if (objStack.length == 0 || objStack[objStack.length - 1]['type'] != OBJ_OPEN_SQUARE)
                {
                    throw new Error("Could not find '['");
                }
                objStack.pop();
                newArray.reverse();
                objStack.push({'type': OBJ_LIST, 'value': newArray});
            }
            else
            {
                // we are building a map
                var newObj: Object = new Object();
                while (objStack.length > 0 &&
                        objStack[objStack.length - 1]['type'] != OBJ_OPEN_BRACE)
                {
                    t1 = objStack.pop();
                    if (t1['type'] == OBJ_COMA)
                    {
                        t1 = objStack.pop();
                    }

                    t2 = objStack.pop();
                    if (t2['type'] != OBJ_COLON)
                    {
                        throw new Error("':' expected but not found");
                    }
                    t2 = objStack.pop();

                    newObj[t2['value']] = t1['value'];
                }
                if (objStack.length == 0 || objStack[objStack.length - 1]['type'] != OBJ_OPEN_BRACE)
                {
                    throw new Error("Could not find '{'");
                }
                objStack.pop();
                objStack.push({'type': OBJ_STRUCT, 'value': newObj});
            }
        }
        
        private function processNumberBytes(currCh: *): Boolean
        {
            var NUM_START:int               = 0;
            var NUM_READING_INT:int         = 1;
            var NUM_READING_DOT:int         = 2;
            var NUM_READING_FRAC:int        = 3;
            var NUM_READING_EXP:int         = 4;
            var NUM_READING_EXP_DIGITS:int  = 5;
            
            if (currStringState == NUM_START)
            {
                currString         = "";
                currStringState = NUM_READING_INT;
                if (currCh == CHAR_MINUS)
                {
                    currString     = '-';
                    return true;
                }
            }
            
            if (currStringState == NUM_READING_INT)
            {
                if (currCh >= CHAR_0 && currCh <= CHAR_9)
                {
                    currString += String.fromCharCode(currCh);
                    return true;
                }
                else
                {
                    currStringState = NUM_READING_DOT;
                }
            }
            
            if (currStringState == NUM_READING_DOT)
            {
                if (currCh == CHAR_DOT)
                {
                    currString += '.';
                    return true;
                }
                else if (currCh == CHAR_e || currCh == CHAR_E)
                {
                    currString += 'e';
                    currStringState = NUM_READING_EXP;
                    return true;
                }
                
                currStringState = NUM_READING_FRAC;
            }
            
            if (currStringState == NUM_READING_FRAC)
            {
                if (currCh >= CHAR_0 && currCh <= CHAR_9)
                {
                    currString += String.fromCharCode(currCh);
                    return true;
                }
                else if (currCh == CHAR_e || currCh == CHAR_E)
                {
                    currString += 'e';
                    currStringState = NUM_READING_EXP;
                    return true;
                }
            }
            
            if (currStringState == NUM_READING_EXP)
            {
                currStringState = NUM_READING_EXP_DIGITS; 
                if (currCh == CHAR_PLUS)
                {
                    currString += '+';
                    return true;
                }
                else if (currCh == CHAR_MINUS)
                {
                    currString += '-';
                    return true;
                }
            }
            
            if (currStringState == NUM_READING_EXP_DIGITS)
            {
                if (currCh >= CHAR_0 && currCh <= CHAR_9)
                {
                    currString += String.fromCharCode(currCh);
                    return true;
                }
            }
            
            currState = STATE_START;
            shift(OBJ_NUMBER, Number(currString));
            
            // invalid char, must be end of numeric
            // value so let the caller take care of the
            // rest by not consuming the character
            return false;
        }
        
        private function processStringBytes(currCh: *): void
        {
            var STR_NORMAL:int = 0;
            var STR_SLASH:int  = 1;
            
            if (currStringState == STR_SLASH)
            {
                currStringState = STR_NORMAL
                if (currCh == CHAR_SLASH)
                {
                    currString += '\\';
                }
                else if (currCh == CHAR_t)
                {
                    currString += '\t';
                }
                else if (currCh == CHAR_n)
                {
                    currString += '\n';
                }
                else if (currCh == CHAR_r)
                {
                    currString += '\r';
                }
                else if (currCh == CHAR_b)
                {
                    currString += '\b';
                }
                else if (currCh == CHAR_f)
                {
                    currString += '\f';
                }
                else if (currCh == CHAR_BSLASH)
                {
                    currString += '\/';
                }
                else if (currCh == CHAR_DQUOTE)
                {
                    currString += '"';
                }
                else if (currCh == CHAR_SQUOTE)
                {
                    currString += '\'';
                }
                else
                {
                    currString += ("\\" + currCh);
                }
            }
            else if (currCh == currDelim)
            {
                currState = STATE_START;
                shift(OBJ_STRING, currString);
            }
            else
            {
                currString += String.fromCharCode(currCh);
            }
        }
    }
}

