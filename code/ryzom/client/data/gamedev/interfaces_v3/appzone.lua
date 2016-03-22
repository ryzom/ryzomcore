
-- global
AppZone = {
  id = "ui:interface:appzone",
  homeuri = "?action=appzone_toolbar",
  launchuri = "?action=launch_app",
  addappuri = "?action=list_user_apps",
  mode = "h_bar",
  imagesize = 26
}

-- use client.cfg value when available
local uri getClientCfg("AppZoneUrl")
if uri == nil or uri == '' then
  uri = 'http://app.ryzom.com/'
end

AppZone.homeuri = uri .. AppZone.homeuri
AppZone.launchuri = uri .. AppZone.launchuri
AppZone.addappuri = uri .. AppZone.addappuri

function AppZone:getRoot()
  return getUI(self.id)
end

function AppZone:onButtonHome()
  local webig = getUI("ui:interface:webig")
  webig:find("html").url = self.addappuri
  webig.active = true
end

function AppZone:calculateSize(count, spacer, mode)
  local w, h
  if mode == "h_bar" then
    -- icon=32, space=2+2
    w = count * self.imagesize + spacer * 15
    w = w + 55 + 10 + 20
    h = self.imagesize + 2
  elseif mode == "h_box" then
    w = count * self.imagesize
    w = w + 20
    h = self.imagesize + 2
    h = h * spacer
    h = h + 15
  elseif mode == "v_bar" then
    -- icon=32, space=2+2
    h = count * self.imagesize + spacer * 15
    h = h + 20 + 20 + 12
    w = self.imagesize + 2 + 18
  else
    h = count * self.imagesize
    h = h + 20 + 25
    w = self.imagesize + 2
    w = w * spacer
    w = w + 16
  end

  local ui = getUI("ui:interface")
  if w > ui.w then
    w = ui.w
  end
  if h > ui.h then
    h = ui.h
  end

  return w, h
end

function AppZone:setElementCount(count,spacer,m)
  self.mode = m

  local root = self:getRoot()
  local content = root:find("content")
  local html = root:find("html")

  local button_toggle = root:find("toggle_mode")
  local button_reload = root:find("browse_reload")
  local button_home = root:find("browse_home")

  local w, h = self:calculateSize(count, spacer, self.mode)
  root.h = h
  root.w = w
  content.w = w
  content.h = h

  -- set position of buttons
  if self.mode == "h_bar" then
    -- button_toggle.posref = "BL BL"
    button_toggle.x = 2
    button_toggle.y = 0
    -- button_reload.posref = "TL BL"
    html.x = 15
    html.y = 0
    button_reload.x = -25
    button_reload.y = -25
    -- button_home.posref = "BR BR"
    button_home.x = 0
    button_home.y = -3 + 5
  elseif self.mode == "h_box" then
    -- button_toggle.posref = "TL TL"
    button_toggle.x = 2
    button_toggle.y = h - 15
    -- button_reload.posref = "TL BL"
    html.x = 0
    html.y = -20
    button_reload.x = -25
    button_reload.y = -4 - 20
    --  button_home.posref = "BR BR"
    button_home.x = 0
    button_home.y = -3 + h - 18
  elseif self.mode == "v_bar" then
    -- button_toggle.posref = "TL TL"
    button_toggle.x = 2
    button_toggle.y = h - 15
    -- button_reload.posref = "TL BL"
    html.x = 0
    html.y = -20
    button_reload.x = 0
    button_reload.y = -4
    -- button_home.posref = "BR BR"
    button_home.x = 4 - 7
    button_home.y = -3
  else
    -- button_toggle.posref = "TL TL"
    button_toggle.x = 2
    button_toggle.y = h - 15
    -- button_reload.posref = "TL BL"
    html.x = 0
    html.y = -20
    button_reload.x = -25
    button_reload.y = -4 - 20
    --  button_home.posref = "BR BR"
    button_home.x = 0 - w + 54 + 12
    button_home.y = -3
  end
end

function AppZone:setMode(m)
  self.mode = m

  self:reload()
end

function AppZone:setActive(s)
  self:getRoot().active = s
end

function AppZone:launchApp(appid, appwin, appurl)
  if not appwin then
    if string.match(appid, "^[0-9]+$") then
      appwin = "app" .. tostring(appid)
    else
      appwin = "webig"
    end
  end

  if not appurl then
    appurl = self.launchuri .. "&appid=" .. tostring(appid)
  end

  if WebBrowser then
    WebBrowser:openWindow(appwin, appurl)
  else
    -- fallback if WebBrowser not present
    local webig = getUI("ui:interface:webig")
    webig:find("html").url = appurl;
    webig.active = true
  end
end

function AppZone:reload()
  local url = self.homeuri
  url = url .. "&mode=" .. tostring(self.mode)

  local html = self:getRoot():find("html")
  html.url = url
end

-- slash command: /appzone <cmd>
function AppZone:handle(cmd)
  if cmd == 'show' then
    self:setActive(true)
  elseif cmd == 'hide' then
    self:setActive(false)
  elseif cmd == 'reload' then
    self:reload()
  elseif cmd == 'list' then
    self:onButtonHome()
  else
    self:launchApp(cmd)
  end
end
