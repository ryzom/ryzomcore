-- main tables of environement & general  editor functions

---------------
-- FUNCTIONS --
---------------

------------------------------------------------------------------------------------------------------------
-- Helper : eval a property in an instance of a class :
-- if the property is a value, then that value is returned
-- if the property is a function, then the 'this' parameter is passed to the function and the result is returned
-- if the property is nil, then the defualt value is returned
function r2:evalProp(prop, this, defaultValue)
	if type(prop) == "function" then return prop(this) -- could have been this:prop(), too...
	elseif prop ~= nil then return prop
	else return defaultValue
	end
end

-------------------------------------------------------------------------------------
-- get translation id for a property, followed by a boolean (false if not found)
function r2:getPropertyTranslationId(prop)
	local translationId	
	local found = true
	if prop.Translation ~= nil then
		translationId = prop.Translation
	elseif i18n.hasTranslation('uiR2EDProp_' .. prop.Name) then
		translationId = 'uiR2EDProp_' .. prop.Name
	elseif i18n.hasTranslation('uiR2ED' .. prop.Name) then
		translationId = 'uiR2ED' .. prop.Name
	else
		translationId = prop.Name
		found = false
	end	
	return translationId, found
end

-------------------------------------------------------------------------------------
-- get translation id for a property, as an ucstring
function r2:getPropertyTranslation(prop)
	local translationId = r2:getPropertyTranslationId(prop)		
	if (i18n.hasTranslation(translationId)) then
		return i18n.get(translationId)
	else
		return ucstring(translationId)
	end
end


------------------------------------------------------------------------------------------------------------
-- return the left quota for the current scenario (called by C++)
function r2:getLeftQuota()
	return r2.ScenarioWindow:getLeftQuota()
end


------------------------------------------------------------------------------------------------------------
-- retrieve class description of an instance (in the r2.Classes table)
function r2:getClass(instance)
	if instance == nil or instance.isNil then
		debugInfo("Calling r2:getClass on nil instance")
		debugInfo(debug.traceback())
		return
	end	
	if instance.Class == nil then
		debugInfo("Calling r2:getClass on class with no 'Class' field")
		debugInfo(debug.traceback())
		return
	end
	return r2.Classes[instance.Class]
end

------------------------------------------------------------------------------------------------------------
-- get parent instance of an object in the editor (that is an object with an InstanceId)
function r2:getParentInstance(object)
	debugInfo(debug.traceback())
	assert(nil) -- deprecated : use special member 'ParentInstance' instead
	--if object == nil then return nil end
	--local parent = object.Parent
	--while parent ~= nil do
	--	if parent.InstanceId ~= nil then
	--		return parent
	--	end
	--	parent = parent.Parent
	--end
	--return nil
end

------------------------------------------------------------------------------------------------------------
-- get max number of acts in a scenario
function r2:getMaxNumberOfAdditionnalActs()
	return getDefine("r2ed_max_num_additionnal_acts")
end

------------------------------------------------------------------------------------------------------------
-- Explore a tree of instance, each instance that has a sheet id is append to the list
function r2:exploreInstanceTree(obj, destTable)
	if obj.InstanceId ~= nil then
		table.insert(destTable, obj)
	end
	for k, v in specPairs(obj) do
		if type(v) == "userdata"  and v.Size ~= nil then			
			-- this is a sub array
			r2:exploreInstanceTree(v, destTable)			
		end
	end
end


-- test from a sheet id if an object is a bot object
function r2:isBotObject(sheetClient)
   return getCharacterSheetSkel(sheetClient, false) == ""
					   or string.match(sheetClient, "object_[%w_]*%.creature") -- patch for bot objects (with skeletons -> wind turbine)
end

-- helper function for pasting : relocate the 'Position' field of 'dest' for proper pasting
function r2:relocatePos(dest) 
   local x, y, z = r2:findEmptyPlace(dest.Position.x, dest.Position.y)
   if x ~= nil then
	  dest.Position.x = x
	  dest.Position.y = y
	  dest.Position.z = z
   end
end

-- Get a new position were to paste an object
function r2:getPastePosition()
	local x, y, z = r2:getUserEntityPosition()
	local fx, fy = r2:getUserEntityFront()
	x = x + fx * 4
	y = y + fy * 4
	x = x + 3 * math.random(-100, 100) / 100
	y = y + 3 * math.random(-100, 100) / 100
	local nx, ny = r2:findEmptyPlace(x, y)
	if nx ~= nil then
		return nx, ny, z 	
	else
		return r2:getUserEntityPosition() -- no empty place found, paste on user
	end	
end


------------------------------------------------------------------------------------------------------------------
--------------------
-- INIT / GLOBALS --
--------------------

--debugInfo("Initializing main tables")
if r2 == nil then
	r2 = {} -- for vianney's tests (is initialized by the caller otherwise)
end

------------------
-- MISC GLOBALS --
------------------

r2.ScratchUCStr = ucstring() -- scratch ucstring, useful to do call from utf8 without to create a new object

---------------------
-- EDITION GLOBALS --
---------------------

r2.maxId = {}

-- define in r2_features.lua
r2.Features={}

-- define in r2_basic_bricks.lua
r2.BasicBricks = {}

-- define in r2_palette.lua
r2.Palette = {}

-- definition of all R2 classes (contains both basic components and components of features)
r2.Classes = {}

-- current content of the clipboard after the selection has been copied
r2.ClipBoard = nil
r2.ClipBoardSrcInstanceId = nil
r2.ClipBoardDisplayName = nil


-----------------------
-- ANIMATION GLOBALS --
-----------------------

-- contains var related to animation
r2.AnimGlobals = 
{	
	Acts	   = nil,		-- set to nil until received by server, contains 
							-- the list of acts accessible at runtime by the animator
	UserTriggers  = nil,	-- set to nil until received by server, contains 
							-- the list of triggers that an animator can fire at runtime

	-------------------------------------------------------------------------------------
	reset = function(this)		
		this.Acts = nil
		this.UserTriggers = nil
	end
}




