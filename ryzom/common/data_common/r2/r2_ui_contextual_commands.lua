----------------------------
-- CONTEXTUAL COMMANDS UI --
----------------------------
-- Code in this file manage the update of the contextual toolbar & 
-- contextual menu for the current selected instance, depending on the vailable options

r2.ContextualCommands = 
{
	--ButtonCache =  -- list of buttons already created in the toolbar
				   -- each entry has the form :
				   -- { Id="id_of_button_in_the_ui", CommandDesc={ },   }
				   -- 'CommandDesc' is one of the command description returned
				   -- by the selected instance when calling 'getAvailableCommands'				   
				   -- }
	--{

	--}
	
	-- description of each command
	CurrentCommands={},
	-- map index of each command to the matching toolbar button
	IndexToButton = {},
	-- Current instance for which this contextual toolbar is displayed
	CurrentInstance = nil,
	-- Index of last triggered command
	LastTriggeredCommandIndex = nil,
	-- Cache of buttons (to avoid costrly template instanciation). Key is the command, Value if a pointer to the button
	ButtonCache = {}
}

------------
-- PUBLIC --
------------

----------------------------------------------------------------------------
-- Setup the contextual toolbar for the given instance
-- passing nil just hides the toolbar
function r2.ContextualCommands:setupToolbar(instance)
	self.CurrentInstance = instance
	table.clear(self.CurrentCommands)
	table.clear(self.IndexToButton)
	table.clear(self.LastTriggeredCommandIndex)
	local toolbar = self:getToolbar()
	local buttonList = toolbar:find("buttons")
	if not instance then
		toolbar.active = false
		-- put all buttons in the cache for futur reuse
		--for index, button in IndexToButton do
			-- detach button from its parent
			--if buttonlist.detachChild(button) then
			--	ButtonCache[CurrentCommands[index]] = button

			--end
		--end
		return
	end
	table.clear(self.CurrentCommands)
	if not instance.Ghost then
		instance:getAvailableCommands(self.CurrentCommands)
	end
	assert(self.CurrentCommands)
	toolbar.active = true	
	buttonList:clear()
	for index, commandDesc in pairs(self.CurrentCommands) do
		self:addToolbarCommand(buttonList, commandDesc, index)		
	end
end


local newCommands = {}

----------------------------------------------------------------------------
-- Update the toolbar content
function r2.ContextualCommands:update()
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
	--local highlightedCommand
	--for index, button in pairs(self.IndexToButton) do
	--	if button == r2.ToolUI:getActiveToolUI() then
	--		debugInfo("highlighted command found")
	--		highlightedCommand = self.CurrentCommands[index]
	--		break
	--	end
	--end
	self:setupToolbar(self.CurrentInstance)
	-- if one command was highlighted, highlight it again in new toolbar

end

----------------------------------------------------------------------------
-- Hightlight the button of the last triggered command
function r2.ContextualCommands:highlightCurrentCommandButton()
	debugInfo("***")	
	if self.LastTriggeredCommandIndex then
		r2.ToolUI:setActiveToolUI(self.IndexToButton[self.LastTriggeredCommandIndex])
	end
end

-------------
-- PRIVATE --
-------------

----------------------------------------------------------------------------
-- Private : Get reference to the contextual toolBar
function r2.ContextualCommands:getToolbar(instance)
	return getUI("ui:interface:r2ed_contextual_toolbar")
end

----------------------------------------------------------------------------
-- Private : add a new button in the toolbar
function r2.ContextualCommands:addToolbarCommand(buttonList, commandDesc, index)
	if not commandDesc.ButtonBitmap or commandDesc.ButtonBitmap == "" then
		return -- no button wanted for this command
	end
	-- buld filenames for the 'normal', 'over' & 'pushed" textures
	local icon = commandDesc.ButtonBitmap
	local iconOver   = nlfile.getFilenameWithoutExtension(icon) .. 
	                   "_over." ..  nlfile.getExtension(icon)
	local iconPushed = nlfile.getFilenameWithoutExtension(icon) .. 
	                   "_pushed." ..  nlfile.getExtension(icon)
	local tmplParams = 
	{
		tooltip     = commandDesc.TextId,
		id          = tostring(index),
		icon        = icon,
		icon_over   = iconOver,
		icon_pushed = iconPushed,
		onclick_l   = "lua",
		params_l    = "r2.ContextualCommands:runCommand(" .. tostring(index) .. ")",
		w = "32",
		h = "32",
		offset_x = select(commandDesc.Separator == true, 8, 0)		
	}

	local newButton = createGroupInstance("r2ed_tool", buttonList.id, tmplParams)
	if newButton then
		buttonList:addChild(newButton)		
		self.IndexToButton[index] = newButton
	end
end

----------------------------------------------------------------------------
-- Private : execute a command triggered by a toolbar button
function r2.ContextualCommands:runCommand(index)
	assert(self.CurrentInstance)
	assert(self.CurrentCommands[index] ~= nil)
	self.LastTriggeredCommandIndex = index
	-- do actual call
	self.CurrentCommands[index].DoCommand(self.CurrentInstance)	
end




