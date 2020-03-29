-- management of visible editor windows

---------------
-- FUNCTIONS --
---------------

-------------------------------------------------------------------------------------
-- refresh the button that allow to toggle a window on / off
function r2:refreshWindowButtons()
	-- debugInfo("refreshWindowButtons")
end


-------------------------------------------------------------------------------------
function r2:switchWindow(name)
	local wnd = getUI(name)
	wnd.active = not wnd.active
end

-------------------------------------------------------------------------------------
-- misc windows activation deactivation
function r2:switchPaletteWindow()	
	self:switchWindow("ui:interface:r2ed_palette")	
end

function r2:switchScenarioWindow()	
	self:switchWindow("ui:interface:r2ed_scenario")	
end

function r2:switchDebugWindow()
	self:switchWindow("ui:interface:debug_info")
end

function r2:switchChatWindow()
	self:switchWindow("ui:interface:main_chat")
end

function r2:switchWebAdminWindow()
	self:switchWindow("ui:interface:r2ed_ring_window")
end

function r2:switchMailBoxWindow()
	self:switchWindow("ui:interface:mailbox")
end

function r2:switchGuildForumWindow()
	self:switchWindow("ui:interface:guild_forum")
end

function r2:switchMapWindow()
	self:switchWindow("ui:interface:map")	
end

function r2:switchPlayerAdminWindow()
	self:switchWindow("ui:interface:ring_chars_tracking")	
end

function r2:switchCustomBBoxWindow()	
	self:switchWindow("ui:interface:r2ed_bbox_edit")
	if r2.CustomBBox.FirstDisplay == true then
		if r2.CustomBBox:getWindow().active then
			r2.CustomBBox:getWindow():center()
		end
		r2.CustomBBox.FirstDisplay = false
	end
	if r2.CustomBBox:getWindow().active then
		runCommand("showR2EntityBoxes")
	else
		runCommand("hideR2EntityBoxes")
	end
end


function r2:isPropertyWindowVisible()
	if r2.PropertyWindowVisible == true then
		if r2.CurrentPropertyWindow ~= nil then
			return true
		end
	end
	return false
end

function r2:switchPropertiesWindow()
	if r2:isPropertyWindowVisible() then
		if r2.CurrentPropertyWindow ~= nil then
			r2.CurrentPropertyWindow.active = false
		end
		r2.PropertyWindowVisible = false
	else
		r2:showProperties(r2:getSelectedInstance())
	end
end

------------------
-- STATIC DATAS --
------------------

r2.PropertyWindowVisible = false -- not stored in window because there are several "properties" windows (one per class)
r2.CurrentPropertyWindow = nil   -- current property window being displayed


-----------------------------------------
-- INIT OF WINDOW POSITIONS AT STARTUP --
-----------------------------------------


-- init default pos for a property window
function r2:initDefaultPropertyWindowPosition(wnd)
	if wnd == nil then 		
		return 
	end
	scrW, scrH = getWindowSize()
	wnd.x = 4
	wnd.y = scrH - 312
	wnd.w = 225
	wnd.h = 300
end

