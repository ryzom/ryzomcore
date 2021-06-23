--
-- custom maps
--

if (game==nil) then
	game= {};
end

-- alternative textures for maps
game.mapTextures = {}
-- game.mapTextures["zorai_map.tga"] = "tryker_map.tga"

-- Dynamic points for the map
game.mapArkPoints = {}

-- register alternative texture for map
function game:setAltMap(mapName, altMap)
	self.mapTextures[mapName] = altMap
end

-- remove alternative map texture
function game:removeAltMap(mapName)
	self.mapTextures[mapName] = nil
end

function game:addMapArkPoint(section, x, y, name, title, texture, url, h)
	if game.mapArkPoints[section] == nil then
		game.mapArkPoints[section] = {}
	end
	if url == nil then
		game.mapArkPoints[section][name] = {x, y, title, texture, "", ""}
	else
		if h ~= nil and h > 0 then
			game.mapArkPoints[section][name] = {x, y, title, texture, "", "game:openMapArkPointHelp([["..url.."]], "..tostring(h)..")"}
		else
			game.mapArkPoints[section][name] = {x, y, title, texture, "game:openUrlOnWebig([["..url.."]])", "game:updateMapArkPointColor()"}
		end
	end
end

function game:delMapArkPoint(section, name)
	game.mapArkPoints[section][name] = nil
end

function game:delMapArkSection(section)
	game.mapArkPoints[section] = nil
end

function game:updateMapArkPointColor()
	local button = getUICaller()
	button.texture_over = "lm_respawn_over.tga"
end

function game:openMapArkPointHelp(url, h)
	local whm = getUI("ui:interface:webig_html_modal")
	local x, y = getMousePos()
	whm.x = x
	whm.y = y
	debug(whm)
	if whm.active == false  then
		runAH(nil, "enter_modal", "group=ui:interface:webig_html_modal")
		whm.child_resize_h = false
		whm.w = 480
		if h == nil then
			h = 240
		end
		whm.h = h
		if game.mapMapArkPointHelpUrl ~= url then
			whm_html = getUI("ui:interface:webig_html_modal:html")
			whm_html:renderHtml("<body style:'background-color: #ffffff00'>...</body>")
			if whm_html ~= nil then
				whm_html:browse(url)
				game.mapMapArkPointHelpUrl = url
			end
		end
		setOnDraw(getUI("ui:interface:webig_html_modal"), "game:updateMapArkPointHelp()")
		game.mapArkPointHelpOpened = 1
	end
end

function game:updateMapArkPointHelp()
	local caller = getUI("ui:interface:webig_html_modal")
	local x, y = getMousePos()
	x0 = game.mapArkPointHelpMousePosX
	if caller.x ~= 0 or caller.y ~= 0 then
		if x < caller.x - 20 or x > caller.x + caller.w + 20 or y < caller.y - caller.h - 20 or y > caller.y + 20 then
			setOnDraw(getUI("ui:interface:webig_html_modal"), "")
			runAH(nil, "leave_modal", "group=ui:interface:webig_html_modal")
		end
	end
end

-- map = getUI("ui:interface:map:content:map_content:actual_map")
function game:onLoadMap(map)
	-- debugInfo("onLoadMap(id=".. map.id ..", texture=".. map.texture ..")");

	delArkPoints()
	game.mapMapArkPointHelpUrl = ""
	for section, points in pairs(game.mapArkPoints) do
		for name, point in pairs(points) do
			addLandMark(point[1], point[2], point[3], point[4], "lua", point[5], "", "", "lua", point[6])
		end
	end

	-- if alt view not enabled
	if getDbProp("UI:VARIABLES:SHOW_ALT_MAP") == 0 or map:isIsland() then
		return
	end

	local texture = map.texture
	if self.mapTextures[texture] ~= nil then
		-- debugInfo("-- using ".. self.mapTextures[texture] .." for " .. texture)
		return self.mapTextures[texture]
	end

end


-- register map overrride
-- game:setAltMap("fyros_map.tga", "fyros_map_sp.tga")

-- TEST
-- game:addMapArkPoint("vip", 4154,-3305, "vip_allegory", "", "allegory_16.tga", "https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=9894&vip=allegory_vip&title=fct_allegory_maker&gender=1", 140)
