
function createPanelContainer(panelDivName, parentContainer, vertical)
{
    // root container
    var newOverlay = new YAHOO.widget.Module(panelDivName, {
        visible: true,
        splitterChildren: [],
        splitterVertical: vertical,
        parentContainer: parentContainer,
        resizer: null,
        nextSibbling: null,
        prevSibbling: null,
    });
    newOverlay.render();
    return newOverlay;
}

function createResizablePanel(panelDivName, panelName)
{
    var x           = 0;
    var y           = 0;
    var width       = 250;
    var height      = 300;
    var cookieName  = panelName + "Layout";
    var cookieValue = YAHOO.util.Cookie.get(cookieName);
    if (cookieValue != null)
    {
    }

    var outPanel = new YAHOO.widget.Panel(panelDivName, {
        xy: [x, y],
        width: width + "px",
        height: height + "px",
        visible: true,
        close: false,
        constraintoviewport: true,
    });
    panels[panelName] = outPanel;
    outPanel.render();

    var panelResizer = new YAHOO.util.Resize(panelDivName, {
        // handles: 'all',
        handles: ['b', 'r', 'br'],
        animate: true,
        animateDuration: .75,
        animateEasing: YAHOO.util.Easing.backBoth,
        proxy: true,
        status: false
    });

    panelResizer.on("startResize", function(args) {
        if (this.cfg.getProperty("constraintoviewport")) {
            var D = YAHOO.util.Dom;
            var clientRegion = D.getClientRegion();
            var elRegion = D.getRegion(this.element);
            panelResizer.set("maxWidth", clientRegion.right - elRegion.left - YAHOO.widget.Overlay.VIEWPORT_OFFSET);
            panelResizer.set("maxHeight", clientRegion.bottom - elRegion.top - YAHOO.widget.Overlay.VIEWPORT_OFFSET);
        }
        else
        {
            panelResizer.set("maxWidth", null);
            panelResizer.set("maxHeight", null);
        }
    }, outPanel, true);

    panelResizer.on("resize", function(args) {
        this.cfg.setProperty("x", args.left + "px");
        this.cfg.setProperty("y", args.top + "px");
        this.cfg.setProperty("width", args.width + "px");
        this.cfg.setProperty("height", args.height + "px");
        Log("l, t, w, h: " + args.left + ", " + args.top + ", " + args.width + ", " + args.height);
    }, outPanel, true);

    panelResizer.on("endResize", function(args) {
        // save the panels dimensions into the cookies!
        Log("Resize finished");
    });

    panelResizers[panelName]    = panelResizer;

    return outPanel;
}

function removePanel(panel)
{
    var panelCfg    = panel.cfg;
    var prevPanel   = panelCfg.getProperty("prevPanel");
    var nextPanel   = panelCfg.getProperty("nextPanel");
    if (prevPanel == null && nextPanel == null)
    {
        // both are null so remove ourselves and also our parent splitter
        // recursively
    }
    else
    {
        if (prevPanel != null)
            prevPanel.cfg.setProperty("nextPanel", nextPanel);
        if (nextPanel != null)
            nextPanel.cfg.setProperty("prevPanel", prevPanel);
    }
    return panel;
}

function addPanelToOverlay(panel, parentOverlay, before, vertical)
{
    var splitterCfg = parentOverlay.cfg;
    var panelCfg    = panel.cfg;
    var isVertical  = splitterCfg.getProperty("isVertical");
    if (typeof vertical === "undefined" || vertical == null)
    {
        vertical = isVertical;
    }
    if (typeof before === "undefined" || before == null)
    {
        before = -1;
    }

    // we have the following cases:
    // 1. The parentOverlay is empty - then just add this item as a child
    //    with 100% size
    // 2. Parent has atleast one overlay and we are adding on the same direction,
    //    then we just insert the panel at the given position
    // 3. Parnet has atleast one overlay and we are doing it in the
    //    opposite direction.  In this case add another SplitterOverlay at
    //    the given position, remove the item at "position" and add the old
    //    item and the new item as children of the new overlay.  

    var siblings = splitterCfg.getProperty("splitterChildren");
    if (siblings == null)
    {
        siblings = [];
        splitterCfg.setProperty("splitterChildren", siblings);
    }

    if (siblings.length == 0)
    {
        removePanel(panel);
        siblings.push(panel);
        parentOverlay.appendToBody(panel);
    }
    else
    {
    }
    // remove the panel from the old parent (but do not destroy it).
    var removedPanel = removePanel(panel);
    if (removedPanel != panel)
    {
        alert("Removed panel MUST equal panel");
        assert(false);
    }
}

function createPanels()
{
    rootOverlay = createPanelContainer("rootOverlay", null, true);
    rootOverlay.render();
    // rootOverlay.setBody("<div>Hello World</div>");

    filesPanel = createResizablePanel("filesPanel", "files");

    addPanelToOverlay(filesPanel, rootOverlay);

    /*
    breakpointsPanel = createResizablePanel("breakpointsPanel", "breakpoints");
    debugStackPanel = createResizablePanel("debugStackPanel", "debugStack");
    debugContextsPanel = createResizablePanel("debugContextsPanel", "debugContexts");
    variablesPanel = createResizablePanel("variablesPanel", "variables");
    optionsPanel = createResizablePanel("optionsPanel", "options");
    navigatorPanel = createResizablePanel("navigatorPanel", "navigator");
    consolePanel = createResizablePanel("consolePanel", "console");

    overlayManager = new YAHOO.widget.OverlayManager();
    overlayManager.register([filesPanel,
                             consolePanel,
                             optionsPanel,
                             variablesPanel,
                             breakpointsPanel,
                             debugStackPanel,
                             debugContextsPanel,
                             navigatorPanel]);
    */

    Log("all items created....");
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

function Log(message)
{
    var consoleArea = $("#consoleArea");
    consoleArea.val(consoleArea.val() + message + "\n");
}

