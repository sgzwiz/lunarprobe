package editor
{
    /**
     * Styles that apply to how a document appears.
     */
    public class Style
    {
        /**
         * Number of child style elements.
         */
        public function get childCount(): int
        {
            return 0;
        }
        
        /**
         * Get the child style at a given index.
         */
        public function getChild(index: int): Style
        {
            return null;
        }
        
        /**
         * Set the style at a given index.
         */
        public function setChild(index: int, style: Style): void
        {
        }
        
        /**
         * Add a child style at a given position.
         */
        public function addChild(style: Style, index: int): void
        {
        }
        
        /**
         * Remove a child at a given index.
         */
        public function removeChild(index: int): Style
        {
            return null;
        }
        
        /**
         * Removes all child styles.
         */
        public function clear(): void
        {
        }
        
        /**
         * Are there any children?
         */
        public function hasChildren(): Boolean
        {
            return false;
        }
    }
}