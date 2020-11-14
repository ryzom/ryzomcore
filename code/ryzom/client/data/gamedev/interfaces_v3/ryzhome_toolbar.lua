RyzhomeBar = {
	id = "ui:interface:webig_ryzhome_toolbar",
	saveuri = "http://app.ryzom.com/app_ryzhome/index.php?action=toolbar_save",
	selectedPage = 1
}

function RyzhomeBar:close()
	getUI("ui:interface:webig_ryzhome_toolbar").active=false
	self:saveConfig()

end

function RyzhomeBar:addItems()
	local url = "http://app.ryzom.com/app_arcc/index.php?action=player_ryzhome_LuaListItems&command=add"
	getUI("ui:interface:web_transactions:content:html"):browse(url)
end

function RyzhomeBar:moveItems()
	local url = "http://app.ryzom.com/app_arcc/index.php?action=player_ryzhome_LuaListItems&command=move"
	getUI("ui:interface:web_transactions:content:html"):browse(url)
end

function RyzhomeBar:removeItems()
	local url = "http://app.ryzom.com/app_arcc/index.php?action=player_ryzhome_LuaListItems&command=remove"
	getUI("ui:interface:web_transactions:content:html"):browse(url)
end

function RyzhomeBar:inviteFriend()
	local url = "http://app.ryzom.com/app_arcc/index.php?action=player_ryzhome_InviteFriend"
	getUI("ui:interface:web_transactions:content:html"):browse(url)
end

function RyzhomeBar:listFriends()
	getUI("ui:interface:web_transactions:content:html"):browse(RyzhomeBar.listFriendsUrl)
	getUI("ui:interface:web_transactions:header_opened:browse_undo").active=false
	getUI("ui:interface:web_transactions:header_opened:browse_redo").active=false
	getUI("ui:interface:web_transactions:header_opened:browse_refresh").active=false
	getUI("ui:interface:web_transactions:header_opened:browse_home").active=false
	local wt = getUI("ui:interface:web_transactions")
	wt.w=316
	wt.h=420
	wt.pop_min_w=316
	wt.pop_max_w=316
	wt.pop_min_h=420
	wt.pop_max_h=420

	local framewin = getUI("ui:interface:webig_ryzhome_list_item")
	if framewin ~= nil then
		framewin.active=false
		wt.x = framewin.x
		wt.y = framewin.y
	end

	getUI("ui:interface:web_transactions").active=true

	setOnDraw(getUI("ui:interface:web_transactions"), "RyzhomeBar:autocloseWebTransactions()")
end

function RyzhomeBar:autocloseWebTransactions()
	local current_url = getUI("ui:interface:web_transactions:content:html").url
	if (current_url ~= RyzhomeBar.listFriendsUrl and current_url ~= inviteFriendsUrl) then
		local framewin = getUI("ui:interface:webig_ryzhome_list_item")
		framewin.x = getUI("ui:interface:web_transactions").x
		framewin.y = getUI("ui:interface:web_transactions").y
		getUI("ui:interface:web_transactions").active=false
		setOnDraw(getUI("ui:interface:web_transactions"), "")
	end
end


function RyzhomeBar:serialize()
	local ui = getUI(self.id)
	local url = "&posx=" .. tostring(ui.x) .. "&posy=" .. tostring(ui.y)
	return url
end

function RyzhomeBar:updateNbrItems(offset)
	RyzhomeBar.nbrItems = RyzhomeBar.nbrItems + offset
	if RyzhomeBar.nbrItems == 0 then
		getUI("ui:interface:webig_ryzhome_toolbar:content:new_items_quantity").hardtext=""
	else
		getUI("ui:interface:webig_ryzhome_toolbar:content:new_items_quantity").hardtext=tostring(RyzhomeBar.nbrItems)
	end
end

function RyzhomeBar:saveConfig()
	local url = self.saveuri .. self:serialize()
	getUI("ui:interface:web_transactions:content:html"):browse(url)
end

function RyzhomeBar:movePage(offset)
	RyzhomeBar.selectedPage = RyzhomeBar.selectedPage + offset
	if RyzhomeBar.selectedPage <= 0 then
		RyzhomeBar.selectedPage = 1
	elseif RyzhomeBar.selectedPage > RyzhomeBar.nbrPages then
		RyzhomeBar.selectedPage = RyzhomeBar.nbrPages
	end
	getUI("ui:interface:webig_ryzhome_list_item:header_opened:page").hardtext=tostring(RyzhomeBar.selectedPage).." / "..tostring(RyzhomeBar.nbrPages)
	RyzhomeBar:setupItems()
end

function RyzhomeBar:listItems()
	RyzhomeBar.recently_removed_item = false

	local framewin = getUI("ui:interface:webig_ryzhome_list_item")
	--framewin.opened=true
	framewin.active=true
	if framewin.x == 0 and framewin.y == 0 then
		local ui = getUI("ui:interface")
		framewin.x = (ui.w - framewin.w) / 2
		framewin.y = (ui.h + framewin.h) / 2
	end 

	if RyzhomeBar.Items == nil then
		RyzhomeBar.Items = {}
	end
end

function RyzhomeBar:useItem(id)
	id = tostring(RyzhomeBar.Items[RyzhomeBar.selectedPage][id][1])
	if RyzhomeBar.itemCommand == "add" then
		RyzhomeBar:addItem(id)
	elseif RyzhomeBar.itemCommand == "remove" then
		RyzhomeBar:removeItem(id)
	elseif RyzhomeBar.itemCommand == "move" then
		RyzhomeBar:moveItem(id)
	end
end

function RyzhomeBar:addItem(id)
	local url = "http://app.ryzom.com/app_arcc/index.php?action=player_ryzhome_Place&command=add&id="..id
	getUI("ui:interface:web_transactions:content:html"):browse(url)
end

function RyzhomeBar:removeItem(id)
	RyzhomeBar.recently_removed_item = true
	RyzhomeBar:spawnItems()
	local v = RyzhomeBar.spawnedItems[id]
	runAH(nil,"add_shape", "shape=sp_mort.ps|x="..v[2].."|y="..v[3].."|z="..v[4].."|angle="..v[5].."|scale="..tostring(tonumber(v[6])*4)..v[7]..v[8]..v[9])
	local url = "http://app.ryzom.com/app_arcc/index.php?action=player_ryzhome_Remove&id="..id
	getUI("ui:interface:web_transactions:content:html"):browse(url)
end

function RyzhomeBar:moveItem(id)
	local url = "http://app.ryzom.com/app_arcc/index.php?action=player_ryzhome_Place&command=move&id="..id
	getUI("ui:interface:web_transactions:content:html"):browse(url)
end

function RyzhomeBar:highlightItem(id)
	if RyzhomeBar.itemCommand == "add" then
		return
	end
	if RyzhomeBar.recently_removed_item then
		RyzhomeBar.recently_removed_item = false
	else
		RyzhomeBar:spawnItems()
		local v = RyzhomeBar.spawnedItems[tostring(RyzhomeBar.Items[RyzhomeBar.selectedPage][id][1])]
		if v then
			runAH(nil, "add_shape", "shape=ma_acc_ascenseur.ps|x="..v[2].."|y="..v[3].."|z="..v[4].."|angle="..v[5].."|scale="..tostring(tonumber(v[6])*2)..v[7]..v[8]..v[9])
		end
	end
end

function RyzhomeBar:callFriendUrl(action, target)
	local url = "http://app.ryzom.com/app_arcc/index.php?action=player_ryzhome_"..action.."&amp;target="..target
	getUI("ui:interface:web_transactions:content:html"):browse(url)
end

function RyzhomeBar:spawnItems()
	runAH(nil, "remove_shapes", "")
	for k,v in pairs(RyzhomeBar.spawnedItems) do
		runAH(nil, "add_shape", "shape="..v[1].."|x="..v[2].."|y="..v[3].."|z="..v[4].."|angle="..v[5].."|scale="..v[6]..v[7]..v[8]..v[9])
	end

end

function RyzhomeBar:setupItems()
	for k = 1, 8 do 
		getUI("ui:interface:webig_ryzhome_list_item:header_opened"):find(":but"..tostring(k)).active=false
		getUI("ui:interface:webig_ryzhome_list_item:header_opened"):find(":icon"..tostring(k)).active=false
		getUI("ui:interface:webig_ryzhome_list_item:header_opened"):find(":text"..tostring(k)).uc_hardtext=""
		getUI("ui:interface:webig_ryzhome_list_item:header_opened"):find(":info"..tostring(k)).uc_hardtext=""
	end
	for k,v in pairs(RyzhomeBar.Items[RyzhomeBar.selectedPage]) do
		if k ~= nil then
			getUI("ui:interface:webig_ryzhome_list_item:header_opened"):find(":icon"..tostring(k)).active=true
			getUI("ui:interface:webig_ryzhome_list_item:header_opened"):find(":but"..tostring(k)).active=true
			local text = ucstring()
			text:fromUtf8(v[3])
			getUI("ui:interface:webig_ryzhome_list_item:header_opened"):find(":text"..tostring(k)).uc_hardtext=text
			text:fromUtf8(v[4])
			getUI("ui:interface:webig_ryzhome_list_item:header_opened"):find(":info"..tostring(k)).uc_hardtext=text
		end
	end
end

if RyzhomePlace == nil then
	RyzhomePlace = {
		saveuri = "",
	}
end

function RyzhomePlace:move(x, y, z)
	pos_x=pos_x+x
	pos_y=pos_y+y
	pos_z=pos_z+z
	RyzhomePlace:update()
end

function RyzhomePlace:rot(a)
	pos_a=pos_a+a
	RyzhomePlace:update()
end

function RyzhomePlace:reset()
	--Ryzhome:addShapes()
	pos_x, pos_y, pos_z = getPlayerPos()
	pos_a = (3.14*getUI("ui:interface:compass:arrow3d:arrow").rotz)/18
	RyzhomePlace:addShape()
	RyzhomeBar:spawnItems()
end

function RyzhomePlace:update()
	RyzhomePlace:addShapes()
	RyzhomePlace:addShape()
	RyzhomeBar:spawnItems()
end

function RyzhomePlace:apply()
	getUI("ui:interface:web_transactions:content:html"):browse(RyzhomePlace.saveuri.."&pos_x="..pos_x.."&pos_y="..pos_y.."&pos_z="..pos_z.."&pos_a="..pos_a)
end

function RyzhomePlace:close()
	--runAH(nil, "remove_shapes", "")
	getUI("ui:interface:webig_ryzhome_place_item").active=false
end

function debug(text)
	local uc = ucstring()
	uc:fromUtf8(tostring(text))
	displaySystemInfo(ucstring(uc), "sys")
end
