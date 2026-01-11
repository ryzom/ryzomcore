-----------------------------------------------------------
-- Base class for objects that have a position in world  --
-----------------------------------------------------------

local worldObject = 
{
	BaseClass = "BaseClass",
	Name="WorldObject",
	Version = 1, -- Version 1 : removed the 'DisplayMode' field
	Prop = 
	{
		{Name="Position", Type="Position"},
		{Name="InheritPos", Type="Number", DefaultValue="1", Visible=false},
		-- DisplayMode : can have one of the following values :
		-- 0,  Visible -> the object is visible, this is the default
		-- 1,  Hidden  -> the object is not visible (not in scene / not in the minimap)
		-- 2,  Frozen  -> the object can't be selected, but is visible
		-- 3,  Locked  -> the object can be selected, but not moved/rotated
		
		-- 

		--{Name="DisplayMode", 
		-- Type="Number", 
		-- DefaultValue="0", 
		-- Visible=function(instance) return instance:isDisplayModeOptionAvailable() end,
		-- WidgetStyle="EnumDropDown", 
		-- Enum={ "uiR2EDDisplayModeVisible", "uiR2EDDisplayModeHidden", "uiR2EDDisplayFrozen", "uiR2EDDisplayLocked" }
		-- }
	}
}


------------------------------------------------------------------
function worldObject.updateVersion(this, scenarioValue, currentValue)
	local patchValue = scenarioValue
	if patchValue < 1 then						
		-- display mode removed in version 1
		if this.DisplayMode then
			r2.requestEraseNode(this.InstanceId, "DisplayMode", -1)
		end
		patchValue = 1
	end	
	if patchValue == currentValue then return true end
	return false
end

------------------------------------------------------------------
-- When this function returns 'true', the user is given the choice to change the display mode of the object
-- false by default
function worldObject.canChangeDisplayMode(this)
	return false
end

------------------------------------------------------------------
-- get world position as a table { x = , y = , z = }
function worldObject.getWorldPos(this)
	if this.InheritPos ~= 0 and this.ParentInstance then		
		local parentPos = this.ParentInstance:getWorldPos() 
		assert(parentPos and parentPos.x and parentPos.y and parentPos.z)
		return { x = this.Position.x + parentPos.x,
		         y = this.Position.y + parentPos.y,
				 z = this.Position.z + parentPos.z }
	end
	return { x = this.Position.x,
	         y = this.Position.y,
			 z = this.Position.z }
end



-- tmp fix for translation to primitives
function r2.getWorldPos(table)
	local instance = r2:getInstanceFromId(table.InstanceId)	
	if instance == nil then
		dumpCallStack(1)
		debugInfo("getInstanceFromId de "..table.InstanceId .." renvoie NIL")
		inspect(r2:getCurrentAct())
		assert(false)
	end
	local result = instance:getWorldPos()	
	return result
end

------------------------------------------------------------------
-- Make this object position relative to another position
-- & send appropriate network msg
-- If the object doesn't herit pos from parent, then its position remains unchanged
-- (example of use : grouping & ungrouping)
function worldObject.requestMakePosRelativeTo(this, parent)
	if not this.InheritPos then return end
	--debugInfo('requestMakePosRelativeTo')
	--debugInfo(colorTag(255, 0, 255) .. 'old local pos = ')
	--luaObject(this.Position)
	--debugInfo(colorTag(255, 0, 255) .. 'old world pos = ')
	--luaObject(this:getWorldPos())
	assert(parent)
	local worldPos = this:getWorldPos()	
	if parent.getWorldPos == nil then
		debugInfo(debug.traceback())
		inspect(parent)
		assert(nil)
	end		 
	local parentWorldPos = parent:getWorldPos()
	local newLocalPos = clone(this.Position)
	newLocalPos.x = worldPos.x - parentWorldPos.x
	newLocalPos.y = worldPos.y - parentWorldPos.y
	newLocalPos.z = worldPos.z - parentWorldPos.z
	--debugInfo(colorTag(255, 0, 255) .. 'parent world pos = ')
	--luaObject(parentWorldPos) 
	--debugInfo(colorTag(255, 0, 255) .. 'new local pos = ')
	--luaObject(newLocalPos) 
	if this.Position:equals(newLocalPos) then
		return -- same new pos, no need to send msg
	end	
	r2.requestSetNode(newLocalPos.InstanceId, "", newLocalPos)
end

------------------------------------------------------------------
-- redefine base class copy : ensure that position is expressed in world coordinates after the copy
function worldObject.copy(this)	
	local result = r2.Classes.BaseClass.copy(this)
	local worldPos = this:getWorldPos()
	result.Position.x = worldPos.x
	result.Position.y = worldPos.y
	result.Position.z = worldPos.z
	return result
end

------------------------------------------------------------------
-- Test if display mode can currenlty be changed by the user (maybe not the case if inherited by parent)
function worldObject.isDisplayModeOptionAvailable(this)
	if not this:canChangeDisplayMode() then return false end
	-- if inherited from parent and parent not visible then don't give any option
	local classDesc= this:getClass()
	if classDesc.DisplayerVisualParams and classDesc.DisplayerVisualParams.InheritDisplayMode then
		if this.ParentInstance and this.ParentInstance:isKindOf("WorldObject") and this.ParentInstance.DisplayerVisual.DisplayMode ~= 0 then
			return false
		end
	end
	return true
end

------------------------------------------------------------------
-- test if the given display mode toggle is supported for this entity
-- 0,  -> always supported (can be visible ...)
-- 1,  -> can this object be hidden ?
-- 2,  -> can this object be frozen ?
-- 3,  -> can this object be locked ?
function worldObject.isDisplayModeToggleSupported(this, displayMode)
	return false
end

------------------------------------------------------------------
-- If this entity has option to change its display mode, add them
function worldObject.getAvailableDisplayModeCommands(this, dest)	
	if not this:isDisplayModeOptionAvailable() then return end
	if this.DisplayerVisual.DisplayMode == 0 then
		if this:isDisplayModeToggleSupported(1) then
			table.insert(dest, this:buildCommand(this.onSetDisplayModeHide, "hide", "uiR2EDDisplayModeHide", "r2ed_toolbar_hide.tga", true))
		end
		if this:isDisplayModeToggleSupported(2) then
			table.insert(dest, this:buildCommand(this.onSetDisplayModeFreeze, "freeze", "uiR2EDDisplayModeFreeze", "r2ed_toolbar_freeze.tga", false))
		end
		if this:isDisplayModeToggleSupported(3) then
			table.insert(dest, this:buildCommand(this.onSetDisplayModeLock, "lock", "uiR2EDDisplayModeLock", "r2ed_toolbar_lock.tga", false))		
		end
	elseif this.DisplayerVisual.DisplayMode == 1 then
		if this:isDisplayModeToggleSupported(1) then
			table.insert(dest, this:buildCommand(this.onSetDisplayModeShow, "show", "uiR2EDDisplayModeShow", "r2ed_toolbar_show.tga", true))
		end
	elseif this.DisplayerVisual.DisplayMode == 2 then
		if this:isDisplayModeToggleSupported(2) then
			table.insert(dest, this:buildCommand(this.onSetDisplayModeShow, "unfreeze", "uiR2EDDisplayModeUnfreeze", "r2ed_toolbar_unfreeze.tga", false))
		end
	elseif this.DisplayerVisual.DisplayMode == 3 then
		if this:isDisplayModeToggleSupported(3) then
			table.insert(dest, this:buildCommand(this.onSetDisplayModeShow, "unlock", "uiR2EDDisplayModeUnlock", "r2ed_toolbar_unlock.tga", false))
		end
	end
end

------------------------------------------------------------------
-- display modes
function worldObject.onSetDisplayModeShow(this)
	--r2.requestNewAction(i18n.get("uiR2EDChangeDisplayAction"))		
	--r2.requestSetNode(this.InstanceId, "DisplayMode", 0)
	this.DisplayerVisual.DisplayMode = 0
	r2.ContextualCommands:update()
end
function worldObject.onSetDisplayModeHide(this)
	--r2.requestNewAction(i18n.get("uiR2EDChangeDisplayAction"))		
	--r2.requestSetNode(this.InstanceId, "DisplayMode", 1)
	this.DisplayerVisual.DisplayMode = 1
	r2.ContextualCommands:update()
end
function worldObject.onSetDisplayModeFreeze(this)
	--r2.requestNewAction(i18n.get("uiR2EDChangeDisplayAction"))		
	--r2.requestSetNode(this.InstanceId, "DisplayMode", 2)
	this.DisplayerVisual.DisplayMode = 2
	r2.ContextualCommands:update()
end
function worldObject.onSetDisplayModeLock(this)
	--r2.requestNewAction(i18n.get("uiR2EDChangeDisplayAction"))		
	--r2.requestSetNode(this.InstanceId, "DisplayMode", 3)   
	this.DisplayerVisual.DisplayMode = 3
	r2.ContextualCommands:update()
end



r2.registerComponent(worldObject)



