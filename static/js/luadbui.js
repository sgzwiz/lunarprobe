
/**
 * Creates the main layout - ie borderlayout using the entire screen.
 * This layout will be our main entry into the LDB UI.
 */
function createMainLayout()
{
    var Dom = YAHOO.util.Dom,
        Event = YAHOO.util.Event;

    Event.onDOMReady(function() {
        theMainLayout = new YAHOO.widget.Layout({
            units: [
                { position: 'top', height: 60, body: 'top1', header: 'Top', gutter: '5px', collapse: true, resize: false },
                { position: 'right', header: 'Right', width: 300, resize: true, gutter: '5px', footer: 'Footer', collapse: true, scroll: true, body: 'right1', animate: true },
                { position: 'bottom', header: 'Console', height: 200, resize: true, body: 'bottom1', gutter: '5px', collapse: true, scroll: true },
                { position: 'left', header: 'Left', width: 200, resize: true, body: 'left1', gutter: '5px', collapse: true, close: true, collapseSize: 50, scroll: true, animate: true },
                { position: 'center', body: 'center1' }
            ]
        });
        theMainLayout.on('render', function() {
            theMainLayout.getUnitByPosition('left').on('close', function() {
                closeLeft();
            });
        });
        theMainLayout.render();
        Event.on('tLeft', 'click', function(ev) {
            Event.stopEvent(ev);
            theMainLayout.getUnitByPosition('left').toggle();
        });
        Event.on('tRight', 'click', function(ev) {
            Event.stopEvent(ev);
            theMainLayout.getUnitByPosition('right').toggle();
        });
        Event.on('padRight', 'click', function(ev) {
            Event.stopEvent(ev);
            var pad = prompt('CSS gutter to apply: ("2px" or "2px 4px" or any combination of the 4 sides)', theMainLayout.getUnitByPosition('right').get('gutter'));
            theMainLayout.getUnitByPosition('right').set('gutter', pad);
        });
        var closeLeft = function() {
            var a = document.createElement('a');
            a.href = '#';
            a.innerHTML = 'Add Left Unit';
            Dom.get('closeLeft').parentNode.appendChild(a);

            Dom.setStyle('tLeft', 'display', 'none');
            Dom.setStyle('closeLeft', 'display', 'none');
            Event.on(a, 'click', function(ev) {
                Event.stopEvent(ev);
                Dom.setStyle('tLeft', 'display', 'inline');
                Dom.setStyle('closeLeft', 'display', 'inline');
                a.parentNode.removeChild(a);
                theMainLayout.addUnit(theMainLayout.get('units')[3]);
                theMainLayout.getUnitByPosition('left').on('close', function() {
                    closeLeft();
                });
            });
        };
        Event.on('closeLeft', 'click', function(ev) {
            Event.stopEvent(ev);
            theMainLayout.getUnitByPosition('left').close();
        });
    });
}

/**
 * Creates a tab container for the files that we are editing/debugging.
 */
function createFilesContainer()
{
    theFilesContainer = new YAHOO.widget.TabView("filesContainer");
}


/**
 * Connects to a script running.
 */
function connectToScript()
{
    /*
    var host = $("#serverHostText").val();
    var port = $("#serverPortText").val();
    alert('Connecting to server on: ' + host + ":" + port);
    */
    DoHandshake();
}

function OnConnected()
{
    alert("We Are Overriding OnConnected");
}

function HandleEvent(result)
{
    alert("We Are Overriding HandleEvent");
}

