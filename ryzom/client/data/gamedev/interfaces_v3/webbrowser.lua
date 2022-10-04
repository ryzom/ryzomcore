
-- global
WebBrowser = {
  template = "webig_browser",
  apps = {}
}

function WebBrowser:addWindow(id, title, ui)
  local found_app = nil
  for k,app in pairs(self.apps) do
    if app.id == id then
      found_app = app
      app.title = title
      app.uiWindow = ui
      app.winw = ui.w
      app.winh = ui.h
    end
  end
  if found_app == nil then
    local app = {}
    app.id = id
    app.title = ""
    app.url = ""
    app.uiWindow = ui
    app.winid = "ui:interface:" .. id
    app.winw = ui.w
    app.winh = ui.h
    app.minimized = true
    app.activeUrl = ""
    app.closedw = 150
    app.closedh = 0
    table.insert(self.apps, app)
  end
  return ui
end


function WebBrowser:openWindow(id, url)
  -- default value if url is not set
  url = url or "http://app.ryzom.com/"

  local newWindow = false
  local app = self:findAppById(id)

  if not app then
    app = {}
    app.id = id
    app.title = ""
    app.url = url
    -- getUI() object
    app.uiWindow = nil
    app.winid = "ui:interface:" .. id
    app.winw = 780
    app.winh = 500
    app.closedw = 150
    app.closedh = 0
    app.minimized = true
    app.activeUrl = ""

    table.insert(self.apps, app)
  end

  if not app.uiWindow then
    -- if there is window present (eg, 'webig'), then reuse it
    app.uiWindow = getUI(app.winid, false)
    if not app.uiWindow then
      app.uiWindow = createRootGroupInstance(self.template, app.id, {
        x = 0, y = 0, w = app.winw, h = app.winh, home = app.url,
        browse_redo = "ui:interface:" .. app.id .. ":browser:header_opened:browse_redo",
        browse_undo = "ui:interface:" .. app.id .. ":browser:header_opened:browse_undo",
        browse_refresh = "ui:interface:" .. app.id .. ":browser:header_opened:browse_refresh"
      })
      if not app.uiWindow then
        return
      end
      app.uiWindow:center()
    end

    newWindow = true
  end

  app.activeUrl = url

  -- trigger on_open event
  if not app.uiWindow.opened then
    app.uiWindow.opened = true
  end

  -- trigger on_active event
  if not app.uiWindow.active then
    app.uiWindow.active = true
  end

  local html = app.uiWindow:find("html")
  html:browse(url)

  setTopWindow(app.uiWindow)
end

function WebBrowser:findAppById(id)
  for k,app in pairs(self.apps) do
    if app.id == id then
      return app
    end
  end
  return nil
end

function WebBrowser:findAppFromUiCaller()
  -- id = app123
  local id = getUICaller().id:match("ui:interface:([^:]*):?")
  local app = self:findAppById(id)
  if app then
    return app
  end
end

function WebBrowser:onActive()
  if app then
    self:restoreWindow(app)
  end
end

function WebBrowser:onClickHeaderClose()
  local app = self:findAppFromUiCaller()
  if app then
    self:saveWindow(app)
  end
end

function WebBrowser:onClickHeaderOpen()
  local app = self:findAppFromUiCaller()
  if app then
    self:restoreWindow(app)
  end
end

-- save current window dimension and minimize window
function WebBrowser:saveWindow(app)
  app.minimized = true
  app.winw = app.uiWindow.w
  app.winh = app.uiWindow.h
  -- minimize
  app.uiWindow.w = app.closedw
  app.uiWindow.h = app.closedh
end

function WebBrowser:restoreWindow(app)
  if app.minimized then
    app.uiWindow.w = app.winw
    app.uiWindow.h = app.winh
    app.minimized = false
  end
end


------------------------------------------------------------------------------------------------------------
--
function WebBrowser:saveWindowPop(app)
	if app == nil then
		app = self:findAppFromUiCaller()
	end

	if app then
		app.minimized = true
		-- save size
		app.winw = app.uiWindow.w
		app.winh = app.uiWindow.h
		app.pop_min_h = app.uiWindow.pop_min_h
		app.pop_max_h = app.uiWindow.pop_max_h
		app.pop_min_w = app.uiWindow.pop_min_w
		-- minimize
		app.uiWindow.w = app.closedw
		app.uiWindow.h =  app.closedh
		app.uiWindow.pop_min_h = 32
		app.uiWindow.pop_min_w = app.closedw
	end
end

------------------------------------------------------------------------------------------------------------
--
function WebBrowser:restoreWindowPop(app)
	if app == nil then
		app = self:findAppFromUiCaller()
	end

	if app and app.minimized then
		app.uiWindow.w = app.winw
		app.uiWindow.h = app.winh
		if app.pop_min_h ~= nil  then
			app.uiWindow.pop_min_h = app.pop_min_h
		end

		if app.pop_max_h ~= nil  then
			app.uiWindow.pop_max_h = app.pop_max_h
		end

		if app.pop_min_w ~= nil  then
			app.uiWindow.pop_min_w = app.pop_min_w
		end
		app.minimized = false
	end
end




function WebBrowser:onClickRedo()
  -- caller is :header_opened:browse_redo
  local uiWindow = getUICaller().parent.parent
  local html = uiWindow:find("html")
  if html ~= nil then
    runAH(nil, "browse_redo", "name=" .. html.id)
  end
end

function WebBrowser:onClickUndo()
  -- caller is :header_opened:browse_undo
  local uiWindow = getUICaller().parent.parent

  local html = uiWindow:find("html")
  if html ~= nil then
    runAH(nil, "browse_undo", "name=" .. html.id)
  end
end

function WebBrowser:onClickRefresh()
  -- caller is :header_opened:browse_refresh
  local uiWindow = getUICaller().parent.parent

  local html = uiWindow:find("html")
  if html ~= nil then
    html:refresh()
  end
end

function WebBrowser:onClickHome()
  -- caller is :header_opened:browse_home
  local uiWindow = getUICaller().parent.parent

  local html = uiWindow:find("html")
  if html ~= nil then
    html:browse("home")
  end
end
