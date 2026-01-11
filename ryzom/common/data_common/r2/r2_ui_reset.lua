-- Reset ui & manage desktop change (one desktop per editor mode)



------------------------------------------------------------------------------------
function setActive(wndName, active)
	local wnd = getUI(wndName, r2.Config.VerboseGetUI)
	if wnd ~= nil then
		wnd.active = active
	end
end

------------------------------------------------------------------------------------
function hide(wndName)
	setActive(wndName, false)	
end	

------------------------------------------------------------------------------------
function show(wndName)
	setActive(wndName, true)		
end	


function r2:hide(wndName)
	hide(wndName)
end

-- Reset windows placement in each desktop (one desktop per mode of the editor, so no virtual desktop are available in r2ed)
function r2:resetDesktop(desktopIndex)	
	local scrW, scrH = getWindowSize()

	----------------------------------------------------------------------------------------------------------
	-- reset the EDITION desktop
	if desktopIndex == 0 then		
		hideAllWindows()								
		local palette = getUI("ui:interface:r2ed_palette")
		if palette then
			palette.active = true				
			palette.x = 4
			palette.y = scrH - 85
			palette.w = 225
			palette.h = 245
		end	
		local scenario = getUI("ui:interface:r2ed_scenario")
		if scenario then				
			scenario.active = false
			--scenario.w = 315
			scenario.w = 350
			scenario.h = 450
			scenario.x = scrW - scenario.w - 5
			scenario.y = scrH - 65
			-- by default the 'scene rollout' is opened
			--r2:setRolloutOpened(scenario:find("geo_tree").caption, scenario:find("geo_tree_list"), true)
			-- by default the 'permanent content' is opened
			--r2:setRolloutOpened(scenario:find("content_tree").caption, scenario:find("content_tree_list"), true)
			-- by default the 'act' rollout is opened
			--r2:setRolloutOpened(scenario:find("acts").caption, scenario:find("act_tree_enclosing"), true)			
		end	
		local webAdmin = getUI("ui:interface:r2ed_web_admin")
		webAdmin.active = false
		--if webAdmin then		
		--	webAdmin.active = false
		--	webAdmin.w = 250
		--	webAdmin.h = 255
		--	webAdmin.x = scrW - webAdmin.w - 5
        -- if scenario then
        -- webAdmin.y = scenario.y - scenario.h - 5
        -- else
        -- webAdmin.y = scrH -  5
        -- end
		-- end	

		local toolbar = getUI("ui:interface:r2ed_toolbar")
		if toolbar then
			toolbar.active = true
			toolbar:invalidateCoords()
			toolbar:updateCoords()			
		end
		
      
		  if r2.Scenario then
         r2:setSelectedInstanceId("") -- force a real select
			 r2:setSelectedInstanceId(r2.Scenario.InstanceId)
		  end
		-- <action handler="set" params="target='ui:interface:debug_info:active'|value=0"/>
		-- <action handler="set" params="target='ui:interface:debug_info:w'|value=350" />
		-- <action handler="set" params="target='ui:interface:debug_info:h'|value=300" />
		-- <action handler="set" params="target='ui:interface:debug_info:x'|value=10000" />
		-- <action handler="set" params="target='ui:interface:debug_info:y'|value=370" />							

	----------------------------------------------------------------------------------------------------------
	-- reset the TEST dektop		
	elseif desktopIndex == 1 then	
		hideAllWindows()
		-- show game related windows in test mode
		-- (same as virtual desktop 0 in game)

		-- Target
		runAH(nil, "set", "target='ui:interface:target:active'|value=1")
		runAH(nil, "set", "target='ui:interface:target:locked'|value=0")	
		runAH(nil, "set", "target='ui:interface:target:x'|value=sub(sub(getprop('ui:interface:w'),getprop('ui:interface:target:w')),4)")
		runAH(nil, "set", "target='ui:interface:target:y'|value=sub(getprop('ui:interface:h'),4)")
   
		-- gestion sets
		runAH(nil, "set", "target='ui:interface:gestionsets:active'|value=1")
		runAH(nil, "set", "target='ui:interface:gestionsets:x'|value=div(sub(getprop('ui:interface:w'),getprop('ui:interface:gestionsets:w')),2)")
		runAH(nil, "set", "target='ui:interface:gestionsets:y'|value=sub(getprop('ui:interface:h'),4)")
	
		-- player
		runAH(nil, "set", "target='ui:interface:player:active'|value=1")
		runAH(nil, "set", "target='ui:interface:player:x'|value=4")
		runAH(nil, "set", "target='ui:interface:player:y'|value=sub(getprop('ui:interface:h'),4)" )
		runAH(nil, "set", "target='ui:interface:player:locked'|value=0")
		runAH(nil, "set_transparent", "ui:interface:player")

		local webAdmin = getUI("ui:interface:r2ed_web_admin")
		webAdmin.active = false
		--if webAdmin then		
		--	webAdmin.active = true
		--	webAdmin.w = 250
		--	webAdmin.h = 255
		--	webAdmin.x = scrW - webAdmin.w - 5
		--	webAdmin.y = scrH - 75			
		--end	
		
		-- bonus malus window
		runAH(nil, "set", "target='ui:interface:bonus_malus:active'|value=1")
		runAH(nil, "set", "target='ui:interface:bonus_malus:x'|value=188")
		runAH(nil, "set", "target='ui:interface:bonus_malus:y'|value=sub(getprop('ui:interface:h'),4)")
		runAH(nil, "set", "target='ui:interface:bonus_malus:locked'|value=1")
		
		-- Compass
		--runAH(nil, "set", "target='ui:interface:compass:x'|value=sub(sub(getprop('ui:interface:w'),getprop('ui:interface:compass:w')),4)")
		--runAH(nil, "set", "target='ui:interface:compass:y'|value=sub(sub(getprop('ui:interface:h'),getprop('ui:interface:target:h')),8)")
		
		-- Help
		--runAH(nil, "set", "target='ui:interface:help_browser:active'|value=1")
		--runAH(nil, "set", "target='ui:interface:help_browser:locked'|value=0")
		--runAH(nil, "set", "target='ui:interface:help_browser:w'|value=410")
		--runAH(nil, "set", "target='ui:interface:help_browser:h'|value=128")
		--runAH(nil, "set", "target='ui:interface:help_browser:x'|value=sub(sub(getprop('ui:interface:w'),getprop('ui:interface:help_browser:w')),4)")
		--runAH(nil, "set", "target='ui:interface:help_browser:y'|value=add(getprop('ui:interface:help_browser:h'),4)")

		-- System Info
		--runAH(nil, "set", "target='ui:interface:system_info:w'|value=div(sub(getprop('ui:interface:w'),add(getprop('ui:interface:help_browser:w'),16)),2)")
		--runAH(nil, "set", "target='ui:interface:system_info:h'|value=128")
		--runAH(nil, "set", "target='ui:interface:system_info:x'|value=4")
		--runAH(nil, "set", "target='ui:interface:system_info:y'|value=add(getprop('ui:interface:system_info:h'),4)")
		
		-- Main Chat
		-- runAH(nil, "set", "target='ui:interface:main_chat:w'|value=getprop('ui:interface:system_info:w')")
		-- runAH(nil, "set", "target='ui:interface:main_chat:h'|value=128")
		-- runAH(nil, "set", "target='ui:interface:main_chat:x'|value=add(getprop('ui:interface:system_info:w'),8)")
		-- runAH(nil, "set", "target='ui:interface:main_chat:y'|value=add(getprop('ui:interface:system_info:h'),4)")

		
		-- Mission Journal
		--runAH(nil, "set", "target='ui:interface:info_player_journal:active'|value=1")
		--runAH(nil, "set", "target='ui:interface:info_player_journal:locked'|value=0")		
		--runAH(nil, "set", "target='ui:interface:info_player_journal:x'|value=4")
		--runAH(nil, "set", "target='ui:interface:info_player_journal:y'|value=add(add(getprop('ui:interface:system_info:h'),getprop('ui:interface:info_player_journal:h')),8)")


		-- Milko Pad
		--runAH(nil, "set", "target='ui:interface:milko:x'|value=sub(sub(getprop('ui:interface:w'),getprop('ui:interface:milko:w')),4)")
		--runAH(nil, "set", "target='ui:interface:milko:y'|value=sub(sub(getprop('ui:interface:compass:y'),getprop('ui:interface:compass:h')),4)")

      -- Hands
      
		local toolbarWindow = getUI("ui:interface:r2ed_toolbar_window")
		toolbarWindow.x=32
		toolbarWindow.y=0

		local milkoPad = getUI("ui:interface:milko_pad")
		milkoPad.y = 0
		milkoPad.x = scrW - milkoPad.w

	----------------------------------------------------------------------------------------------------------
	-- reset the DM destop		
	elseif desktopIndex == 2 then

		hideAllWindows()
		local webAdmin = getUI("ui:interface:r2ed_web_admin")
		webAdmin.active = false
		--if webAdmin then		
		--	webAdmin.active = true
		--	webAdmin.w = 250
		--	webAdmin.h = 255
		--	webAdmin.x = scrW - webAdmin.w - 5
		--	webAdmin.y = scrH - 5
		--end	
		local toolbar = getUI("ui:interface:r2ed_toolbar")
		if toolbar then
			toolbar.active = true
			toolbar:invalidateCoords()
			toolbar:updateCoords()			
		end

		local dmToolbar = getUI("ui:interface:r2ed_windows_dm_bar")
		if dmToolbar then
			dmToolbar.active = true
			dmToolbar:invalidateCoords()
			dmToolbar:updateCoords()			
		end

		hide("ui:interface:bonus_malus") -- patch for the bonus-malus window
	elseif desktopIndex == 3 then
		hideAllWindows()
	end
	r2:resetDesktopVisibleWindows(desktopIndex)
end

	
local function cleanEnv(contName)
	local cont = getUI("ui:interface" .. contName, r2.Config.VerboseGetUI)
	if cont then
		cont:deleteLUAEnvTable(true) -- recursively delete lua environments
		cont:setOnDraw("")
	end
end



----------------------------------------------------------------------------------------------------------
function r2:adjustToolbarGap()
	local toolbar = getUI("ui:interface:r2ed_toolbar")
	local dx = 0
	local numGroups = toolbar:getNumGroups()
	for k = 0, numGroups - 1 do
		local gr = toolbar:getGroup(k)
		if gr.active then
			gr.x = dx
			dx = 4
		else
			gr.x = 0
		end
	end
end

----------------------------------------------------------------------------------------------------------
-- common to r2:onChangeDesktop & r2:resetDesktop
function r2:resetDesktopVisibleWindows(desktopIndex)		
	-- reset the EDIT desktop
	if desktopIndex == 0 then				
		if (r2.CurrentPropertyWindow) then			
			r2.CurrentPropertyWindow.active = false
			r2.CurrentPropertyWindow = nil
		end
		r2.PropertyWindowVisible = false
		show("ui:interface:r2ed_tool_context_help")
		if config.R2EDExtendedDebug == 1 then
			show("ui:interface:r2ed_current_session")
		else
			hide("ui:interface:r2ed_current_session")
		end
		hide("ui:interface:welcome_info")
		hide("ui:interface:compass")
		show("ui:interface:r2ed_main_menu_button")
		show("ui:interface:r2ed_main_bl")		
		show("ui:interface:r2ed_select_bar")
		hide("ui:interface:r2ed_select_bar:buttons")
		hide("ui:interface:r2ed_select_bar:sequences")		
		hide("ui:interface:milko_pad")
		hide("ui:interface:windows")
		hide("ui:interface:r2ed_npc")
		hide("ui:interface:ring_chars_tracking")
		local b1 = getUI("ui:interface:r2ed_select_bar"):find("b1")
		if b1 then
			b1:updateCoords() -- force to update the coords, icon position not good otherwise
		end
		hide("ui:interface:bonus_malus") -- patch for the bonus-malus window
		r2.ToolUI:updateUndoRedo()		
		hide("ui:interface:r2ed_connect")
		hide("ui:interface:r2ed_toolbar_window")
		-- reset the TEST desktop
	elseif desktopIndex == 1 then		
		if config.R2EDExtendedDebug == 1 then
			show("ui:interface:r2ed_current_session")
		else
			hide("ui:interface:r2ed_current_session")
		end
		hide("ui:interface:compass")
		hide("ui:interface:welcome_info")
		show("ui:interface:r2ed_main_bl")
		hide("ui:interface:r2ed_testbar")		
		hide("ui:interface:r2ed_main_menu_button")		
		show("ui:interface:milko_pad")
		hide("ui:interface:windows")				
		hide("ui:interface:r2ed_connect")
		hide("ui:interface:ring_chars_tracking")
		if not r2.isSessionOwner() and r2.AccessMode ~= "Editor" then
			hide("ui:interface:r2ed_toolbar_window")
		else
			show("ui:interface:r2ed_toolbar_window")
		end

	-- reset the DM desktop		
	elseif desktopIndex == 2 then
		hide("ui:interface:compass")
		if config.R2EDExtendedDebug == 1 then
			show("ui:interface:r2ed_current_session")
		else
			hide("ui:interface:r2ed_current_session")
		end
		hide("ui:interface:r2ed_main_bl")
		show("ui:interface:r2ed_testbar")
		show("ui:interface:r2ed_main_menu_button")		
		hide("ui:interface:milko_pad")
		hide("ui:interface:windows")		
		hide("ui:interface:bonus_malus") -- patch for the bonus-malus window
		hide("ui:interface:r2ed_connect")
		hide("ui:interface:r2ed_toolbar_window")
		hide("ui:interface:ring_chars_tracking")
	else
		hide("ui:interface:bonus_malus") -- patch for the bonus-malus window
	end	

	hide("ui:interface:team_share") -- patch for the 'team share' window
	
	game:updateMissionJournalMode()
	r2:disableAnimals()
end

-- called by C ++ : reset non savable parts after a desktop change
function r2:onChangeDesktop(desktopIndex)
	--debugInfo("On change desktop = " .. tostring(desktopIndex))	
	hideAllNonSavableWindows();

	-- in each case, hide welcom and ring access windows
	getUI("ui:interface:npc_web_browser").active=false

	----------------------------------------------------------------------------------------------------------
	-- set the EDITION desktop
	if desktopIndex == 0 then		
		r2:setFixedLighting(false)
		hide("ui:interface:feature_help")
		r2.ui.AnimBar:clearActions()				
		local toolbar = getUI("ui:interface:r2ed_toolbar")
		if toolbar then
			toolbar.active = true
			toolbar.r2ed_tool_select.active = true
			toolbar.r2ed_tool_rotate.active = true
			toolbar.r2ed_tool_undo.active = true
			toolbar.r2ed_tool_redo.active = true
			toolbar.r2ed_tool_copy.active = true
			toolbar.r2ed_tool_paste.active = true
			toolbar.r2ed_tool_teleport.active = true
			toolbar.r2ed_tool_display_mode.active = true
			toolbar.r2ed_tool_start.active = true
			toolbar.r2ed_tool_stop.active = false
			toolbar.r2ed_stop_live.active = false
			--toolbar.r2ed_tool_teleport.x = 4
			r2:adjustToolbarGap()
			toolbar.r2ed_freeze_bot_objects.x = 4
			toolbar.r2ed_unfreeze_bot_objects.x = 0
			toolbar:invalidateCoords()
			toolbar:updateCoords()
			toolbar.r2ed_live.active = false

			
		end	
		local windowsDMBar = getUI("ui:interface:r2ed_windows_dm_bar")
		if windowsDMBar then
			windowsDMBar.active=false
			windowsDMBar:invalidateCoords()
			windowsDMBar:updateCoords()
		end			
		cleanEnv("r2ed_scenario")
		cleanEnv("r2ed_bbox_edit")
		cleanEnv("r2ed_toolbar")
		cleanEnv("r2ed_windowbar")
		cleanEnv("r2ed_testbar")
		cleanEnv("r2ed_toolbar_admin")
		cleanEnv("r2ed_table_test")
		cleanEnv("r2ed_editbox_test")
		cleanEnv("lua_inspector")
		cleanEnv("r2ed_palette")
		cleanEnv("r2ed_connect")
		cleanEnv("r2ed_property_sheet_no_selection")
		cleanEnv("r2ed_property_sheet_no_properties")
		cleanEnv("r2ed_scenario")
		r2.ScenarioWindow:resetWindow()
		r2.ContextualCommands:setupToolbar(nil)				

		-- set new title for th "keys" window
		local keys = getUI("ui:interface:keys")
		if keys then
			keys.uc_title = i18n.get("uiR2EDEditingKeys")
		end
	
		if r2.UserComponentsManager then
			r2.UserComponentsManager:updateUserComponentsUi()
		end
		r2:setupFreezeBotObjectButton()
		local goTestButton = getUI("ui:interface:r2ed_toolbar"):find("r2ed_tool_start").unselected.button
		local goTestMenu = getUI("ui:interface:r2ed_main_menu"):find("go_test")	
		goTestButton.frozen = false
		goTestMenu.grayed = false
	----------------------------------------------------------------------------------------------------------
	-- set the TEST desktop OR player desktop
	elseif desktopIndex == 1 then
		r2:setFixedLighting(false)
		hide("ui:interface:feature_help")
		r2.ui.AnimBar:clearActions()
		r2.ui.AnimBar:update()
		local toolbar = getUI("ui:interface:r2ed_toolbar")		
		toolbar.active = false
		local toolbarWindow = getUI("ui:interface:r2ed_toolbar_window")
		toolbarWindow:find("r2ed_anim_dm_mode").active = (r2.AccessMode == "Editor")
		toolbarWindow:find("r2ed_stop_live").active = not (r2.AccessMode == "Editor") and r2.isSessionOwner()
		toolbarWindow:find("player_control").active = (not (r2.AccessMode == "Editor")) and r2.isSessionOwner()
		toolbarWindow:find("r2ed_live").active = false
		
		-- resize the toolbarWindow depending on content
		r2:resizeToolbarWindow()
				
		local windowsDMBar = getUI("ui:interface:r2ed_windows_dm_bar")		
		if windowsDMBar then			
			windowsDMBar.r2ed_live.active = (not (r2.AccessMode == "Editor")) and r2.isSessionOwner() 
			windowsDMBar.player_control.active = (not (r2.AccessMode == "Editor")) and r2.isSessionOwner()			
			windowsDMBar:invalidateCoords()			
			windowsDMBar:updateCoords()			
		end	
		
		r2:adjustToolbarGap()

		local keys = getUI("ui:interface:keys")
		if keys then
			keys.uc_title = i18n.get("uiR2EDTestOrDMKeys")
		end

	----------------------------------------------------------------------------------------------------------
	-- reset the DM desktop		
	elseif desktopIndex == 2 then   
		r2:setFixedLighting(false)
		hide("ui:interface:feature_help")
		r2.ui.AnimBar:clearActions()     
		local toolbar = getUI("ui:interface:r2ed_toolbar")
		if toolbar then
			toolbar.active = true
			toolbar.r2ed_freeze_bot_objects.active = false
			toolbar.r2ed_unfreeze_bot_objects.active = false
			toolbar.r2ed_tool_select.active = false
			toolbar.r2ed_tool_rotate.active = false
			toolbar.r2ed_tool_undo.active = false
			toolbar.r2ed_tool_redo.active = false			
			toolbar.r2ed_tool_copy.active = false
			toolbar.r2ed_tool_paste.active = false
			toolbar.r2ed_tool_teleport.active = true
			toolbar.r2ed_tool_display_mode.active = false
			toolbar.r2ed_tool_start.active = false
			toolbar.r2ed_tool_stop.active = (r2.AccessMode == "Editor")			
			toolbar.r2ed_stop_live.active = (not (r2.AccessMode == "Editor")) and r2.isSessionOwner()
			--toolbar.r2ed_tool_teleport.x = -20 -- compensate x from previous buttons
			--toolbar.r2ed_stop_live.x = -8 -- compensate x from previous buttons
			toolbar.r2ed_live.active = (r2.AccessMode=="Editor" or r2.Mode=="DM") 
			r2:adjustToolbarGap()
			toolbar:invalidateCoords()
			toolbar:updateCoords()
		end

		
		local windowsDMBar = getUI("ui:interface:r2ed_windows_dm_bar")
		if windowsDMBar then
			windowsDMBar.active=true
			windowsDMBar.r2ed_live.active = (not (r2.AccessMode == "Editor")) and r2.isSessionOwner() 
			windowsDMBar.player_control.active = (not (r2.AccessMode == "Editor")) and r2.isSessionOwner()			

			windowsDMBar:invalidateCoords()
			windowsDMBar:updateCoords()
		end
		r2.ui.AnimBar:update()
		r2.ui.AnimBar:updateDMControlledEntitiesWindow()					
		local keys = getUI("ui:interface:keys")
		if keys then
			keys.uc_title = i18n.get("uiR2EDTestOrDMKeys")
		end
	end	

	-- special : if "key" window is visible on current desktop, hide, then show it again to force a refresh of its content
	local keys = getUI("ui:interface:keys")
	if keys.active then
		keys.active = false
		keys.active = true
	end

	r2:resetDesktopVisibleWindows(desktopIndex)
	r2.ToolUI:updateToggleWindowButtons()
	r2.ToolUI:updateToggleWindowDMButtons()
end

function r2:resizeToolbarWindow()
	local toolbarWindow = getUI("ui:interface:r2ed_toolbar_window")	
	toolbarWindow:updateCoords()
	local w = toolbarWindow:find("enclosing").w_real + 16		
	toolbarWindow.w = w
	toolbarWindow.pop_min_w = w
	toolbarWindow.pop_max_w = w
	toolbarWindow:updateCoords()
end

-- for masterless mode only : display option to control player & quit
function r2:initDMToolbarWindowPos()

	if not r2.isSessionOwner() then return end

	local scrW, scrH = getWindowSize()
	-- init dm toolbar default pos
	local toolbarWindow = getUI("ui:interface:r2ed_toolbar_window")	
	toolbarWindow.active = true
	toolbarWindow.x= 11
	toolbarWindow.y=scrH - 190
end
	
-- for masterless mode only : display option to control player & quit
function r2:popDMToolbarWindow()
	local toolbarWindow = getUI("ui:interface:r2ed_toolbar_window")	
	toolbarWindow.active = true	
	toolbarWindow:find("r2ed_anim_dm_mode").active = false
	toolbarWindow:find("r2ed_stop_live").active = true
	toolbarWindow:find("r2ed_live").active = true
	toolbarWindow:find("player_control").active = true
	r2:resizeToolbarWindow()
	setTopWindow(toolbarWindow)
	hide("ui:interface:r2ed_main_bl")
	game:updateMissionJournalMode()
	r2:disableAnimals()
	getUI("ui:interface:welcome_info").active=false
end


-- fix for previous versions : dm toolbar was shown on players desktop
function r2:playerModeUIFix()
	local toolbarWindow = getUI("ui:interface:r2ed_toolbar_window")	
	toolbarWindow.active = false	
	hide("ui:interface:r2ed_main_bl")
	game:updateMissionJournalMode()
	r2:disableAnimals()
	getUI("ui:interface:welcome_info").active=false
end


function r2:disableAnimals()
	local animals = getUI("ui:interface:animal_global")		
	animals.content.no_available_animals.active = true
	animals.list.active = false
	animals.header_opened.active = false
	animals.header_closed.active = true
end

















