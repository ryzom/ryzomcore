--
-- custom maps
--

if (game==nil) then
	game= {};
end

-- alternative textures for maps
game.mapTextures = {}
-- game.mapTextures["zorai_map.tga"] = "tryker_map.tga"

-- Dynamic points/icons for the map
game.mapArkPoints = {}

-- Hide sections for the map
game.mapHideSections = {}

-- Region sections for the map
game.mapRegionSections = {}

-- Region sections for the map
game.mapArkPointsCachedHelp = {}

-- Shapes spawned
game.spawnShapesByZone = {}

-- register alternative texture for map1
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
	if game.mapArkPoints[section] ~= nil then
		game.mapArkPoints[section][name] = nil
	end
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
	if whm.active == false  then
		runAH(nil, "enter_modal", "group=ui:interface:webig_html_modal")
		whm.child_resize_h = false
		whm.w = 480
		if h == nil then
			h = 240
		end
		whm.h = h
		local whm_html = getUI("ui:interface:webig_html_modal:html")
		if game.mapArkPointsCachedHelp[url] == nil then
			if whm_html ~= nil then
				game.mapMapArkPointHelpUrl = url
				whm_html:renderHtml("<body style:'background-color: #ffffff00'>...</body>")
				whm_html:browse(url)
			end
		else
			whm_html:renderHtml(game.mapArkPointsCachedHelp[url])
		end
		setOnDraw(getUI("ui:interface:webig_html_modal"), "game:updateMapArkPointHelp([["..url.."]])")
		game.mapArkPointHelpOpened = 1
	end
end

function game:updateMapArkPointHelp()
	local caller = getUI("ui:interface:webig_html_modal")
	local whm_html = getUI("ui:interface:webig_html_modal:html")
	if whm_html.html ~= "<body style:'background-color: #ffffff00'>...</body>" and game.mapArkPointsCachedHelp[game.mapMapArkPointHelpUrl] == nil then
		game.mapArkPointsCachedHelp[game.mapMapArkPointHelpUrl] = whm_html.html
	end

	local x, y = getMousePos()
	x0 = game.mapArkPointHelpMousePosX
	if caller.x ~= 0 or caller.y ~= 0 then
		if x < caller.x - 20 or x > caller.x + caller.w + 20 or y < caller.y - caller.h - 20 or y > caller.y + 20 then
			setOnDraw(getUI("ui:interface:webig_html_modal"), "")
			runAH(nil, "leave_modal", "group=ui:interface:webig_html_modal")
		end
	end
end

function game:onLoadMap(map)
	if map then
		game.currentMap = map
		if map.continent then
			game.currentMapContinent = map.continent
		end
		if map.texture then
			game.currentMapTexture = map.texture
		end
	end


	local texture = game.currentMap.texture
	if not texture then
		texture = game.currentMapTexture
	end


	delArkPoints()
	game.mapMapArkPointHelpUrl = ""
	for section, points in pairs(game.mapArkPoints) do
		real_section = {}
		map_section = section
		for k, v in string.gmatch(section, "(%w+)/(%w+)") do
			real_section = {k, v}
		end

		if real_section[1] then
			section = real_section[1]
			map_section = real_section[2]
		end

		if game.mapHideSections[section] == nil then
			if game.mapRegionSections[map_section] == nil or game.mapRegionSections[map_section][texture] == true then
				for name, point in pairs(points) do
					addLandMark(point[1], point[2], point[3], point[4], "lua", point[5], "", "", "lua", point[6])
				end
			end
		end
	end

	-- if alt view not enabled
	if getDbProp("UI:VARIABLES:SHOW_ALT_MAP") == 0 or map:isIsland() then
		return
	end


	if self.mapTextures[texture] ~= nil then
		-- debugInfo("-- using ".. self.mapTextures[texture] .." for " .. texture)
		return self.mapTextures[texture]
	end
end

function game:openFullMap()
	local ui = getUI("ui:interface:map")
	if ui.active == false then
		ui.active = true
	end

	if game.saveMapFull then
		game.saveMapFull = false
		ui.x = game.saveMapX
		ui.y = game.saveMapY
		ui.w = game.saveMapW
		ui.h = game.saveMapH

		game.savedMapFullZoom = getActualMapZoom()
		if game.savedMapZoom then
			setActualMapZoom(game.savedMapZoom)
		end
	else
		game.saveMapFull = true
		game.saveMapX = ui.x
		game.saveMapY = ui.y
		game.saveMapW = ui.w
		game.saveMapH = ui.h
		ui.x = 0
		ui.y = 0
		ui.w = getUI("ui:interface").w
		ui.h = getUI("ui:interface").h
		game.savedMapZoom = getActualMapZoom()
		if game.savedMapFullZoom then
			setActualMapZoom(game.savedMapFullZoom)
		end
		setTopWindow(ui)
	end
end

function game:addSpawnShapesByZone(zone, continent, name, displayIcon, setup, finish, openShape, text, icon)
	local id1 = -1
	local id2 = -1

	if game.spawnShapesByZone[continent] == nil then
		game.spawnShapesByZone[continent] = {}
	end

	if game.spawnShapesByZone[continent][name] then
		id1 = game.spawnShapesByZone[continent][name][9]
		id2 = game.spawnShapesByZone[continent][name][10]
	end

	table.insert(setup, id1)
	table.insert(setup, id2)
	table.insert(setup, finish)
	table.insert(setup, openShape)
	game.spawnShapesByZone[continent][name] = setup
	game.spawnShapesByZone[continent][name][8] = Json.decode(setup[8])

	if not text then
		text =  i18n.get("uiWisdomChest"):toUtf8()
	end

	if not icon then
		icon = "ico_box"
	end

	if displayIcon == 1 then
		game:addMapArkPoint(zone, setup[2], setup[3], setup[1], text, icon..".tga")
	else
		game:delMapArkPoint(zone, setup[1])
	end
end

function game:doSpawnShapesByZone(continent)
	if game.spawnShapesByZone[continent] then
		for name, shape in pairs(game.spawnShapesByZone[continent]) do

			if shape[9] ~= nil and shape[9] > 0 then
				deleteShape(shape[9])
			end

			if shape[10] ~= nil  and shape[9] > 0then
				deleteShape(shape[10])
			end

			local setup = shape[8]
			game.spawnShapesByZone[continent][name][9] = SceneEditor:doSpawnShape(shape[1]..".shape", setup, shape[2], shape[3], shape[4], shape[5], shape[6], shape[7], "user", 1, false, setup["action"], setup["url"], false, false, setup["textures"], "", false)
			if shape[11] == 0 then
				game.spawnShapesByZone[continent][name][10] = SceneEditor:doSpawnShape("ge_mission_evenement.ps", setup, shape[2], shape[3], shape[4]+0.35, shape[5], shape[6], shape[7], "user", 1, false, setup["action"], setup["url"], false, false, setup["textures"], "", false)
			else
				game.spawnShapesByZone[continent][name][10] = nil
			end
		end
	end
end

game.mapRegionSections["Silan"] = {}
game.mapRegionSections["Silan"]["newbieland_city.tga"] = true
game.mapRegionSections["Zorai"] = {}
game.mapRegionSections["Zorai"]["zorai_map.tga"] = true

game:addMapArkPoint("Vip/Silan", 10276, -11791, "vip_silan_tryker", "", "dynicon_vip.tga", "https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=9894&vip=nb_tryker_leader&title=fct_chief_explorer&gender=1", 150)
game:addMapArkPoint("Vip/Silan", 10341, -11822, "vip_silan_matis",  "", "dynicon_vip.tga", "https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=9894&vip=nb_matis_leader&title=fct_matis_master_artisan&gender=1", 150)
game:addMapArkPoint("Vip/Silan", 10382, -11741, "vip_silan_zorai",  "", "dynicon_vip.tga", "https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=9894&vip=nb_zorai_leader&title=fct_sage&gender=1", 150)
game:addMapArkPoint("Vip/Silan", 10366, -11692, "vip_silan_fyros",  "", "dynicon_vip.tga", "https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=9894&vip=nb_fyros_leader&title=fct_fyros_commander&gender=1", 150)
game:addMapArkPoint("Vip/Silan", 10304, -11719, "vip_silan_ranger", "", "dynicon_vipbox.tga", "https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=9894&vip=chiang_the_strong&title=fct_ranger_leader&gender=1", 150)

game:addMapArkPoint("Vip", 4154, -3305, "vip_allegory", "", "allegory_16.tga", "https://app.ryzom.com/app_arcc/index.php?action=mScript_Run&script=9894&vip=allegory_vip&title=fct_allegory_maker&gender=1", 150)


-- register map overrride
-- game:setAltMap("fyros_map.tga", "fyros_map_sp.tga")

-- VERSION --
RYZOM_MAP_VERSION = 324
