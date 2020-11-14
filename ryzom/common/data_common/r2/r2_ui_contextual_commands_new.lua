----------------------------
-- CONTEXTUAL COMMANDS UI --
----------------------------
-- Code in this file manage the update of the contextual toolbar & 
-- contextual menu for the current selected instance, depending on the vailable options

r2.ContextualCommands = r2.newToolbar()


r2.ContextualCommands.MenuCommands={}
r2.ContextualCommands.CurrentMenuInstance = nil -- for now, will always be equal to CurrentInstance, or nil if menu hasn't been displayed yet
r2.ContextualCommands.CurrentMenu = nil



------------
-- PUBLIC --
------------


local newCommands = {}


----------------------------------------------------------------------------
-- Update the toolbar content, avoiding to resetup it if there where no changes
function r2.ContextualCommands:update()
	-- TODO nico : report this behavior in the base class
	if self.CurrentInstance then
		table.clear(newCommands)
		if not self.CurrentInstance.Ghost then
			self.CurrentInstance:getAvailableCommands(newCommands)
		end
		if isEqual(newCommands, self.CurrentCommands) then
			return -- commands remains unchanged, no need to update the ui
		end
	end
	-- if one of the command is highlighted, let it highlighted after the toolbar has been rebuilt
	local highlightedCommand
	for index, button in pairs(self.IndexToButton) do
		if button == r2.ToolUI:getActiveToolUI() then
			debugInfo("highlighted command found")
			highlightedCommand = self.CurrentCommands[index]
			break
		end
	end
	self:setupToolbar(self.CurrentInstance)
	-- if one command was highlighted, highlight it again in new toolbar
	if highlightedCommand then
		for index, command in pairs(self.CurrentCommands) do
			if command == highlightedCommand then
				debugInfo("re-highlighting command")
				r2.ToolUI:setActiveToolUI(self.IndexToButton[index])
				break
			end
		end
	end
	-- for now, if menu is displayed then toolbar must be, too
	if self.CurrentMenuInstance then
		-- update menu
		if self.CurrentMenu.active then
			self:setupMenu(self.CurrentMenuInstance, self.CurrentMenu)
		end
	end
	-- remove current context help -> 
	disableContextHelp()
end


----------------------------------------------------------------------------
-- Special function called by the framework when the "r2ed_context_command" action is triggered
-- Like other action (defined in actions.xml), this action can have been triggered by a key assigned to it in the 'keys' window
function r2:execContextCommand(commandId)
	if not r2.ContextualCommands.CurrentInstance then return end
	-- see in list of current commands if there is a command with the good id
	for key, command in pairs(r2.ContextualCommands.CurrentCommands) do
		if command.Id == commandId then
			command.DoCommand(r2.ContextualCommands.CurrentInstance)	
			return
		end
	end
end

----------------------------------------------------------------------------
-- called by the ui when it wants to display the tooltip for one of the contextual commands
function r2:updateContextualCommandTooltip(index)
	local command = r2.ContextualCommands.ToolbarCommands[index + 1]
	assert(command)
	local keyName = ucstring(runExpr(string.format("getKey('r2ed_context_command', 'commandId=%s')", command.Id)))
	if keyName == i18n.get("uiNotAssigned") then
		-- no associated key
		setContextHelpText(i18n.get(command.TextId))
	else
		setContextHelpText(concatUCString(i18n.get(command.TextId), " (", keyName, ")"))
	end
end


local commands = {} -- to avoid reallocs

----------------------------------------------------------------------------
-- Setup a contextual menu for the given instance
-- passing nil just hides the toolbar
function r2.ContextualCommands:setupMenu(instance, menu)
	-- TMP (test of menu exported methods)
	--if menu then
	--	local rm = menu:getRootMenu()
	--	debugInfo("****************************")
	--	debugInfo("Num line " .. tostring(rm:getNumLine()))
	--	for i =  0, rm:getNumLine() -1 do
	--		debugInfo("Id for line " .. tostring(i) .. " is " .. tostring(rm:getLineId(i)))
	--	end
	--	debugInfo("Line with id 'dynamic_content_start' has index " .. tostring(rm:getLineFromId('dynamic_content_start')))
	--	debugInfo("Line 6 is separator = " .. tostring(rm:isSeparator(6)))
	--	rm:addLine(ucstring("toto"), "lua", "debugInfo('pouet')", "toto")
	--	rm:addSeparator()
	--	rm:addLine(ucstring("tutu"), "lua", "debugInfo('pouet')", "tutu")
	--	rm:addLine(ucstring("titi"), "lua", "debugInfo('pouet')", "titi")
	--	local titiIndex = rm:getLineFromId('titi')
	--	rm:addSeparatorAtIndex(titiIndex)
	--	rm:addLine(ucstring("bidon"), "lua", "debugInfo('pouet')", "titi")
	--	debugInfo("################################")
	-- end	
	
	
	self.CurrentMenuInstance = instance	
	self.CurrentMenu = menu	
	table.clear(self.MenuCommands)
	if not instance then return end
	-- delete entries for dynamic content
	local menuRoot = menu:getRootMenu()
	local startLine = menuRoot:getLineFromId("dynamic_content_start")
	local endLine = menuRoot:getLineFromId("dynamic_content_end")
	assert(startLine ~= -1 and endLine ~= -1)
	for lineToDel = endLine - 1, startLine + 1, -1 do
		menuRoot:removeLine(lineToDel)		
	end
	-- retrieve dynamic commands
	table.clear(commands)
	if not instance.Ghost then
		instance:getAvailableCommands(commands)
	end
	local currentLine = startLine + 1
   local currentActivityLine = 0
	local commandIndex = 1
	local activityAdded = false   
	local activityMenuIndex = menuRoot:getLineFromId("activities")
   local activityMenu
   if activityMenuIndex ~= -1 then
        activityMenu = menuRoot:getSubMenu(activityMenuIndex)
   end
   if activityMenu then
      activityMenu:reset()         
   end   
	for commandIndex, command in pairs(commands) do
		if command.ShowInMenu then
			local destNode
			local line         
			if command.IsActivity then
				destNode = activityMenu
				line = currentActivityLine
				activityAdded = true
			else
				line = currentLine            
				destNode = menuRoot
			end
			destNode:addLineAtIndex(line, i18n.get(command.TextId), "lua", 
                                                    "r2.ContextualCommands:runMenuCommand(" .. tostring(table.getn(self.MenuCommands) + 1) .. ")", "dyn_command_" .. tostring(commandIndex))
			-- if there's a bitmap, build a group with the buitmap in it, and add to menu
			if command.ButtonBitmap and command.ButtonBitmap ~= "" then
				local smallIcon   = nlfile.getFilenameWithoutExtension(command.ButtonBitmap) .. 
	                   "_small." ..  nlfile.getExtension(command.ButtonBitmap)
				local menuButton = createGroupInstance("r2_menu_button", "", { bitmap = smallIcon, })
				if menuButton then
					destNode:setUserGroupLeft(line, menuButton)
					assert(destNode:getUserGroupLeft(line) == menuButton)
					-- TMP (test for menu exported lua methods)
					-- menuButton = createGroupInstance("r2_menu_button", "", { bitmap = smallIcon, })
					-- menuRoot:setUserGroupRight(currentLine, menuButton)
					-- assert(menuRoot:getUserGroupRight(currentLine) == menuButton)
				end				
			end
         local keyNameGroup = createGroupInstance("r2_keyname", "", { id = command.Id })
         if keyNameGroup then
            local keyName = ucstring(runExpr(string.format("getKey('r2ed_context_command', 'commandId=%s')", command.Id)))
            if keyName == i18n.get("uiNotAssigned") then
               -- no associated key
               keyNameGroup:find("t").uc_hardtext = keyName
            else
               keyNameGroup:find("t").uc_hardtext = concatUCString(ucstring("(") , keyName, ucstring(")"))
            end
            destNode:setUserGroupRight(line, keyNameGroup)
         end
			table.insert(self.MenuCommands, command)
         if command.IsActivity then
            currentActivityLine = currentActivityLine + 1
         else
            currentLine = currentLine + 1
         end
		end		
	end	
end

----------------------------------------------------------------------------
function r2.ContextualCommands:runMenuCommand(index)
	assert(self.CurrentMenuInstance)
	assert(self.MenuCommands[index] ~= nil)	
	-- do actual call
	self.MenuCommands[index].DoCommand(self.CurrentInstance)
end


----------------------------------------------------------------------------
-- Hightlight the button of the last triggered command
function r2.ContextualCommands:highlightCurrentCommandButton()	
	if self.LastTriggeredCommandIndex then
		r2.ToolUI:setActiveToolUI(self:getButton(self.LastTriggeredCommandIndex - 1))
	end
end

----------------------------------------------------------------------------
-- Hightlight the button of the last triggered command
function r2.ContextualCommands:highlightCommandButton(commandId)	
	r2.ToolUI:setActiveToolUI(self.IndexToButton[self.CommandIdToIndex[commandId]])	
end




-------------------------------------------------------------------
-- PRIVATE :IMPLEMENT METHODS REQUIRED BY r2_ui_toolbar_base.lua --
-------------------------------------------------------------------

-- return a reference on the toolbar
function r2.ContextualCommands:getToolbar()
	return getUI("ui:interface:r2ed_contextual_toolbar_new")
end

-- return the max number of command that the toolbar may contains
function r2.ContextualCommands:getMaxNumCommands()
	return tonumber(getDefine("r2ed_max_num_contextual_buttons"))
end

-- get a button from its index in the toolbar
function r2.ContextualCommands:getButton(buttonIndex)
	if self.ButtonList == nil then
		self.ButtonList = self:getToolbar():find("buttons")
	end
	return self.ButtonList["b" .. tostring(buttonIndex)]
end

-- setup a button from a command in the toolbar
function r2.ContextualCommands:setupButton(button, commandDesc, buttonIndex)
	assert(button)   
	button.active = true
	-- buld filenames for the 'normal', 'over' & 'pushed" textures
	local icon = commandDesc.ButtonBitmap
	local iconOver   = nlfile.getFilenameWithoutExtension(icon) .. 
	                   "_over." ..  nlfile.getExtension(icon)
	local iconPushed = nlfile.getFilenameWithoutExtension(icon) .. 
	                   "_pushed." ..  nlfile.getExtension(icon)

	local selectedGroup    = button:find("selected")
	local unselectedGroup  = button:find("unselected")
	local selectedButton   = selectedGroup:find("button")
	local unselectedButton = unselectedGroup:find("button")

	selectedGroup.active   = false
	unselectedGroup.active = true

	

	selectedButton.texture		      = icon
	selectedButton.texture_over       = iconOver
	selectedButton.texture_pushed     = iconPushed	
	selectedButton.parent.Env.Toolbar = self

	unselectedButton.texture		  = icon
	unselectedButton.texture_over     = iconOver
	unselectedButton.texture_pushed   = iconPushed	
	unselectedButton.parent.Env.Toolbar = self
	
	
	selectedButton.onclick_l = "lua"
	selectedButton.params_l  = "r2.runBaseToolbarCommand(getUICaller().parent.Env.Toolbar, " .. tostring(buttonIndex) .. ")"
	unselectedButton.onclick_l = "lua"
	unselectedButton.params_l  = selectedButton.params_l
	
	button.child_resize_wmargin = select(commandDesc.Separator == true, 8, 0)		
   
end

-- retrieve a command list for the given instance
function r2.ContextualCommands:getAvailableCommands(instance, dest)
	--inspect(instance)
	table.clear(dest)
	if not instance.Ghost then
		instance:getAvailableCommands(dest)
	end		
end








