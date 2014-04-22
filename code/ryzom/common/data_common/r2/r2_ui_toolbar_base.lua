----------------------------
-- GENERIC TOOLBAR SYSTEM --
----------------------------

-- Generic code for toolbar
--
-- A toolbar is a set of 'buttons' that triggers 'commands'
-- A 'command' is a structure returned by a function like baseClass.getAvailableCommand (see there for doc)
-- A 'button' is a widget thet may have any form you want, derivers should just tell how to setup a button from a command
--
-- Usage :
-- 
-- -- 1°) creation
-- myToolBar = r2.newToolbar()
--
-- -- 2°) definition of toolbar behavior. Function that should be given by the deriver are given in section below
-- function myTooBar:getToolbar()
-- ...
-- end
-- function myToolbar ...
--
-- NB : command index are starting at 0 in the ui, and at 1 in lua


--
--
--
local toolbarTemplate = {}


------------------------------------
-- TO BE IMPLEMENTED BY DERIVERS  --
------------------------------------

-- return a reference on the toolbar ui
function toolbarTemplate:getToolbar()
	assert(false) -- ! SHOULD BE IMPLEMENTED BY DERIVER
end

-- return the max number of commands that the toolbar may contains
function toolbarTemplate:getMaxNumCommands()
	assert(false) -- ! SHOULD BE IMPLEMENTED BY DERIVER
end

-- get a button from its index in the toolbar (starting from 0)
function toolbarTemplate:getButton(index)
	assert(false) -- ! SHOULD BE IMPLEMENTED BY DERIVER
end

-- setup a button from a commandDesc.
function toolbarTemplate:setupButton(button, commandDesc, buttonIndex)
	assert(false) -- ! SHOULD BE IMPLEMENTED BY DERIVER
end

-- retrieve a command list for the given instance
function toolbarTemplate:getAvailableCommands(instance, dest)
	assert(false) -- ! SHOULD BE IMPLEMENTED BY DERIVER
end

-----------------------
-- PUBLIC INTERFACE  --
-----------------------

----------------------------------------------------------------------------------------------------
-- Setup the contextual toolbar for the given instance
-- passing nil just hides the toolbar
function toolbarTemplate:setupToolbar(instance)
	assert(false)
end

----------------------------------------------------------------------------------------------------
-- run a command in the toolbar from its index
function toolbarTemplate:runCommand(index)
	assert(false)
end



----------------------------------------------------------------------------------------------------
-- call this to create a new toolbar, then specialize it
function r2.newToolbar()
	local tb = clone(toolbarTemplate)
	
	-- description of each command
	tb.ToolbarCommands={}
	-- map index of each command to the matching toolbar button
	tb.IndexToButton = {}
	-- Current instance for which this contextual toolbar is displayed
	tb.CurrentInstance = nil
	-- Index of last triggered command
	tb.LastTriggeredCommandIndex = nil
	-- Cache for current commands
	tb.CurrentCommands = {}
	-- Command id to button 
	tb.CommandIdToIndex = {}
	
		
	----------------------------------------------------------------------------
	-- Setup the contextual toolbar for the given instance
	-- passing nil just hides the toolbar
	function tb:setupToolbar(instance)
		self.CurrentInstance = instance
		-- clear current list of commands
		table.clear(self.ToolbarCommands)
		table.clear(self.CurrentCommands)
		table.clear(self.IndexToButton)
		table.clear(self.CommandIdToIndex)
		self.LastTriggeredCommandIndex = nil
		local toolbar = self:getToolbar()
		local buttonList = toolbar:find("buttons")
		if not instance then
			toolbar.active = false		
			return
		end
		local commands = self.CurrentCommands
		table.clear(commands)
		self:getAvailableCommands(instance, commands)
		assert(self.ToolbarCommands)
		toolbar.active = true	
		local buttonIndex = 0
		local maxNumButtons = self:getMaxNumCommands()
		for index, commandDesc in pairs(commands) do
			local button
			if commandDesc.ButtonBitmap ~= "" and commandDesc.ButtonBitmap ~= nil then
				-- new button wanted			
				local button = self:getButton(buttonIndex)
				if not button then
					inspect(buttonList)
					assert(button)
				end
				self:setupButton(button, commandDesc, buttonIndex)
				self.IndexToButton[index] = button
				self.ToolbarCommands[buttonIndex + 1] = commandDesc
				self.CommandIdToIndex[commandDesc.Id] = index
				buttonIndex = buttonIndex + 1
				if buttonIndex == maxNumButtons then break end
			end
		end
		-- hide buttons that are not visible anymore
		for index = buttonIndex, maxNumButtons - 1 do
			local button = self:getButton(index)
			button.active = false
		end



	end

	----------------------------------------------------------------------------
	-- Run the command with the given index
	function tb:runCommand(index)
		assert(self.CurrentInstance)
		assert(self.ToolbarCommands[index] ~= nil)
		self.LastTriggeredCommandIndex = index
		-- do actual call		
		self.ToolbarCommands[index].DoCommand(self.CurrentInstance)
 	end
	return tb
end

----------------------------------------------------------------------------
-- Private : execute a command triggered by a toolbar button
function r2.runBaseToolbarCommand(toolbar, index)	
	assert(toolbar) -- deriver should provide the "Toolbar" field in the groups containing control that can trigger commands!
	toolbar:runCommand(index  + 1)	
end



























