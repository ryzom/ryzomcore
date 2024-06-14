if arkNpcShop == nil then
	arkNpcShop = {}
end

function arkNpcShop:selectSheetItem(item, i, sheet)
	for name, value in pairs(arkNpcShop.lockedSlots[item]) do
		if value == item.."_"..i then
			arkNpcShop.lockedSlots[item][name] = nil
		end
	end

	local items = arkNpcShop.all_valid_items[item][i][sheet]
	arkNpcShop.player_money_per_items[item][i] = items[3]
	webig:addSheet("UI:TEMP:ARK:ITEM:"..tostring(item).."_"..tostring(i), items[1], items[2], getDbProp("UI:TEMP:ARK:ITEM:"..tostring(item).."_"..tostring(i)..":QUANTITY"), nil, items[4])
	setDbProp("UI:TEMP:ARK:ITEM:"..tostring(item).."_"..tostring(i)..":SERIAL", items[5])
	arkNpcShop.lockedSlots[item][items[5]] = item.."_"..i
	updateSelectedItems()
	runAH(nil, "leave_modal", "group=ui:interface:webig_html_modal")
end


function arkNpcShop:showBuy()
	getUI("ui:interface:ark_shop_buy_item"):find("ok").active=true
end

function arkNpcShop:openSection(url)
	framewin = getUI(arkNpcShop.window):find("buy"):renderHtml(arkNpcShop.PleaseWait)
	getUI("ui:interface:web_transactions"):find("html"):browse(url)
end

function arkNpcShop:updateWindow(px, py)
	local stop = false
	if px ~= 0 and py ~= 0 then
		local x, y, z = getPlayerPos()
		if (px-x)*(px-x)+(py-y)*(py-y) > 25 then
			stop = true
		end

		if arkNpcShop.target ~= getTargetSlot() then
			stop = true
		end
	end

	if stop then
		local w = getUI(arkNpcShop.window)
		w.active = false
		setOnDraw(w, "")
		getUI("ui:interface:ark_shop_buy_item").active = false
		arkNpcShop.player_can_buy = false
		broadcast(arkNpcShop.TooFar)
	end

	local diff = math.floor((nltime.getLocalTime() - arkNpcShop.lastMultipleItemsUpdate ) / 10)
	if diff >= 120 then
		arkNpcShop.lastMultipleItemsUpdate = nltime.getLocalTime()
		if arkNpcShop.player_money_per_items ~= nil then
			for item, price in pairs(arkNpcShop.player_money_per_items) do
				if arkNpcShop.lastMultipleItemsIndex[item] == nil then
					arkNpcShop.lastMultipleItemsIndex[item] = {}
				end
				for i = 1,5 do
					local w = getUI(arkNpcShop.window):find("ark_npc_shop_item_"..item.."_price"..tostring(i))
					if w ~= nil and arkNpcShop.all_items[item] ~= nil and arkNpcShop.all_items[item][i] ~= nil then
						if price[i] < 1 then
							if arkNpcShop.lastMultipleItemsIndex[item][i] == nil then
								arkNpcShop.lastMultipleItemsIndex[item][i] = 0
							end

							arkNpcShop.lastMultipleItemsIndex[item][i] = arkNpcShop.lastMultipleItemsIndex[item][i] + 1
							if arkNpcShop.lastMultipleItemsIndex[item][i] > tablelength(arkNpcShop.all_items[item][i]) then
								arkNpcShop.lastMultipleItemsIndex[item][i] = 1
							end

							local multiple = arkNpcShop.lastMultipleItemsIndex[item][i]
							if multiple ~= nil then
								local sheet = arkNpcShop.all_items[item][i][multiple]
								if sheet ~= "" then
									setDbProp("UI:TEMP:ARK:ITEM:"..item.."_"..tostring(i)..":SHEET", getSheetId(sheet..".sitem"))
								end
							end
						end
					end
				end
			end
		end
	end
end

function arkNpcShop:checkitems(db, items, quality, quality_max, id, must_be_equiped, item_i)
	if arkNpcShop.lockedSlots[id] == nil then
		arkNpcShop.lockedSlots[id] = {}
	end
	total = 0
	if arkNpcShop.all_valid_items[id] == nil then
		arkNpcShop.all_valid_items[id] = {}
	end

	arkNpcShop.all_valid_items[id][item_i] = {}

	local selected_item = 9999
	local last_selected = nil

	for i = 0, 499, 1 do
		local sheet =  getDbProp("SERVER:INVENTORY:BAG:"..tostring(i)..":SHEET")
		if sheet ~= 0 then

			local name = string.lower(getSheetName(sheet))
			for item_id, item in pairs(items) do
				if item == name or item..".sitem" == name then
					local qual = getDbProp("SERVER:INVENTORY:BAG:"..tostring(i)..":QUALITY")
					local quant = getDbProp("SERVER:INVENTORY:BAG:"..tostring(i)..":QUANTITY")
					local color = getDbProp("SERVER:INVENTORY:BAG:"..tostring(i)..":USER_COLOR")
					local locked = getDbProp("SERVER:INVENTORY:BAG:"..tostring(i)..":RESALE_FLAG") == 3

					if qual >= quality and qual <= quality_max and locked == false then
						valid = true

						if must_be_equiped ~= 0 then
							valid = false
							for j = 0, 19, 1 do
								local slot = getDbProp("SERVER:INVENTORY:EQUIP:"..tostring(j)..":INDEX_IN_BAG")
								if slot == i then
									valid = true
								end
							end
						end

						if valid then
							local use_quant = getDbProp("UI:TEMP:ARK:ITEM:"..tostring(id).."_"..tostring(item_i)..":QUANTITY")

							if quant >= use_quant then
								table.insert(arkNpcShop.all_valid_items[id][item_i], {sheet, qual, quant, color, i})
								local valid_item_i = #arkNpcShop.all_valid_items[id][item_i]
								local db_id = "UI:TEMP:ARK:POPUP_ITEM_"..tostring(id).."_"..tostring(item_i).."_"..tostring(valid_item_i)
								webig:deleteItem(db_id)
								webig:copyItems("SERVER:INVENTORY:BAG:"..tostring(i), db_id)
								setDbProp(db_id..":QUANTITY", use_quant)

								if qual < selected_item then
									total = quant
									selected_item = qual
									if last_selected ~= nil then
										arkNpcShop.lockedSlots[id][last_selected] = nil
									end
									last_selected = i
									arkNpcShop.lockedSlots[id][i] = tostring(id).."_"..tostring(item_i)
									webig:copyItems("SERVER:INVENTORY:BAG:"..tostring(i), db)
									setDbProp(db..":SERIAL", i)
									setDbProp(db..":QUANTITY", use_quant)
								end
							end
						end
					end
				end
			end
		end
	end

	return total
end

function arkNpcShop:checkslot(db, slotid)
	total = 0
	local index = getDbProp("SERVER:INVENTORY:EQUIP:"..tostring(slotid)..":INDEX_IN_BAG")-1
	if index ~= -1 then
		local sheet = getDbProp("SERVER:INVENTORY:BAG:"..tostring(index)..":SHEET")
		local qual = getDbProp("SERVER:INVENTORY:BAG:"..tostring(index)..":QUALITY")
		setDbProp(db..":SHEET", sheet)
		setDbProp(db..":QUALITY", qual)
		return 1
	end
	return 0
end


function arkNpcShop:OpenSheetInfosWindow(id)
	local w = getUI(arkNpcShop.window)
	local x = w:find("buy")
	if x == nil then
		x = w
	end
	x = x:find("ark_npc_shop_item_"..tostring(id))
	runAH(x:find("sheet"), "open_help_auto", "")

end

function arkNpcShop:HideHelpWindow(id)
	-- Check what help window are active
	local help_active={}
	for i = 0,7 do
		help_active[i] = getUI("ui:interface:sheet_help"..i).active
	end

	arkNpcShop:OpenSheetInfosWindow(id)

	-- Apply previous stats of help window
	for i = 0,7 do
		getUI("ui:interface:sheet_help"..i).active = help_active[i]
	end
end

function arkNpcShop:CheckMoney()
	local win = getUI("ui:interface:ark_shop_buy_item")
	local value = tonumber(win:find("edit"):find("eb").input_string)
	if value == nil or value == 0 then
		value = 1
	end

	if arkNpcShop.price == -1 then
		arkNpcShop.max_quantity = 1
	end

	if arkNpcShop.max_quantity ~= 0 and value > arkNpcShop.max_quantity then
		win:find("edit"):find("eb").input_string = arkNpcShop.max_quantity
		value = arkNpcShop.max_quantity
	end


	local total = 9999999999
	if arkNpcShop.price == -1 then
		total = getDbProp("UI:TEMP:ARK_MONEY_TOTAL")
	else
		total = math.floor(arkNpcShop.price*value)
	end

	if total > arkNpcShop.player_money then
		win:find("ok").hardtext="uiNotEnoughMoney"
		arkNpcShop.player_can_buy = false
	else
		win:find("ok").hardtext = arkNpcShop.ActionName
		arkNpcShop.player_can_buy = true
	end
	setDbProp("UI:TEMP:ARK_MONEY_TOTAL", total)
end

function arkNpcShop:Close()
	if arkNpcShop.window == "ui:interface:ark_npc_shop" then
		local framewin = getUI(arkNpcShop.window)
		if framewin ~= nil then
			framewin.active=false
		end
	else
		local framewin = getUI(arkNpcShop.window)
		framewin:renderHtml("please wait...")
	end

	framewin = getUI("ui:interface:ark_shop_buy_item")
	if framewin ~= nil then
		framewin.active=false
	end

end

function arkNpcShop:CloseShopBuyItem()
	framewin = getUI("ui:interface:ark_shop_buy_item")
	if framewin ~= nil then
		framewin.active=false
	end
end

function arkNpcShop:timer(id, len)
	local diff = math.floor((nltime.getLocalTime() - savedTime) / 50)
	getUI("ui:interface:current_action").active=true
	setDbProp("UI:PHRASE:ACT_BAR_LEN", math.floor(diff * (100/len)))
	if diff >= len then
		getUI("ui:interface:current_action").active=false
		setOnDraw(getUI("ui:interface:current_action"), "")
		local quantity = getUI("ui:interface:ark_shop_buy_item"):find("edit"):find("eb").input_string
		getUI("ui:interface:web_transactions"):find("html"):browse(arkNpcShop.ValidateUrl..quantity.."&item_id="..id.."&item_selection="..arkNpcShop.selectedItems[id])
	end
end


function arkNpcShop:Buy(id)
	local item = arkNpcShop.items[id]
	local win = getUI("ui:interface:ark_shop_buy_item")
	local quantity = win:find("edit"):find("eb").input_string
	if arkNpcShop.player_can_buy then
		local message  = ucstring()
		message:fromUtf8("@{F5FF}"..getUI("ui:interface:target").title..": @{FF0F}I\'m checking to see if you\'re trying to rip him off... ")
		-- displaySystemInfo(message, "BC")

		if arkNpcShop.AtysPoint then
			if item[8] == 0 then
				local message  = ucstring()
				message:fromUtf8(arkNpcShop.AtysPointsBuyMessage)
				displaySystemInfo(message, "BC")
				savedTime = nltime.getLocalTime()
				getUI("ui:interface:current_action").active=true
				local len = item[1]
				if len > 200 then
					len = 200
				end
				setOnDraw(getUI("ui:interface:current_action"), "arkNpcShop:timer("..id..", "..tostring(len)..")")
			else
				getUI("ui:interface:web_transactions"):find("html"):browse(arkNpcShop.ValidateUrl..quantity.."&item_id="..id.."&item_selection="..arkNpcShop.selectedItems[id])
			end
		else
			getUI("ui:interface:web_transactions"):find("html"):browse(arkNpcShop.ValidateUrl..quantity.."&item_id="..id.."&item_selection="..arkNpcShop.selectedItems[id])
		end
	end
	arkNpcShop:Close()
	if arkNpcShop.window ~= "ui:interface:ark_npc_shop" then
		getUI("ui:interface:encyclopedia").active=false
	end
end

function arkNpcShop:generateMp(mps)
	local final = {}
	local generateMps = {}
	for _, mp in pairs(mps) do
		if game.RawCreaturesMats ~= nil and string.sub(mp, 1, 1) == "!" then
			local params = {}
			for str in string.gmatch(string.sub(mp, 2), "([^!]+)") do
				table.insert(params, str)
			end
			local sheets = params[1]
			local qual = params[2]
			local eco = params[3]

			if qual == "" or qual == nil then
				qual = "cdef"
			end

			if eco == "" or eco == nil then
				eco = "dfljp"
			end

			for sheet in string.gmatch(sheets, "([^|]+)") do
				if string.sub(sheet, 1, 1) == "#" then
					local all_sheets = game.RawCreaturesMats[string.sub(sheet, 2)]
					if sheets ~= nil then
						for i_sheet, f_sheet in pairs(all_sheets) do
							if generateMps[i_sheet] == nil then
								generateMps[i_sheet] = {}
							end
							table.insert(generateMps[i_sheet], f_sheet)
						end
					end
				else
					table.insert(final, sheet..c_eco..c_qual.."01")
				end
			end

			for i = 1, #qual do
				local c_qual = string.sub(qual, i, i)
				for j = 1, #eco do
					local c_eco = string.sub(eco, j, j)
					for _, sheets in pairs(generateMps) do
						for _, f_sheet in pairs(sheets) do
							table.insert(final, f_sheet..c_eco..c_qual.."01")
						end
					end
				end
			end
		else
			table.insert(final, mp)
		end
	end

	return final
end


function arkNpcShop:getHtmlIcon(id, item)
	if string.sub(item[2], 1, 1) == "#" then
		addDbProp("UI:TEMP:ARK:SELECTITEM:RESALE_FLAG", 0)

		if string.sub(item[3], 1, 1) == "!" then
			webig:addSheet("UI:TEMP:ARK:SELECTITEM", getSheetId(item[7]), item[4], tonumber(string.sub(item[3], 2)))
		else
			webig:addSheet("UI:TEMP:ARK:SELECTITEM", getSheetId(item[7]), item[4], 1)
		end
		addDbProp("UI:TEMP:ARK:SELECTITEM:USER_COLOR", item[9])
		return [[<div class="ryzom-ui-grouptemplate" style="template:arkshop_inv_item;id:inv_special_bag_item;usesheet:true;isvirtual:false;sheetdb:UI:TEMP:ARK:SELECTITEM;w:40;params_r:arkNpcShop:OpenSheetInfosWindow(]]..id..[[);"></div>]]
	else
		return [[<div class="ryzom-ui-grouptemplate" style="template:arkshop_inv_item;id:inv_special_bag_item;usesheet:false;isvirtual:true;w:44;quantity: ;quality:]]..tostring(item[4])..[[;tooltip:]]..tostring(item[6])..[[;gc2:true;gc1:true;img1:]]..tostring(item[2])..[[;col_over:0 0 0 0"></div>]]
	end
end

function arkNpcShop:OpenItemWindow(id, buy)
	local item = arkNpcShop.items[id]
	if arkNpcShop.all_items[id] ~= nil and arkNpcShop.all_items[id].need_real_item ~= nil then
		if string.sub(item[3], 1, 1) == "!" then
			arkNpcShop.max_quantity = 1
		else
			arkNpcShop.max_quantity = item[3]
			for _, price in ipairs(arkNpcShop.player_money_per_items[id]) do
				if price < arkNpcShop.max_quantity then
					arkNpcShop.max_quantity = price
				end
			end
		end
	else
		if string.sub(item[3], 1, 1) == "!" then
			arkNpcShop.max_quantity = 1
		else
			arkNpcShop.max_quantity = item[3]
		end
	end

	arkNpcShop:HideHelpWindow(id)

	local non_buy_window_w = 400

	if string.sub(item[2], 1, 1) == "#" then
		local item_type = getSheetFamily(item[7])
		local display_preview = item_type == "SHIELD" or item_type == "ARMOR" or item_type == "MELEE_WEAPON" or item_type == "RANGE_WEAPON"
		if ui_item_preview then
			ui_item_preview.active = display_preview
		end

		if display_preview then
			non_buy_window_w = 495
		else
			-- Items who are not named items must display help window when no buy
			if buy == nil and item[2] == "#sheet" then
				arkNpcShop:OpenSheetInfosWindow(id)
				return
			end
		end
	end

	if buy == true then
		arkNpcShop.price = item[1]
		setDbProp("UI:TEMP:ARK_MONEY_PRICE", arkNpcShop.price)
		setDbProp("UI:TEMP:ARK_MONEY_TOTAL", arkNpcShop.price)
		if arkNpcShop.price > arkNpcShop.player_money then
			arkNpcShop.player_can_buy = false
		else
			arkNpcShop.player_can_buy = true
		end
	end

	framewin = getUI("ui:interface:ark_shop_buy_item")
	if framewin == nil then
		createRootGroupInstance("webig_bot_chat_buy_item", "ark_shop_buy_item", {id="content", infosclick="arkNpcShop:OpenSheetInfosWindow("..id..")",  onclick="arkNpcShop:Buy("..id..")"})
		framewin = getUI("ui:interface:ark_shop_buy_item")
		framewin.x = math.ceil(((getUI("ui:interface").w - framewin.w))/2)
		framewin.y = math.ceil(((getUI("ui:interface").h + framewin.h))/2)
	end

	local ui_item_show_desc = getUI("ui:interface:ark_shop_buy_item:content:header_opened:desc")
	if ui_item_show_desc then
		ui_item_show_desc.active = item[2] == "#sheet"
	end

	runAH(nil, "proc", "ark_shop_animate_preview_body")

	if buy == true then
		local eb = framewin:find("edit"):find("eb")
		eb.input_string = arkNpcShop.max_quantity
		eb:setFocusOnText()
		eb:setSelectionAll()
		framewin:find("item_total_price"):find("icone").texture = arkNpcShop.MoneyIcon
		framewin:find("item_price"):find("icone").texture = arkNpcShop.MoneyIcon
		framewin:find("item_total_price"):find("tt").tooltip = arkNpcShop.Money
		framewin:find("item_price"):find("tt").tooltip = arkNpcShop.Money
		arkNpcShop.window_params = {-70, 305, 408}
	else
		arkNpcShop.window_params = {-20, 380, non_buy_window_w}
	end

	framewin:find("top").active = buy == true -- active onlye if buy
	framewin:find("scroll_text").y=arkNpcShop.window_params[1]
	framewin:find("scroll_text").h=arkNpcShop.window_params[2]
	framewin.w=arkNpcShop.window_params[3]
	framewin.uc_title = getUCtf8(item[6])

	if buy == true then
		local html = ""
		html = arkNpcShop:getHtmlIcon(id, item)
		framewin:find("buy_sell_slot"):renderHtml(html)
		if string.sub(item[5], 1 , 8) == "https://" then
			framewin:find("ok").active=false
			framewin:find("infos"):renderHtml(arkNpcShop.PleaseWait)
			framewin:find("infos"):browse(item[5].."&rnd="..tostring(nltime.getLocalTime()))
		else
			framewin:find("ok").active=true
			framewin:find("infos"):renderHtml("<table width=\'380px\'><tr><td>"..item[5].."</td></tr></table>")
		end
		arkNpcShop:CheckMoney()
		-- framewin:setModalParentList(arkNpcShop.window)
	else
		html = arkNpcShop:getHtmlIcon(id, item)
		framewin:find("infos"):renderHtml([[
			<table width="100%">
				<tr>
					<td width="40px" valign="top">]]..html..[[</td>
					<td >]]..item[5]..[[</td>
				</tr>
			</table>
			]])
	end

	framewin:find("desc").params_l = "arkNpcShop:OpenSheetInfosWindow("..id..")"
	framewin:find("ok").params_l = "arkNpcShop:Buy("..id..")"
	framewin.opened=true
	framewin.active=true
end

function arkNpcShop:selectSheet(item, id)
	html = ""
	items = arkNpcShop.all_valid_items[item][id]
	for i=1,#items do
		if arkNpcShop.lockedSlots[item][items[i][5]] == nil then
			local params = "arkNpcShop:selectSheetItem("..tostring(item)..","..tostring(id)..","..tostring(i)..")"
			html = html..[[<div class="ryzom-ui-grouptemplate" id="ark_npc_shop_select_item_]]..item..[[" style="template:arkshop_inv_item;display:inline;id:inv_special_bag_item;usesheet:true;sheetdb:UI:TEMP:ARK:POPUP_ITEM_]]..item.."_"..id.."_"..i..[[;isvirtual:false;w:40;overlay2_active:1;gc2:true;gc1:true;col_over:0 0 0 0;params_l:]]..params..[[;params_r:]]..params..[["></div>&nbsp;]]
		end
	end

	runAH(nil, "enter_modal", "group=ui:interface:webig_html_modal")
	local whm = getUI("ui:interface:webig_html_modal")
	whm.child_resize_h = false
	whm.w = 240
	if h == nil then
		h = 120
	end
	whm.h = h
	whm_html = getUI("ui:interface:webig_html_modal:html")
	whm_html:renderHtml(html)
end

function arkNpcShop:updateTexts(id, ctrl, text1, text2, text3)
	local shop_item = getUI(arkNpcShop.window):find("ark_npc_shop_item_"..id)
	if shop_item then
		if ctrl then
			shop_item:find("ctrl").tooltip = getUCtf8(ctrl)
		else
			shop_item:find("ctrl").tooltip = ""
		end
		shop_item:find("text1").uc_hardtext_format = text1
		shop_item:find("text2").uc_hardtext_format = getUCtf8(text2)
		if text3 then
			shop_item:find("text3").uc_hardtext_format = getUCtf8(text3)
		else
			shop_item:find("text3").uc_hardtext_format = ""
		end
	end
end
