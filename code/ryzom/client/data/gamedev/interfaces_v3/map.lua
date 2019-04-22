--
-- custom maps
--

if (game==nil) then
	game= {};
end

-- alternative textures for maps
game.mapTextures = {}
-- game.mapTextures["zorai_map.tga"] = "tryker_map.tga"

-- register alternative texture for map
function game:setAltMap(mapName, altMap)
	self.mapTextures[mapName] = altMap
end

-- remove alternative map texture
function game:removeAltMap(mapName)
	self.mapTextures[mapName] = nil
end

-- map = getUI("ui:interface:map:content:map_content:actual_map")
function game:onLoadMap(map)
	-- debugInfo("onLoadMap(id=".. map.id ..", texture=".. map.texture ..")");

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


