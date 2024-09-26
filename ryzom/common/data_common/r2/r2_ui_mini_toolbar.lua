------------------
-- MINI TOOLBAR --
------------------


r2.MiniToolbar = r2.newToolbar()




-------------------------------------------------------------------
-- PRIVATE :IMPLEMENT METHODS REQUIRED BY r2_ui_toolbar_base.lua --
-------------------------------------------------------------------

-- return a reference on the toolbar
function r2.MiniToolbar:getToolbar()
	return getUI("ui:interface:r2ed_main_menu_button:mini_toolbar")
end

-- return the max number of command that the toolbar may contains
function r2.MiniToolbar:getMaxNumCommands()
	return tonumber(getDefine("r2ed_max_num_mini_buttons"))
end

-- get a button from its index in the toolbar
function r2.MiniToolbar:getButton(buttonIndex)
	if self.ButtonList == nil then
		self.ButtonList = self:getToolbar():find("buttons")
	end
	return self.ButtonList["b" .. tostring(buttonIndex)]
end

-- setup a button from a command in the toolbar
function r2.MiniToolbar:setupButton(button, commandDesc, buttonIndex)
	assert(button)
	button.active = true
	-- buld filenames for the 'normal', 'over' & 'pushed" textures
	local icon = commandDesc.ButtonBitmap
	local iconOver   = nlfile.getFilenameWithoutExtension(icon) .. 
	                   "_over." ..  nlfile.getExtension(icon)
	local iconPushed = nlfile.getFilenameWithoutExtension(icon) .. 
	                   "_pushed." ..  nlfile.getExtension(icon)	

	button.b.texture		      = icon
	button.b.texture_over       = iconOver
	button.b.texture_pushed     = iconPushed	
	button.b.tooltip			= i18n.get(commandDesc.TextId)
	button.Env.Toolbar = self
	
	button.b.onclick_l = "lua"
	button.b.params_l  = "r2.runBaseToolbarCommand(getUICaller().parent.Env.Toolbar, " .. tostring(buttonIndex) .. ")"	
end

-- retrieve a command list for the given instance
function r2.MiniToolbar:getAvailableCommands(instance, dest)	
	-- OBSOLETE
	instance:getAvailableMiniCommands(dest)
end

