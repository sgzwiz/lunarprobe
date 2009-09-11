package editor
{
    public class SelectionModel
    {
        /**
         * List of selections
         */
        protected var selections:Array = new Array();
        
        /**
         * Constructor
         */
        public function SelectionModel()
        {
        }
        
        /**
         * How many selections are there?
         */
        public function getSelectionCount(): int
        {
            return selections.length;
        }
        
        /**
         * Clear all selections.
         */
        public function clearSelections(): void
        {
            selections = new Array();
        }
        
        /**
         * Get the ith selection.
         */
        public function getSelection(i: int): Selection
        {
            return selections[i] as Selection;
        }
        
        /**
         * Set a given selection
         */
        public function setSelection(i: int, sel:Selection): void
        {
            selections[i] = sel;
        }
        
        /**
         * Adds a selection at a given index.
         */
        public function addSelection(sel: Selection, index: int = -1): void
        {
            selections.push(sel);
        }
        
        /**
         * Removes a selection at a given index.
         */
        public function removeSelection(index: int): void
        {
            selections.splice(index);
        }
    }
}