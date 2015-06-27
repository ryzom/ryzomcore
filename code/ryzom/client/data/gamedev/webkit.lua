--
if not Webkit then
    Webkit = {
        count = 0
    }
end

function Webkit:newWindow(url, width, height)
    local id = "webkit" .. tostring(self.count);
    self.count = self.count + 1;

    local ui = createRootGroupInstance("webkit_browser", id, {
        x = 0,
        y = 0,
        w = width,
        h = height,
        home = url
    })
    ui.active = true;
end

-- window becomes visible (:active=true)
function Webkit:onActive()
    local uiWindow = getUICaller()
    -- debugInfo("-- onActive [" .. uiWindow.id .. "]")

    if not uiWindow.opened then
        uiWindow.opened = true
    end

    if  not uiWindow.active then
        uiWindow.active = true
    end
end

--
function Webkit:onJsDeactive()
    local uiWindow = getUICaller()
    -- debugInfo("-- onJsDeactive [" .. uiWindow.id .. "]")
    runAH(getUICaller(), "webkit_jsdialog_cancel", "");
end

-- window is minimized
function Webkit:onClickHeaderClose()
    -- ui:interface:webkit:header_closed
    local uiWindow = getUICaller().parent
    -- fixme: save current width/height and minimize window
end

-- window is restored from minimized state
function Webkit:onClickHeaderOpen()
    -- ui:interface:webig:header_opened
    local uiWindow = getUICaller().parent

    -- fixme: restore saved width/height
end

function Webkit:onClickHome()
    -- caller is :header_opened:browse_home
    local uiWindow = getUICaller().parent.parent

    -- window "home" attribute is used
    local html = uiWindow:find("html")
    html:browse("home")
end

function Webkit:onClickRedo()
    -- caller is :header_opened:browse_redo
    local uiWindow = getUICaller().parent.parent

    local html = uiWindow:find("html")
    html:browseRedo();
end

function Webkit:onClickUndo()
    -- caller is :header_opened:browse_undo
    local uiWindow = getUICaller().parent.parent

    local html = uiWindow:find("html")
    html:browseUndo();
end

function Webkit:onClickRefresh()
    -- caller is :header_opened:browse_refresh
    local uiWindow = getUICaller().parent.parent

    local html = uiWindow:find("html")
    html:refresh()
end

function Webkit:onGrabKeyboard()
    -- caller is :header_opened:browse_refresh
    local uiWindow = getUICaller().parent.parent

    local html = uiWindow:find("html")
    html.grab_keyboard = not html.grab_keyboard
end

function Webkit:onMenuHome()
    -- :header_closed
    -- :header_opened
    local uiWindow = getUICaller().parent

    local html = uiWindow:find("html")
    html:browse("home")
end

function Webkit:onMenuZoomIn()
    -- :header_closed
    -- :header_opened
    local uiWindow = getUICaller().parent

    local html = uiWindow:find("html")
    html:zoomIn()
end

function Webkit:onMenuZoomOut()
    -- :header_closed
    -- :header_opened
    local uiWindow = getUICaller().parent

    local html = uiWindow:find("html")
    html:zoomOut()
end
function Webkit:onMenuZoomReset()
    -- :header_closed
    -- :header_opened
    local uiWindow = getUICaller().parent

    local html = uiWindow:find("html")
    html:zoomReset()
end

function Webkit:onMenuGrabKeyboard()
    local uiWindow = getUICaller().parent

    local html = uiWindow:find("html")
    html.grab_keyboard = not html.grab_keyboard;
end

function Webkit:onMenuQuit()
    local uiWindow = getUICaller().parent
    deleteUI(uiWindow)
end


