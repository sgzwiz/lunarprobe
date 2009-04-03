
package docker
{
    import flash.events.Event;
    import mx.containers.Panel;
    import mx.containers.Canvas;
    import mx.controls.Alert;
    import mx.controls.Button;

    // Doc containers are always Boxes
    public class DockContainer extends Canvas
    {
        // Gets the child at a given index
        public override function addDocChild(child: DisplayObject, docinfo: Object): void
        {
            addDockChildAt(index, docinfo, -1);
        }

        public override function addChildAt(child: DisplayObject, docinfo: Object, int index)
        {
        }
    }
}

