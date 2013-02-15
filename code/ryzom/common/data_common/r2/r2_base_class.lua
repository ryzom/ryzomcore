---------------------------------------------
-- Base class for R2 components / features --
---------------------------------------------

-- NB : throughout the file 'this' is used instead of the lua 'self' to denote the fact
-- that method are not called on the class definition, but on the instances of this class

baseClass = -- not local here because of the implementation ...
{
	----------------------
	-- CLASS PROPERTIES --
	----------------------	

	BaseClass          = "",			-- Name of the base class
    Version = 0,
	Name			   = "BaseClass",	-- Name of the class			
	BuildPropertySheet = true,          -- True if objects of this class should be editable by using a generic property sheet
	                                    -- Setting to 'true' will cause the framework to create the property sheet ui at launch
	Menu="",							-- ui path of the contextual menu for this class
	--
	DisplayerUI = "",					-- name of the C++ class that displays this object in the ui
	DisplayerUIParams = "",             -- parameters passed to the ui displayer when it is created
	--
	DisplayerProperties		= "R2::CDisplayerLua",		   -- 'lua' type of displayer takes the name of a lua function that must return the class of the displayer
	DisplayerPropertiesParams = "propertySheetDisplayer",  -- name of the function that build the displayer that update the property sheet.
	--
	TreeIcon="",						-- icon to be displayed in the tree (or a method returning it)
	PermanentTreeIcon="",				-- icon to be displayed in the tree if in permanent content (or a method returning it)
	SelectBarType = "",					-- type in select bar
	-- rollout header for the generic property sheet (a string giving the xml code for the header)	
	PropertySheetHeader = nil,

	-------------------
	-- CLASS METHODS --
	-------------------
	ClassMethods = {},

	--------------------------
	-- INSTANCES PROPERTIES --
	--------------------------
	-- * Unlike class properties above, they are created in each instance of the class. They are directly accessible in the objects.
	-- * They are C++ objects exposed through the metatable mechanism
	-- * They are *READ-ONLY*. They should be modified by network commands like r2.requestSetNode and the like
	--                         To store client side transcient values, use the native property 'User' (detailed below) instead
	-- TODO complete doc (available types, widgets ...)
	Prop = 
	{
		{Name="InstanceId", Type="String", WidgetStyle="StaticText", Category="Advanced", Visible=false },
	},	


	-- 'VIRTUAL' PROPERTIES (NOT IMPLEMENTED)
	-- Not really properties of this class, but 'shortcuts' to other, real properties
	-- The virtual properties are use by the property sheet to display properties than are jnot directly in the object,
	-- but that can be found in another place. (son object for example, or object that require an indirection)
	-- each virtual prop should provide a 'getPath' function which takes the InstanceId of the instance in which it is contained as its parameter
	-- It should return a couple : realOwnerInstanceId, realPropName (arrays not supported yet)
	-- an observer is placed on the real property to take its changes in account in the property sheet
	-- IMPORTANT: the getPath function will be called each frame to see if observers should be removed
	-- and placed on another target

	-- Example : indirection to fictive string property 'Toto' in the Scenario
	--VirtualProp = 
	--{
	--	{Name="InstanceId", Type="String", WidgetStyle="StaticText", Category="Advanced", Visible=true,
	--   getPath = function()
	--		return r2.Scenario.InstanceId, "Toto"
	--	 end,
	--	},
	--},

	---------------------------------
	-- NATIVE / SPECIAL PROPERTIES --
	---------------------------------

	-- They are implemented in C++ (thanks to the lua metatables)
	-- Parent		  : (R/O) :  The direct parent of this object. If parent object is in a table, returns the table
	-- ParentInstance : (R/O) : Next parent of this object with an InstanceId. In object is contained in a property that
	--					is a table, the object containing the table will be returned, not the table
	-- IndexInParent  : (R/O) : If parent is a table, then return index into it, -1 otherwise
	-- User			  : Read/Write lua table attached to the object. The 'User' field itself is R/O This is the place to store client side
	--                  edition variables.
	-- Size			  : (R/O) : If object is a table, returns its size, nil otherwise
	-- DisplayerUI    : (R/O) : for instances : returns reference to the ui displayer. May be nil. 
	--                  In the case where the field 'DisplayerUI' of this class definition id 'R2::CDisplayerLua', it would return
    --                  the displayer object created by a call to the function defined in 'DisplayerUIParams' (in this class definition)
	-- DisplayerVisual     : (R/O) : Same as DisplayerUI but for 'in scene' displayer
	-- DisplayerProperties : (R/O) : Same as DisplayerUI but for the properties displayer
	-- Selectable          : (R/W) : default is true. When false, the  object can't be selected in the scene. This flag is local to the instance (ancestor state not inherited)
	-- SelectableFromRoot  : (R/O) : True if this object and also its ancestor are selectable

	------------
	-- EVENTS --
	------------
	-- Events that are sent to the displayers, are also sent to instances that hold those displayers :
	-- By default they are not handled
	-- To handle, one shouldd redefine :
	-- function baseClass.onCreate(this)
	-- function baseClass.onErase(this)	
	-- etc ...
	-- see r2_ui_displayers.lua for details	

}

---------------------
-- GENERAL METHODS --
---------------------

-- Methods are defined in the class definition and are nevertheless callable on instances as follow :
-- instance:methodName(params ...)
-- In the class, the method would be defined as follow:
-- methodName = function(this, param1, param2 ...)  ... some code ... end
-- 'this' will be filled at runtime by a reference on the instance on which the method is called.
-- Method calling is possible thanks to the metamethod mechanism (in this case it is implemented in C++)
-- Calling a method is in fact equivalent to doing the following :
-- r2:getClass(instance).methodName(instance, param1, param2 ..)


---------------------------------------
-- TYPE / CLASS / PROPERTIES METHODS --
---------------------------------------

---------------------------------------------------------------------------------------------------------
-- return a reference to the class of this object
function baseClass.getClass(this)	
	return r2:getClass(this)
end

---------------------------------------------------------------------------------------------------------
-- get description of a property (as found in the 'Prop' table of the class definition) from its name
function baseClass.getPropDesc(this, propName)
	return this:getClass().NameToProp[propName]
end

---------------------------------------------------------------------------------------------------------
-- return name of the parent class
function baseClass.getClassName(this)
	return this.Class
end

---------------------------------------------------------------------------------------------------------
-- test if object is of the given class (or derived from the class)
-- param 'class' should be a string identifying the class
function baseClass.isKindOf(this, className)

	assert( type(this) == "userdata")
	local currClass = this:getClass()
	while currClass do
		if currClass.Name == className then
			return true
		end
		currClass = r2.Classes[currClass.BaseClass]
	end
	return false
end


---------------------------------------------------------------------------------------------------------
-- return a 'this' of the base class
-- Use this to access a method defined in a base class from a derived class
--
-- example : this:delegate():doThis() -- Call the doThis function in the parent class
--
-- Expected behavior is the same than with C++ :
-- Call from a delegated pointer is static
-- any further call is the call chain is polymorphic
-- Calls to delegate can be chained
-- NB : this function shouldn't be redefined in derived classes (the delegation mechanism uses a pointer on this function)
--function baseClass.delegate(this)	
--	return __baseClassImpl.delegate(this) -- implementation defined in "r2_base_class_private.lua"	
--end

---------------------------------------------------------------------------------------------------------
-- Get actual C++ "this" for this object. Because of the delegation mechanism, this may be a raw C++ object
-- or a lua table that performs the delegation
-- OBSOLETE, TO REMOVE
function baseClass.getRawThis(this)
	-- return __baseClassImpl.getRawThis(this) -- implementation defined in "r2_base_class_private.lua"
	return this
end


---------------------------------------------------------------------------------------------------------
-- compare current "this", with another "this" pointer
-- This should be the standard way to compare instance in memory because 'this' may sometime be a userdata
-- (raw C++ pointer to internal C++ object), or a table (delegated 'this' pointer)
-- OBSOLETE, TO REMOVE
function baseClass.isSameObjectThan(this, other)	
	--if this:isKindOf("Act") then
	--	breakPoint()
	--end
	--return this:getRawThis() == other:getRawThis()
	return this == other
end




---------------------------------------------------------------------------------------------------------
-- Helper : Return world position (that is, absolute position). By default, object deriving from the base class have no position
function baseClass.getWorldPos()
	return { x = 0, y = 0, z = 0 }
end

---------------------------------------------------------------------------------------------------------
-- When adding content, pionneer have a limited budget. This method gives the 'cost' of this object (0 by default)
--function baseClass.getUsedQuota(this)
--	return 0
--end

-------------------
-- SCENARIO COST --
-------------------

---------------------------------------------------------------------------------------------------------
-- See wether this element has a cost in the scenario
function baseClass.hasScenarioCost(this)
	return false
end

---------------------------------------------------------------------------------------------------------
-- get local cost cached in object
function baseClass.getLocalCost(this)
	return defaulting(this.User.Cost, 0)
end

---------------------------------------------------------------------------------------------------------
-- set local cost in object
function baseClass.setLocalCost(this, cost)
	this.User.Cost = cost
end

---------------------------------------------------------------------------------------------------------
-- get local static cost cached in object
function baseClass.getLocalStaticCost(this)
	return defaulting(this.User.StaticCost, 0)
end

---------------------------------------------------------------------------------------------------------
-- set local static cost in object
function baseClass.setLocalStaticCost(this, cost)
	this.User.StaticCost = cost
end



function baseClass.getStaticObjectCost(this) 
	return 0
end

function baseClass.getAiCost(this) 
	return 0
end

----------------------
-- OBJECT HIERARCHY --
----------------------

---------------------------------------------------------------------------------------------------------
-- append all sub-content that is "kind of" 'kind' to 'destTable' 
-- NB : this is very SLOW!!! please use iterators instead (see r2:enumInstances)
function baseClass.appendInstancesByType(this, destTable, kind)	
	assert(type(kind) == "string")	
	if this.CompareClass and this.CompareClass == true then
		if this.Class == kind then
			if destTable == nil then
				dumpCallStack(1)
			end
			table.insert(destTable, this:getRawThis())	
		end
	elseif this:isKindOf(kind) then
		if destTable == nil then
			dumpCallStack(1)
		end
		table.insert(destTable, this:getRawThis())		
	end	
end

---------------------------------------------------------------------------------------------------------
-- Append all instances rooted at this object (including this one)	
 
function baseClass.getSons(this, destTable)
	r2:exploreInstanceTree(this, destTable)		
end

---------------------------------------------------------------------------------------------------------
-- Search first ancestor of the wanted kind (that is of class 'className' or a derived class)
function baseClass.getParentOfKind(this, className)
	local parent = this.ParentInstance
	while parent do
		assert(parent.isKindOf)
		if parent:isKindOf(className) then return parent end
		parent = parent.ParentInstance
	end
	return nil
end

---------------------------------------------------------------------------------------------------------
-- Search parent until an act is found
function baseClass.getParentAct(this)
	return this:getParentOfKind("Act")
end

---------------------------------------------------------------------------------------------------------
-- Search parent until a scenario is found
function baseClass.getParentScenario(this)
	return this:getParentOfKind("Scenario")	
end

---------------------------------------------------------------------------------------------------------
-- See if hits object is inserted in the default feature (that is : single npcs, bot objects, roads etc. with no enclosing group or feature)
function baseClass.isInDefaultFeature(this)	
	return this.ParentInstance:isKindOf('DefaultFeature')
end


--------------------------
-- UI METHODS / DISPLAY --
--------------------------

---------------------------------------------------------------------------------------------------------
-- Called by the contextual menu/toolbar when the 'delete' option is chosen by the user
-- on THIS client
-- This is the place to perform additionnal deletion tasks
-- Example : a vertex may want to delete its containing region when there are 2 vertices left only
-- default -> just call 'r2.requestEraseNode'
function baseClass.onDelete(this)
	if this.User.DeleteInProgress == true then return end
	this.User.DeleteInProgress = true
	this:setDeleteActionName()
	this:selectNext()
	r2.requestEraseNode(this.InstanceId, "", -1)
	r2.requestEndAction()	
end

-- helper : add "delete : name_of_the_thing_being_deleted"  in the action historic as the name of the delete action that is about 
-- to be done
function baseClass.setDeleteActionName(this)
	r2.requestNewAction(concatUCString(i18n.get("uiR2EDDeleteAction"), this:getDisplayName()))
end


---------------------------------------------------------------------------------------------------------
-- Test wether the user can delete this object
function baseClass.isDeletable(this)
	if this.Deletable and this.Deletable == 0 then  return false end
	return true
end

---------------------------------------------------------------------------------------------------------
-- called when the instance is selected (default is no op)
function baseClass.onSelect(this)
   
end

---------------------------------------------------------------------------------------------------------
-- Tell if object can be selected as next object if a predecessor object
-- has been selected in the parent list
function baseClass.isNextSelectable(this)
	return false
end

---------------------------------------------------------------------------------------------------------
-- get next selectable object (or nil else)
function baseClass.getNextSelectableObject(this)
	local startIndex = this.IndexInParent
	if type(startIndex) ~= "number" then return nil end
	local currIndex = startIndex
	while true do
		currIndex = currIndex + 1
		if currIndex == this.Parent.Size then
			currIndex = 0
		end
		local instance = this.Parent[currIndex]
		if currIndex == startIndex then break end 
		if instance ~= nil then
			local firstSon = instance:getFirstSelectableSon()
			if firstSon ~= nil and firstSon:isNextSelectable() then
				return firstSon
			elseif instance.Selectable and instance:isNextSelectable() then
				return instance
			end
		end
	end
	if this.ParentInstance:isKindOf("DefaultFeature") then
		return this.ParentInstance:getNextSelectableObject()
	end
	return nil
end

---------------------------------------------------------------------------------------------------------
-- select object next to this one, if there's one
function baseClass.selectNext(this)	
	local nextSelection = this
	while 1 do
		nextSelection = nextSelection:getNextSelectableObject()
		if not nextSelection or nextSelection == this then
			r2:setSelectedInstanceId("")
			return
		end	
		if nextSelection then
			-- should not be frozen or hiden
			if (not nextSelection.DisplayerVisual) or (nextSelection.DisplayerVisual.DisplayMode ~= 1 and nextSelection.DisplayerVisual.DisplayMode ~= 2) then							
				r2:setSelectedInstanceId(nextSelection.InstanceId)
				return
			end			
		end	
	end
end

---------------------------------------------------------------------------------------------------------
-- if an object is not selectable if may nevertheless contain selectable object, the first one is returned by this method
function baseClass.getFirstSelectableSon(this)
	return nil
end

---------------------------------------------------------------------------------------------------------
-- get select bar type
function baseClass.getSelectBarType(this)
	return r2:evalProp(this:getClass().SelectBarType, this, "")	
end

---------------------------------------------------------------------------------------------------------
-- get name of tree icon
function baseClass.getTreeIcon(this)
	return r2:evalProp(this:getClass().TreeIcon, this, "")	
end	

---------------------------------------------------------------------------------------------------------
-- get name of tree icon
function baseClass.getPermanentTreeIcon(this)
	return r2:evalProp(this:getClass().PermanentTreeIcon, this, "")	
end	

---------------------------------------------------------------------------------------------------------
-- get name of tree icon according to permanent or current act
function baseClass.getContextualTreeIcon(this)
	if this:getParentAct() and this:getParentAct():isBaseAct() then
		return this:getPermanentTreeIcon()
	else
		return this:getTreeIcon()
	end
end	

---------------------------------------------------------------------------------------------------------
-- get name of permanent statut icon 
function baseClass.getPermanentStatutIcon(this)
	return "" 
end	
---------------------------------------------------------------------------------------------------------
-- get name of icon to be displayed in the slect bar
function baseClass.getSelectBarIcon(this)
   return this:getContextualTreeIcon()
end

---------------------------------------------------------------------------------------------------------
-- Get the display name (in i18n format). This name will be displayed in the property sheet or inthe instance tree
function baseClass.getDisplayName(this)
	local displayName = ucstring()
	if this.Name ~= nil and this.Name ~= "" then		
		displayName:fromUtf8(this.Name)
	else
		return i18n.get("uiR2EDNoName")
		-- local className = this.Class
		-- -- tmp patch
		-- if this:isKindOf("Npc") then			
		-- if this:isBotObject() then
		-- 	className = "Bot object"
		-- end
		-- end
		-- className = className .. " : " .. this.InstanceId
		-- displayName:fromUtf8(className)
	end
	return displayName
end

---------------------------------------------------------------------------------------------------------
-- Get the base name for instance name generation (should return a ucstring)
function baseClass.getBaseName(this)
	return ucstring("")
end


---------------------------------------------------------------------------------------------------------
-- return true if this instance can by displayed as a button in the select bar
function baseClass.displayInSelectBar(this)
	return true
end

---------------------------------------------------------------------------------------------------------
-- get first parent that is selectable in the select bar
function baseClass.getFirstSelectBarParent(this)
	local curr = this.ParentInstance
	while curr and not curr:displayInSelectBar() do
		curr = curr.ParentInstance
	end
	return curr
end

---------------------------------------------------------------------------------------------------------
-- search the first son that could be inserted in the select bar
-- default is to look recursively in the 'son select bar container'
function baseClass.getFirstSelectBarSon(this)
	local sons = this:getSelectBarSons()
	if not sons then return nil end
	for k, v in specPairs(sons) do
		if v:displayInSelectBar() then
			return v
		end
		local firstSelectBarSon = v:getFirstSelectBarSon()
		if firstSelectBarSon ~= nil then
			return firstSelectBarSon
		end
	end
end

---------------------------------------------------------------------------------------------------------
-- test if object can have sons than are displayable in the select bar
function baseClass.canHaveSelectBarSons(this)
	return false;
end

---------------------------------------------------------------------------------------------------------
-- return the default container that may contain object displayable in the select bar
function baseClass.getSelectBarSons()
	return nil
end

---------------------------------------------------------------------------------------------------------
-- called by the select bar when it displays its menu. Additionnal can be added there
function baseClass.completeSelectBarMenu(rootMenu)
	-- no-op
end

---------------------------------------------------------------------------------------------------------
-- The following method is called when the default ui displayer wants to know where to attach an object in the instance tree
-- Default behaviour is to return the tree node of the parent object when one is found
function baseClass.getParentTreeNode(this)	
	parent = this.ParentInstance
	while parent ~= nil do		
		if parent.User.TreeNodes ~= nil then
			return parent.User.TreeNodes
		end
		parent = parent.ParentInstance
	end	
	return nil
end

--------------------------------------------------------------------------------------------			
-- Helper function for features : return the feature parent tree node in their act
function baseClass.getFeatureParentTreeNode(this)	
	
	--return this:getParentAct():getContentTreeNodes("macro_components")
	return this:getParentAct():getContentTreeNodes()
end


--------------------------------------------------------------------------------------------
-- TODO: test if the object can be exported (true by default)
function baseClass.isExportable(this)
	return true
end

---------------------------------------------------------------------------------------------------------
-- This method is called by the C++ when the contextual menu is about to be displayed
function baseClass.onSetupMenu(this)
	local class = r2:getClass(this)	
	local menuName = class.Menu
	if menuName == nil then return end	
	local menu = getUI(menuName)	
	-- setup menu entries to select parents	
	--for i = 1,8 do		
	-- 	menu["selectParent" .. tostring(i)].active = false
	--end	
	-- local parent = this.ParentInstance
	-- for i = 1,9 do
	-- 	if parent == nil or parent.Parent == nil then break end		
	-- 	menu["selectParent" .. tostring(i)].active = true
	-- 	menu["selectParent" .. tostring(i)].uc_hardtext = i18n.get("uimR2EDSelectParent") + (parent.InstanceId .. "  (" .. parent.Class .. ")")
	-- 	--debugInfo(colorTag(0, 255, 255) .. tostring(i))
	-- 	parent = parent.ParentInstance
	-- end		
	-- -- setup cut & paste entries
	-- local cuttedSelection = r2:getCuttedSelection()
	-- if cuttedSelection and this.accept ~= nil then
	-- 	local canPaste = this:accept(cuttedSelection)
	-- 	debugInfo("canPaste = " .. tostring(canPaste))
	-- 	menu.paste.grayed = not canPaste
	-- else
	-- 	menu.paste.grayed = true
	-- end
   -- debug options   
   local extDebug = config.R2EDExtendedDebug == 1   
   menu.dump_lua_table.active   = extDebug
   menu.inspect_lua_table.active = extDebug
   menu.translateFeatures.active = extDebug
   menu.dump_dialogs_as_text.active = extDebug
   menu.update_dialogs_from_text.active =  extDebug

   menu.cut.active = extDebug
   menu.paste.active = extDebug

   

	r2.ContextualCommands:setupMenu(this, menu)

	

	-- delete entries for dynamic content
	-- local menuRoot = menu:getRootMenu()
	-- local startLine = menuRoot:getLineFromId("dynamic_content_start")
	-- local endLine = menuRoot:getLineFromId("dynamic_content_end")
	-- assert(startLine ~= -1 and endLine ~= -1)
	-- for lineToDel = endLine - 1, startLine + 1, -1 do
	--	menuRoot:removeLine(lineToDel)		
	-- end
	-- retrieve dynamic commands
	-- local commands = this:getAvailableCommands()
	-- local currentLine = startLine + 1
	-- local commandIndex = 1
	-- for commandIndex, command in commands do
	--	menuRoot:addLineAtIndex(currentLine, i18n.get(command.TextId), "lua", "", "dyn_command_" .. tostring(commandIndex))
		-- if there's a bitmap, build a group with the buitmap in it, and add to menu
		-- if command.ButtonBitmap and command.ButtonBitmap ~= "" then
		--	local menuButton = createGroupInstance("r2_menu_button", "", { bitmap = command.ButtonBitmap, })
		--	if menuButton then
		--		menuRoot:setUserGroupLeft(currentLine, menuButton)
		--	end
		-- end
		--currentLine = currentLine + 1		
	--end   
end
---------------------------------------------------------------------------------------------------------
-- Show the property window for this instance
-- (react to the event 'show properties' triggered in the ui, by contextual menu or toolbar)
function baseClass.onShowProperties(this)
	-- for now a global (see r2_ui_property_sheet.lua)
	r2:showProperties(this)
end

---------------------------------------------------------------------------------------------------------
-- Return list of currently available commands to launch on that instance.
-- such commands are displayed in the contextual toolbar or in the contextual menu.
-- Returned value should be an array (starting at index 1) with commands of the following format :
--
-- { DoCommand = function(instance) ...,  -- code to execute when the command is triggered (by menu or toolbar)
--                                        -- Because the function takes 'instance' as a parameter, it may be 
--                                        -- either a global function or a method of this class
--   Id = "",							  -- Id of the action. The action "r2ed_context_command" defined in actions.xml
--                                        -- will search for this id when a key is pressed to find the good action
--   TextId  = "...",                     -- Text id for entry menu & toolbar tooltip
--   ButtonBitmap = "filename.tga",       -- Name of the button to display in the toolbar, nil
--                                        -- or "" if the command should not appear in the contextual toolbar   
--   Separator = "true",                  -- optionnal, false by default : specify if there should be a separator 
--                                        -- between this button and previous buttons
--   ShowInMenu = false,                   -- false if the entry shouldn't be displayed in the menu
--   IsActivity = false					  -- true if event is an activity
-- }
--
-- 'getAvailableCommands' should be called by derived class, possibly adding their
--  own commands
--
-- See also : 'buildCommand'

function baseClass.getAvailableCommands(this, dest)	
	if this.ParentInstance:isKindOf("UserComponentHolder") then
		table.insert(dest, this:buildCommand(this.onRemoveFromUserComponent, "removeFromUserComponent", "uimR2EDRemoveFromUserComponent", ""))
	end
	if this:isDeletable() and this.User.DeleteInProgress ~= true then
		table.insert(dest, this:buildCommand(this.onDelete, "delete", "uimR2EDMenuDelete", "r2_toolbar_delete.tga"))
	end
	if this:getClass().BuildPropertySheet then
		table.insert(dest, this:buildCommand(this.onShowProperties, "properties", "uimR2EDMenuProperties",  "r2_toolbar_properties.tga", true))
	end

	if this:isKindOf("NpcCustom") then
		table.insert(dest, this:buildCommand(this.customizeLook, "customize_look", "uiR2EDCustomizeLook", "r2_toolbar_customize_look.tga", false))
	end

end

---------------------------------------------------------------------------------------------------------
-- Build a single command entry to be used by  'getAvailableCommands'
-- A command entry translates into a button in the contextual toolbar
function baseClass.buildCommand(this, command, id, textId, bitmap, separator, showInMenu)
	if showInMenu == nil then showInMenu = true end
	return 
	{	
		DoCommand    = command,
		TextId       = textId,
		Id			 = id,
		ButtonBitmap = bitmap,
		Separator    = separator,
		ShowInMenu   = showInMenu,
		IsActivity   = false
	}
end

---------------------------------------------------------------------------------------------------------
-- same as 'buildCommand', but for activities
function baseClass.buildActivityCommand(this, command, id, textId, bitmap, separator, showInMenu)
	local result = this:buildCommand(command, id, textId, bitmap, separator, showInMenu)
	result.IsActivity = true
   return result
end

---------------------------------------------------------------------------------------------------------
-- Special, class method (not instance method) for dev : returns a table containing all infos on which the xml generic property sheet depends 
-- When this table is modified, then the xml property sheet will be rebuild for this class (when 'resetEditor'
-- is called for example.
function baseClass.ClassMethods.getGenericPropertySheetCacheInfos(this)
	local infos = {}
	infos.Prop = this.Prop									-- if one of the properties change, then must rebuild the property sheet
	infos.PropertySheetHeader = this.PropertySheetHeader    -- if the xml header change, then must rebuild the sheet, too
	return infos
end


---------------------------------------------------------------------------------------------------------
-- get list of command for display in the mini toolbar
function baseClass.getAvailableMiniCommands(this, result)	
	-- OBSOLETE
	--	table.insert(result, this:buildCommand(this.editDialogs, "edit_dialogs", "uiR2EDEditDialogs",  "r2_icon_dialog_mini.tga"))
	--	table.insert(result, this:buildCommand(this.editActions, "edit_actions", "uiR2EDEditActions",  "r2_icon_action_mini.tga"))
	--	table.insert(result, this:buildCommand(this.editReactions, "edit_reactions", "uiR2EDEditReactions",  "r2_icon_reaction_mini.tga"))	
end

---------------------------------------------------------------------------------------------------------
-- Return true if sequences can be edited on that object
function baseClass.isSequencable(this)
	return false
end

---------------------------------------------------------------------------------------------------------
-- For sequencable object only (baseClass.isSequencable) : return the lookup string for a verb from an activity name
-- Indeed, an activity may have different name depending on who performs it
-- for example, the "Feed In Zone" activity will be name "work" for a worker kitin instead of "feed" for a carnivore
function baseClass.getActivityVerbLookupName(this, activityName)
	return activityName
end


---------------------------------------------------------------------------------------------------------
-- is the object global to the scenario ? The select bar will call this to force the good update
-- for global objects that are selectable (plot items ...)
function baseClass.isGlobalObject(this)
	return false
end



function baseClass.onRemoveFromUserComponent(this)
	r2_core.CurrentHolderId = this.ParentInstance.InstanceId
	r2_core:removeUserComponentElement(this.InstanceId)
end

-------------
-- REF IDS --
-------------

-- Set the value of a refId inside this object. (empty string to delete)
-- This will push a new action name & call r2.requestNode
function baseClass.setRefIdValue(this, refIdName, targetId)	
	local name = this:getDisplayName()
	local refIdUCName = r2:getPropertyTranslation(this:getClass().NameToProp[refIdName])
	if targetId == "" then
	r2.requestNewAction(concatUCString(i18n.get("uiR2EDRemovingTargetAction"), name,
								i18n.get("uiR2EDAddingReferenceSeparator"), refIdname))
	else
		local targetName = r2:getInstanceFromId(targetId):getDisplayName()
		r2.requestNewAction(concatUCString(i18n.get("uiR2EDAddingReferenceAction"), name,
									i18n.get("uiR2EDAddingReferenceSeparator"), refIdUCName,
									i18n.get("uiR2EDAddingReferenceToAction"), targetName))
	end
	r2.requestSetNode(this.InstanceId, refIdName, targetId)

end


---------------------------
-- COPY / PASTE HANDLING --
---------------------------

---------------------------------------------------------------------------------------------------------
-- see if this object support copy (false by default)
function baseClass.isCopyable(this)
	return false
end

---------------------------------------------------------------------------------------------------------
-- Create a canonical copy of this object, this copy can be used by subsequent calls to 'newCopy'
function baseClass.copy(this)	
	-- implementation in "r2_base_class_private.lua"
end


-- Create a new copy from a canonical copy
-- New instance ids are generated
-- Default behavior is to remove all external dependencies and to rename 
-- internal dependencies
-- The result can be used as input to 'paste' & 'ghostPaste'
function baseClass.newCopy(canonicalCopy)	
	-- implementation in "r2_base_class_private.lua"
end


---------------------------------------------------------------------------------------------------------
-- Paste the current clipboard
-- not really a method here, because 'src' id a lua table (should be the content of the clipboard ...) that can be used with 
-- a r2.request.. command.
-- - this function should copy the result in a suitable place (maybe into current selection, or at global scope)
--   NB : if newPlace is not true, then the result should be past at the coordinates found in src
-- - It should check that there is some room in the scenario before doing the copy
function baseClass.paste(src, newPlace, srcInstanceId)	
	if r2:getLeftQuota() <= 0 then
		r2:makeRoomMsg()
		return
	end
end

function baseClass.pasteGhost(src)	
		
	if r2:getLeftQuota() <= 0 then
		r2:makeRoomMsg()
		return
	end
end



-- TMP TMP : move events test

--
function baseClass.onTargetInstancePreHrcMove(this, targetAttr, targetIndexInArray)
	debugInfo(string.format("instance: pre hrc move : %s", targetAttr))
end
--
function baseClass.onTargetInstancePostHrcMove(this, targetAttr, targetIndexInArray)
	debugInfo(string.format("instance : post hrc move : %s", targetAttr))	
end



-- IMPLEMENTATION
r2.doFile("r2_base_class_private.lua")


r2.registerComponent(baseClass)

baseClass = nil




















