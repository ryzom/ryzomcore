
-- create the webig namespace without reseting if already created in an other file.
if (webig==nil) then
	webig= {}
end

if (webig.sheetLists==nil) then
	webig.sheetLists = {}
end

if (webig.urls==nil) then
	webig.urls = {}
end

function getUCtf8(text)
	local uctext = ucstring()
	if type(text) == "string" then
		uctext:fromUtf8(text)
		return uctext
	end
	return text
end

function game:openUrlOnWebig(url, noblink, close_if_open)
	local winframe = getUI("ui:interface:open_url_window")
	if close_if_open and  winframe ~= nil and winframe.active == true then
		winframe.active = false
	else
		WebBrowser:openWindow("open_url_window", url)
		winframe = getUI("ui:interface:open_url_window")
		winframe.opened=true
		winframe.active=true
		winframe.w = 830
		winframe.h = 600
		setTopWindow(winframe)
		if noblink == nil then
			winframe:blink(2)
		end
	end
end


function webig:addSheet(dst, sheet, quality, quantity, worned, user_color, rm_class_type, rm_faber_stat_type)
	if quality == nil then quality=0 end
	if quantity == nil then quantity=0 end
	if worned == nil then worned=0 end
	if user_color == nil then user_color=0 end
	if rm_class_type == nil then rm_class_type=0 end
	if rm_faber_stat_type == nil then rm_faber_stat_type=0 end
	addDbProp(dst..":SHEET", sheet)
	addDbProp(dst..":WORNED", worned)
	addDbProp(dst..":QUALITY", quality)
	addDbProp(dst..":QUANTITY", quantity)
	addDbProp(dst..":USER_COLOR", user_color)
	addDbProp(dst..":RM_CLASS_TYPE", rm_class_type)
	addDbProp(dst..":RM_FABER_STAT_TYPE", rm_faber_stat_type)
	addDbProp(dst..":CREATE_TIME", 0)
	addDbProp(dst..":SERIAL", 0)
end

function webig:cleanSheets(db)
	delDbProp(db)
end

function webig:addSheetList(name, ctrl, db, size)
	webig.sheetLists[name] = {}
	webig.sheetLists[name].ctrl = ctrl
	webig.sheetLists[name].db = db
	webig.sheetLists[name].selection = ""
	webig.sheetLists[name].size = size
end

function webig:copyItems(src, dst)
	addDbProp(dst..":SHEET", getDbProp(src..":SHEET"))
	addDbProp(dst..":WORNED", getDbProp(src..":WORNED"))
	addDbProp(dst..":QUALITY", getDbProp(src..":QUALITY"))
	addDbProp(dst..":QUANTITY", getDbProp(src..":QUANTITY"))
	addDbProp(dst..":USER_COLOR", getDbProp(src..":USER_COLOR"))
	addDbProp(dst..":RM_CLASS_TYPE", getDbProp(src..":RM_CLASS_TYPE"))
	addDbProp(dst..":RM_FABER_STAT_TYPE", getDbProp(src..":RM_FABER_STAT_TYPE"))
	addDbProp(dst..":CREATE_TIME", 0)
	addDbProp(dst..":SERIAL", 0)
end

function webig:swapItems(src, dst)
	local sheet = getDbProp(dst..":SHEET")
	local worned = getDbProp(dst..":WORNED")
	local quality = getDbProp(dst..":QUALITY")
	local quantity = getDbProp(dst..":QUANTITY")
	local user_color = getDbProp(dst..":USER_COLOR")
	local rm_class_type = getDbProp(dst..":RM_CLASS_TYPE")
	local rm_faber_stat_type = getDbProp(dst..":RM_FABER_STAT_TYPE")

	addDbProp(dst..":SHEET", getDbProp(src..":SHEET"))
	addDbProp(dst..":WORNED", getDbProp(src..":WORNED"))
	addDbProp(dst..":QUALITY", getDbProp(src..":QUALITY"))
	addDbProp(dst..":QUANTITY", getDbProp(src..":QUANTITY"))
	addDbProp(dst..":USER_COLOR", getDbProp(src..":USER_COLOR"))
	addDbProp(dst..":RM_CLASS_TYPE", getDbProp(src..":RM_CLASS_TYPE"))
	addDbProp(dst..":RM_FABER_STAT_TYPE", getDbProp(src..":RM_FABER_STAT_TYPE"))
	addDbProp(dst..":CREATE_TIME", 0)
	addDbProp(dst..":SERIAL", 0)

	addDbProp(src..":SHEET", sheet)
	addDbProp(src..":WORNED", worned)
	addDbProp(src..":QUALITY", quality)
	addDbProp(src..":QUANTITY", quantity)
	addDbProp(src..":USER_COLOR", user_color)
	addDbProp(src..":RM_CLASS_TYPE", rm_class_type)
	addDbProp(src..":RM_FABER_STAT_TYPE", rm_faber_stat_type)
	addDbProp(src..":CREATE_TIME", 0)
	addDbProp(src..":SERIAL", 0)
end

function webig:deleteItem(src)
	addDbProp(src..":SHEET", 0)
	addDbProp(src..":WORNED", 0)
	addDbProp(src..":QUALITY", 0)
	addDbProp(src..":QUANTITY", 0)
	addDbProp(src..":USER_COLOR", 0)
	addDbProp(src..":RM_CLASS_TYPE", 0)
	addDbProp(src..":RM_FABER_STAT_TYPE", 0)
	addDbProp(src..":CREATE_TIME", 0)
	addDbProp(src..":SERIAL", 0)
end

function webig:paramDbSheetSlot(sheet_list, ctrl)
	local ctrlSheet = webig.sheetLists[sheet_list].ctrl:find("list:"..ctrl)
	if ctrlSheet ~= nil then
		ctrlSheet.left_click="lua"
		ctrlSheet.left_click_params="webig:addOrRemoveDbSheet(\'"..sheet_list.."\', \'"..ctrl.."\')"
		ctrlSheet.dragable=true
		ctrlSheet.can_drop=true
		ctrlSheet.on_drop="lua"
		ctrlSheet.on_drop_params="webig:dropDbSheet(\'"..sheet_list.."\', \'"..ctrl.."\', \'%src\')"
		ctrlSheet.on_can_drop="lua"
		ctrlSheet.on_can_drop_params="webig:canDropDbSheet(\'"..sheet_list.."\', \'"..ctrl.."\', \'%src\')"
	end
end

function webig:paramDbSheetSelect(sheet_list, ctrl, lua_function)
	local ctrlSheet = webig.sheetLists[sheet_list].ctrl:find("list:"..ctrl)
	if ctrlSheet ~= nil then
		ctrlSheet.left_click="lua"
		ctrlSheet.left_click_params=lua_function.."(\'"..sheet_list.."\', \'"..ctrl.."\')"
		ctrlSheet.dragable=false
		ctrlSheet.can_drop=false
	end
end

function webig:canDropDbSheet(sheet_list, ctrl, src)
	webig.sheetLists[sheet_list].ctrl:find("list:"..ctrl).can_drop=true
end

function webig:dropDbSheet(sheet_list, ctrl, src)
 	local db = webig.sheetLists[sheet_list].db
 	local sl_id = webig.sheetLists[sheet_list].ctrl.id
 	if (string.sub(src, 1, string.len(sl_id)) == sl_id) then -- copy from same list sheet
		local pos=nil
		for i=1, string.len(src) do
			if string.sub(src, i, i) == ":" then
				pos = i+1
			end
		end
		id = string.sub(src, pos, string.len(src))
		webig:swapItems(db..":"..id, db..":"..ctrl)
 	else
		slot = getUI(src)
		if slot ~= nil then
			id = findReplaceAll(src, slot.parent.id..":", "")
			webig:copyItems("LOCAL:INVENTORY:BAG:"..id, db..":"..ctrl)
		end
	end
end


function webig:addOrRemoveDbSheet(sheet_list, ctrl)
 	local db = webig.sheetLists[sheet_list].db
	if getDbProp(db..":"..ctrl..":SHEET") == 0 then -- Add item
		webig:AddDbSheet(sheet_list, ctrl)
	else
		webig:removeDbSheetQuantity(sheet_list, ctrl)
	end
end

function webig:AddDbSheet(sheet_list, ctrl)
	runAH(nil, "enter_modal", "group=ui:interface:webig_html_modal")
	local whm = getUI("ui:interface:webig_html_modal")
	whm.child_resize_h=false
	whm.h = 44*webig.sheetLists[sheet_list].size
	whm.w = 224
	whm = getUI("ui:interface:webig_html_modal:html")
	if whm ~= nil then
		if string.sub(sheet_list, 1, 3) == "ark" then
			whm:browse(webig.ark_modal_urls[sheet_list])
		end
		whm:refresh() -- url need be setted before
	end
	webig.sheetLists[sheet_list].selection = ctrl
end

function webig:removeDbSheetQuantity(sheet_list, ctrl)
 	local db = webig.sheetLists[sheet_list].db
 	webig:copyItems(db..":"..ctrl, "UI:DROP_DESTROY_ITEM:ITEM")
	runAH(nil, "set_keyboard_focus", "select_all=true|target=ui:interface:webig_drop_destroy_item_quantity_modal:edit:eb")
	getUI("ui:interface:webig_drop_destroy_item_quantity_modal:ok_cancel:ok").onclick_l="lua"
	getUI("ui:interface:webig_drop_destroy_item_quantity_modal:ok_cancel:ok").params_l="webig:doRemoveDbSheetQuantity(\'"..sheet_list.."\', \'"..ctrl.."\')"
	getUI("ui:interface:webig_drop_destroy_item_quantity_modal:edit:eb").on_enter="lua"
	getUI("ui:interface:webig_drop_destroy_item_quantity_modal:edit:eb").on_enter_params="webig:doRemoveDbSheetQuantity(\'"..sheet_list.."\', \'"..ctrl.."\')"
	runAH(nil, "enter_modal", "group=ui:interface:webig_drop_destroy_item_quantity_modal")
	setDbProp("UI:DROP_DESTROY_ITEM:ITEM:QUANTITY", getDbProp(db..":"..ctrl..":QUANTITY"))
	getUI("ui:interface:webig_drop_destroy_item_quantity_modal:edit:eb").input_string=tostring(getDbProp(db..":"..ctrl..":QUANTITY"))
end

function webig:doRemoveDbSheetQuantity(sheet_list, ctrl)
	local db = webig.sheetLists[sheet_list].db
	runAH(nil, "leave_modal", "group=ui:interface:webig_drop_destroy_item_quantity_modal")
	local new_quantity = tonumber(getUI("ui:interface:webig_drop_destroy_item_quantity_modal:edit:eb").input_string)
	local current_quantity = getDbProp(db..":"..ctrl..":QUANTITY")
	if new_quantity >= current_quantity then
		webig:deleteItem(db..":"..ctrl)
	else
		addDbProp(db..":"..ctrl..":QUANTITY", new_quantity)
	end
end

function getUICallerRoot()
	return getUI(getUICaller().id:match("(ui:interface:[^:]*):?"))
end

function webig:openUrl(url, ui)
	if ui == nil then
		getUI("ui:interface:web_transactions"):find("html"):browse(url)
	else
		getUI("ui:interface:"..ui):find("html"):browse(url)
	end
end

function webig:openUrlInBg(url)
	if webig.urls[url] ~= nil then
		WebQueue:push(webig.urls[url])
	else
		WebQueue:push(url)
	end
end






-- VERSION --
RYZOM_WEBIG_VERSION = 10469