-- In this file we define functions that serves for inventory window
------------------------------------------------------------------------------------------------------------
-- create the game namespace without reseting if already created in an other file.
if (game==nil) then
	game= {};
end

function game:initEquipPreview()
	setChar3dDBfromServerDB("UI:TEMP:PREVIEWCHAR3D")

	local scene = getUI("ui:interface:inventory:content:equip:midsection:content:visu3d:char3d")
	local headz = scene:getElement("character#0").headz
	local pz = scene:getElement("character#0").posz
	local height = headz - pz
	local camera = scene:getElement("camera#0")
	camera.tgtz = pz + height*0.55
	camera.posz = camera.tgtz + 0.5
	camera.posy = 26 - 1.6*(height)
	scene.distlimitmin = 25 - camera.posy
	scene.distlimitmax = 27 - camera.posy

	scene = getUI("ui:interface:inv_equip:content:equip:midsection:content:visu3d:char3d")
	headz = scene:getElement("character#0").headz
	pz = scene:getElement("character#0").posz
	height = headz - pz
	camera = scene:getElement("camera#0")
	camera.tgtz = pz + height*0.55
	camera.posz = camera.tgtz + 0.5
	camera.posy = 26 - 1.6*(height)
	scene.distlimitmin = 25 - camera.posy
	scene.distlimitmax = 27 - camera.posy
end

function game:updateEquipOnResize(base, force)
	local path = "ui:interface:"..base
	local win = getUI(path)
	local equipPath = path..":content:equip"
	local equip = getUI(equipPath)
	if equip ~= nil and (equip.active ~= false or force) then
		local w = win.w
		local h = win.h
		local hotbar = getUI(equipPath..":hotbar_c")
		local hotbarTitle = getUI(equipPath..":hotbarTitle")
		local midsection = getUI(equipPath..":midsection")
		local jewelry = getUI(equipPath..":jewelry")
		local armors = getUI(equipPath..":armors")
		local handl = getUI(equipPath..":handl")
		local handr = getUI(equipPath..":handr")
		if h < 345 then
			hotbar.active = false
			hotbarTitle.active = false
		else
			hotbar.active = true
			hotbarTitle.active = true
		end
		if w < 450 then
			midsection.active = false
			jewelry.x = -10
			armors.x = 10
			jewelry.y = 0
			armors.y = 0
		else
			midsection.active = true
			jewelry.x = -20
			armors.x = 20
			jewelry.y = -5
			armors.y = -5
		end
		if w > 390 and h > 344 then
			handr.active = true
			handl.active = true
		else
			handr.active = false
			handl.active = false
		end
	end
end

function updateChest()
	local index = getUICaller().selection
	local chest = "A"
	local inv = ""
	if string.sub(getUICaller().id, 1, 23) == "ui:interface:inv_guild2" or string.sub(getUICaller().id, 1, 37) == "ui:interface:inventory:content:guild2" then
		chest = "B"
		inv = "2"
	end
	runCommand("a", "setGuildInventoryChest", chest, index)
	doUpdateChest(index, chest, inv)
	if string.sub(getUICaller().id, 1, 22) == "ui:interface:inv_guild" then
		ui = getUI("ui:interface:inventory:content:guild"..inv..":ibw:chest")
	else
		ui = getUI("ui:interface:inv_guild"..inv..":header_opened:ibw:chest")
	end
	ui.selection = index
end

function doUpdateChest(index, chest, inv)
	setDbProp("UI:SAVE:GUILD_INVENTORY_CHEST_"..chest, index)
	local ui = ""
	ui = getUI("ui:interface:inv_guild"..inv..":content:iil:bag_icons")
	runAH(ui, "list_sheet_change_start_item", "index="..tostring(index*getDefine("max_guild_invslot")))
	ui = getUI("ui:interface:inv_guild"..inv..":content:iil:bag_list")
	runAH(ui, "list_sheet_text_change_start_item", "index="..tostring(index*getDefine("max_guild_invslot")))
	ui = getUI("ui:interface:inventory:content:guild"..inv..":iil:bag_icons")
	runAH(ui, "list_sheet_change_start_item", "index="..tostring(index*getDefine("max_guild_invslot")))
	ui = getUI("ui:interface:inventory:content:guild"..inv..":iil:bag_list")
	runAH(ui, "list_sheet_text_change_start_item", "index="..tostring(index*getDefine("max_guild_invslot")))
end

function updateChestList(init)
	removeOnDbChange(getUI("ui:interface:inv_guild"),"@UI:VARIABLES:CURRENT_SERVER_TICK")
	local uis = {"ui:interface:inv_guild:header_opened:ibw", "ui:interface:inv_guild2:header_opened:ibw", "ui:interface:inventory:content:guild:ibw", "ui:interface:inventory:content:guild2:ibw" }
	local ui = ""
	for iid, id in pairs(uis) do
		ui = getUI(id..":chest")
		ui:resetTexts()
		local all_str_available = true
		for i=0, 19 do
			local name_id = getDbProp("SERVER:GUILD:CHEST:"..tostring(i)..":NAME")
			if name_id > 0 then
				if not isSrvStringAvailable(name_id) then
					all_str_available = false
				else
					ui:addText(ucstring(getSrvString(name_id):toUtf8().."    "))
				end
			end
		end
		if not all_str_available then
			addOnDbChange(getUI("ui:interface:inv_guild"), "@UI:VARIABLES:CURRENT_SERVER_TICK", "updateChestList()")
		elseif init == true then
			debug("RANGE ===== "..tostring(getUI(id..":bulk_weight:encombrement").range))
			ui.selection = (iid - 1) % 2;
			local max_guild_invslot = tostring(getDefine("max_guild_invslot"))
			getUI(id..":bulk_weight:encombrement").value = runExpr("getItemsBulk('"..getDefine("guild_inv_dbentry").."',mul("..max_guild_invslot..","..tostring(ui.selection).."),"..max_guild_invslot..")")
			getUI(id..":bulk_weight:encombrement").range = runExpr("getChestMaxBulk('"..getDefine("guild_chests_dbentry").."',"..tostring(ui.selection)..")")
			removeOnDbChange(getUI("ui:interface:inv_guild"), "@SERVER:GUILD:CHEST:0:BULK_MAX")
		end
	end
end

-- VERSION --
RYZOM_INVENTORY_VERSION = 324
