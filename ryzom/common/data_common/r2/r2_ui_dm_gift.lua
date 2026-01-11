-- Handlers for dm gift interface 


r2.DMGift = 
{
	ChosenSlotIndex = 0, -- index of the slot on which the user clicked to choose a plot item	
}



local noItemModalUIPath = "ui:interface:r2ed_dm_gift_no_plot_items"	-- item browser with msg 'no available items'
local chooseItemModalUIPath = "ui:interface:r2ed_dm_gift_choose_plot_item" -- item browser
local chooseItemQuantityModalUIPath = "ui:interface:r2ed_choose_plot_item_quantity" -- quantity modal
local dmGiftUIPath = "ui:interface:r2ed_dm_gift" -- dm gift window

--------------------------------------------------------------------------------
-- The DM has clicked on a slot, prompt the interface to choose a plot item (is some are availables)
function r2.DMGift:onSlotClicked(index)
	-- if not plot item are avilable, prompt a special interface to tell how to add them
	if getDbProp("LOCAL:R2:PLOT_ITEMS:0:SHEET") == 0 then
		enableModalWindow(getUICaller(), noItemModalUIPath)
		return
	end
	enableModalWindow(getUICaller(), chooseItemModalUIPath)
	self.ChosenSlotIndex = index
end

--------------------------------------------------------------------------------
-- an item has been chosen in the plot item list
function r2.DMGift:setDMGiftItem(slot, sheetId, quantity)
	setDbProp("LOCAL:R2:DM_GIFT:" .. tostring(slot) .. 	":SHEET", sheetId)
	setDbProp("LOCAL:R2:DM_GIFT:" .. tostring(slot) .. 	":QUANTITY", quantity)
end


--------------------------------------------------------------------------------
-- an item has been chosen in the plot item list
function r2.DMGift:onItemChosen(sheetName)   
	if sheetName ~= "UI:EMPTY" then		
      local sheetId = getDbProp(sheetName..":SHEET")
		-- prompt the quantity dialog unless 'ctrl' is down, in which case
		-- a single item is added
		if not isCtrlKeyDown() then
			setDbProp("LOCAL:R2:CURR_PLOT_ITEM:SHEET", sheetId)		
			enableModalWindow(getUICaller(), chooseItemQuantityModalUIPath)
			local editBox = getUI(chooseItemQuantityModalUIPath):find("eb")
			editBox.input_string = "1"      
			setCaptureKeyboard(editBox)
			editBox:setSelectionAll()		
		else
			self:setDMGiftItem(self.ChosenSlotIndex, sheetId, 1)
		end
	else
		-- directly send message to clear the slot
		self:setDMGiftItem(self.ChosenSlotIndex, 0, 0)
	end
end

--------------------------------------------------------------------------------
-- an item has been chosen in the plot item list
function r2.DMGift:onValidateQuantity()
   disableModalWindow()
   local sheetId = getDbProp("LOCAL:R2:CURR_PLOT_ITEM:SHEET")
   local editBox = getUI(chooseItemQuantityModalUIPath):find("eb")		
   local quantity = tonumber(editBox.input_string)
   if quantity == 0 then sheetId = 0 end
   if quantity > 99 then quantity = 99 end
	self:setDMGiftItem(self.ChosenSlotIndex, sheetId, quantity)
end

--------------------------------------------------------------------------------
-- the server has required that the dm gift window be shown
function r2.DMGift:cancelModals()
	if getUI(noItemModalUIPath).active or
	   getUI(chooseItemModalUIPath).active or
	   getUI(chooseItemQuantityModalUIPath).active
	then
		disableModalWindow()
	end
end


--------------------------------------------------------------------------------
-- the server has required that the dm gift window be shown
function r2.DMGift:begin()
	local wnd = getUI(dmGiftUIPath)
	if wnd.active then
		-- just hide the window
		self:cancel()
		return
	end
	-- show the window
	self:cancelModals()
	wnd.active = true
	wnd:center()
	wnd:blink(1)
	-- clear all slots
	for k = 0, 7 do
		self:setDMGiftItem(k, 0, 0)
	end
	runAH(nil, "r2ed_dm_gift_begin", "")	
end

--------------------------------------------------------------------------------
-- the server has required that the dm gift window be stopped
function r2.DMGift:validate()
	-- hide the windows
	self:cancelModals()
	getUI(dmGiftUIPath).active = false
	-- send server msg
	runAH(nil, "r2ed_dm_gift_validate", "")
end

--------------------------------------------------------------------------------
-- force to cancel the current dm gift
function r2.DMGift:cancel()
	-- hide the window
	self:cancelModals()
	getUI(dmGiftUIPath).active = false
end








