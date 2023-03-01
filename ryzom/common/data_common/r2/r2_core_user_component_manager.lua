r2_core = {}

r2_core.UserComponentsPath = "./examples/user_components/"

r2_core.UserComponentTable = {}

r2_core.UserComponentManager = {}

local userComponentManager = r2_core.UserComponentManager

userComponentManager.CurrentExportList = {}

userComponentManager.InstanceIds = {}

userComponentManager.Texts = {}

userComponentManager.PlotItems = {}

userComponentManager.Positions = {}

userComponentManager.CurrentDesc = ""

userComponentManager.IsInitialized = false

userComponentManager.InitialComponentToLoad = 0


--------------------------------------------------------------------------------------------------
-- Methods called when the holder specific ui buttons are pressed
function r2_core:doNothing(x, y, z)

end

function r2_core:testIsExportable(entityId)
	local holder = r2:getInstanceFromId(r2_core.CurrentHolderId)
	assert(holder)
	local k, v = next(holder.Components, nil)
	local entity = r2:getInstanceFromId(entityId)

	while k do
		if entity.ParentInstance:isKindOf("NpcGrpFeature") 
			and entity.ParentInstance.InstanceId == v.InstanceId then
			return false
		end
		if v.InstanceId == entityId then
			return false
		end
		k, v = next(holder.Components, k)
	end

	return true

end

local function getIndex(id, tbl)	
	local k, v = next(tbl, nil)
	while k do
		if v.InstanceId and v.InstanceId == id then
			return k
		end
		k, v = next(tbl, k)
	end	
	return -1
end

function r2_core:addEntityToExporter(instanceId)	
	local entityId = instanceId
	local tmpInstance = r2:getInstanceFromId(entityId)
	if tmpInstance:isKindOf("Npc") and tmpInstance.ParentInstance:isKindOf("NpcGrpFeature") then
		entityId = tmpInstance.ParentInstance.InstanceId
	end
		
	local entity = r2:getInstanceFromId(entityId)
	assert(entity)
	
	local parent = entity.ParentInstance
	local parentId = parent.InstanceId

	r2_core.UserComponentManager:replacePosition(entityId)
	
	if parent:isKindOf("DefaultFeature") then
		local index = getIndex(entityId, parent.Components)	
		r2.requestMoveNode(parentId, "Components", index, r2_core.CurrentHolderId, "Components", -1)
	elseif parent:isKindOf("Act") then
		local index = getIndex(entityId, parent.Features)
		r2.requestMoveNode(r2:getCurrentAct().InstanceId, "Features", index, r2_core.CurrentHolderId, "Components", -1)	
	end
	local container = getUI("ui:interface:r2ed_scenario")
	local tree = container:find("content_tree_list")
	tree:forceRebuild()	
end


function r2_core:testCanPickUserComponentElement(instanceId)
	local instance = r2:getInstanceFromId(instanceId)
	assert(instance)
	local parent = instance.ParentInstance
	
	local holder = r2:getInstanceFromId(r2_core.CurrentHolderId)
	assert(holder)
	
	if parent:isKindOf("NpcGrpFeature") and holder.InstanceId == parent.ParentInstance.InstanceId then
		return true
	end

	if holder.InstanceId == parent.InstanceId then
		return true
	end
	return false
end

function r2_core:removeUserComponentElement(instanceId)
	local holder = r2:getInstanceFromId(r2_core.CurrentHolderId)
	assert(holder)

	local index = getIndex(instanceId, holder.Components)

	local entity = r2:getInstanceFromId(instanceId)
	assert(entity)
		
	local currentAct = r2:getCurrentAct()

	if entity:isKindOf("Region") or entity:isKindOf("Road") or entity:isBotObject() then
		local refPosition = r2.Scenario.Acts[0].Position
		r2_core.UserComponentManager:restorePosition(instanceId, refPosition)	
		r2.requestMoveNode(r2_core.CurrentHolderId, "Components", index, r2.Scenario.Acts[0].Features[0].InstanceId, "Components", -1)
	elseif entity:isKindOf("Npc") then
		local refPosition = currentAct.Position
		if entity.ParentInstance:isKindOf("NpcGrpFeature") then
			local grpId = entity.ParentInstance.InstanceId
			local grpIndex = getIndex(grpId, holder.Components)
			r2_core.UserComponentManager:restorePosition(grpId, refPosition)
			r2.requestMoveNode(r2_core.CurrentHolderId, "Components", grpIndex, currentAct.InstanceId, "Features", -1)
		else
			r2_core.UserComponentManager:restorePosition(instanceId, refPosition)
			r2.requestMoveNode(r2_core.CurrentHolderId, "Components", index, currentAct.Features[0].InstanceId, "Components", -1)
			--getCurrentAct().Features[0].Components
		end
	else
		local refPosition = currentAct.Position
		r2_core.UserComponentManager:restorePosition(instanceId, refPosition)
		r2.requestMoveNode(r2_core.CurrentHolderId, "Components", index, currentAct.InstanceId, "Features", -1)
		--getCurrentAct().Features
	end
	
	local container = getUI("ui:interface:r2ed_scenario")
	local tree = container:find("content_tree_list")
	tree:forceRebuild()
end

--------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------
-- Debug 
local printDebug
printDebug = function(tbl)
	local k,v = next(tbl, nil)
	while k do
		if type(v) == "table" then 
			debugInfo(k .. " : TABLE")
			printDebug(v)
		else 
			debugInfo(k .. " = " ..tostring(v))
		end
		k,v = next(tbl, k)
	end
end

local foundInTable = function(tbl, key)
	local k, v = next(tbl, nil)
	while k do
		if (k == key) then return true end
		k,v = next(tbl, k)
	end	
	return false
end

local insertExistingId = function(tblSearch, tblInsert, clientid)
	local k, v = next(tblSearch, nil)
	while k do
		if (k == clientid) then
			tblInsert[clientid] = v
		end
		k, v = next(tblSearch, k)
	end
end

local checkLinkedId = function(instanceIdTbl, refIdTbl)
	local k, v = next(refIdTbl, nil)
	while k do
		local key, value = next(instanceIdTbl, nil)
		while key do
			if key == k and value ~= v then
				refIdTbl[k] = value
			end
			key, value = next(instanceIdTbl, key)
		end	
		k, v = next(refIdTbl, k)
	end
end

local countIds = function(tbl)
	local count = 0
	local k, v = next(tbl, nil)
	while k do
		count = count + 1
		k, v = next(tbl, k)
	end
	return count
end

local countUCIds
countUCIds = function(tbl)
	local count = 0
	local k, v = next(tbl, nil)
	while k do
		if type(v) ~= "table" and type(v) ~= "userdata" then
			if k == "InstanceId" or string.find(tostring(v), "UserComponent_") ~= nil 
			or string.find(tostring(v), "UserText_") ~= nil or string.find(tostring(v), "UserItem_") ~= nil then
				count = count + 1
			end
		elseif type(v) == "table" then
			count = count + countUCIds(v)
		end
		k, v = next(tbl, k)
	end
	return count
end

local findUserSlotIn = function(str)
	local currentSlot = r2.getUserSlot() .."_"
	if string.find(str, currentSlot) ~= nil then
		return true
	end
	return false
end


--------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------
function userComponentManager:init()
	--init usercomponent table & retrieves entries from file if not empty
	r2_core.UserComponentTable = {}
	local userComponentTableOk = loadfile("save/r2_core_user_component_table.lua")
	if userComponentTableOk then
		userComponentTableOk()
	end
	--doFile on previously loaded user component files
	local ucTable = r2_core.UserComponentTable
	self.InitialComponentToLoad = table.getn(ucTable)
	local k, v = next(ucTable, nil)
	while k do
		--local fun = loadfile(v[2])
		r2.loadUserComponentFile(v[2])
		--if not fun then
		--	debugInfo("Couldn't load file '"..v[2].."' while initializing UserComponentManager.")
		--else
		--	local ok, msg = pcall(fun)
		--	if not ok then
		--		debugInfo("Error while loading component '"..filename.."' err: "..msg)
		--	end
		--	debugInfo("Component '" ..v[2].."' loaded.")
		--end
		k, v = next(ucTable, k)
	end

	if r2_core.UserComponentManager.IsInitialized == false and r2_core.UserComponentManager.InitialComponentToLoad == 0 then
		r2_core.UserComponentManager.IsInitialized = true
	end

end

function userComponentManager:isInitialized()
	return self.IsInitialized
end


--------------------------------------------------------------------------------------------------------------------
-- Gets all the InstanceIds and the references to InstanceIds and store them separately.
-- Each refId is unique and is renamed from client1_XX to userComponent_XX.
-- Returns a table containing all the instanceIds and refIds.
--

function userComponentManager:getRefIdTableFrom(argTbl)

	local refIdTbl = {}
	local instanceIdTbl = {}
	local plotItemTbl = {}

	local nbInstance = 0
	local nbText = 0
	local nbItems = 0

	
	local addTextToUCManager = function(instanceId)
		local key, value = next(r2.Scenario.Texts.Texts, nil)
		while key do
			if value.InstanceId == instanceId then
				userComponentManager.Texts[instanceIdTbl[instanceId]] = value.Text
				return
			end	
			key, value = next(r2.Scenario.Texts.Texts, key)
		end
	end 

	local addItemToUCManager = function(instanceId)
		local key, value = next(r2.Scenario.PlotItems, nil)
		debugInfo("Adding '" ..instanceId.."' to manager item table")
		while key do
			if value.InstanceId == instanceId then
				--copy the full plot item into manager's table
				userComponentManager.PlotItems[plotItemTbl[instanceId]] = value
				return
			end
			key, value = next(r2.Scenario.PlotItems, key)
		end
	end

	-- Doesn't insert the value if it's already in the table.
	parseInstanceTable = function(tbl)
		
		--parsing string properties
		local key,val = next(tbl, nil) 
		while key do
			if (type(val) ~= "userdata") then
				-- If the property is a real InstanceId
				if (key == "InstanceId") then 
					-- If the instanceId is not already in the instanceIdTbl then add it as a key, with its usercomponent
					-- translation as value.
					if not foundInTable(instanceIdTbl, tostring(val)) then
						nbInstance = nbInstance + 1
						instanceIdTbl[val] = "UserComponent_"..nbInstance
					end
				-- else if the instanceid found is a reference to another InstanceId
				--elseif (string.find(tostring(val), "Client1_") ~= nil) then
				elseif findUserSlotIn(tostring(val)) then
					--if exporting a dialog, rename specifically the refId pointing to the said text (stored in r2.Scenario.Texts)
					if key == "Says" then
						if not foundInTable(instanceIdTbl, tostring(val)) then
							nbText = nbText + 1
							instanceIdTbl[val] = "UserText_" ..nbText
							addTextToUCManager(val)
						end
					--else if plotItem XXXXXX
					elseif string.find(key, "Item") ~= nil then
						if not foundInTable(plotItemTbl, tostring(val)) then
							nbItems = nbItems + 1
							plotItemTbl[val] = "UserItem_" ..nbItems
							addItemToUCManager(val)
						end
					elseif not foundInTable(refIdTbl, tostring(val)) then
						if foundInTable(instanceIdTbl, tostring(val)) then --a refid pointing to an instanceid present in the component itself (zone...)
							insertExistingId(instanceIdTbl, refIdTbl, tostring(val))	
						else
							refIdTbl[val] = "Not exported" 
						end
					end
				end
			end
			key,val = next(tbl, key)
		end

		--parsing userdatas
		key, val = next(tbl, nil)
		while key do
			if (type(val) == "userdata") then
				parseInstanceTable(val)	
			end
			if (type(val) == "table") then 
				--inspect(val)
				assert(nil)
			end
			key, val = next(tbl, key)
		end
	end	
	
	local i = 1
	
	--printDebug(argTbl)
	local nbExportedInstances = table.getn(argTbl)

	local k, v = next(argTbl, nil)
	while k do
		local tmpInstance = r2:getInstanceFromId(v)
		if tmpInstance ~= nil then
			parseInstanceTable(tmpInstance)
		end
		k, v = next(argTbl, k)
	end
	
	checkLinkedId(instanceIdTbl, refIdTbl)

	userComponentManager.InstanceIds = instanceIdTbl

	local refIdTable = 
	{
		RefIds = refIdTbl,
		InstanceIds = instanceIdTbl,
		PlotItemsId = plotItemTbl
	}			
	return refIdTable
end

--------------------------------------------------------------------------------------------------------------------
local generatePreCode = function(fName)
	
	local userComponentBody = ""

	local writePlotItemBlock = function(plotItem, ucId, index)
		--file:write("\t{\n")
		local str = "\t{\n"
		local k, v = next(plotItem, nil)
		while k do
			if k == "InstanceId" then
				--file:write("\t\t[" ..string.format("%q", k).. "]\t=\t" ..string.format("%q", ucId).. ",\n")
				str = str .."\t\t[" ..string.format("%q", k).. "]\t=\t" ..string.format("%q", ucId).. ",\n"
			elseif type(v) == "string" then
				--file:write("\t\t[" ..string.format("%q", k).. "]\t=\t" ..string.format("%q", v).. ",\n")
				str = str .."\t\t[" ..string.format("%q", k).. "]\t=\t" ..string.format("%q", v).. ",\n"
			else
				--file:write("\t\t[" ..string.format("%q", k).. "]\t=\t" ..tostring(v).. ",\n")
				str = str .."\t\t[" ..string.format("%q", k).. "]\t=\t" ..tostring(v).. ",\n"
			end
			k, v = next(plotItem, k)
		end
		--file:write("\t},\n")
		str = str .."\t},\n" 
		return str
	end


	local featureName = fName
	if featureName == nil or featureName == "" then
		featureName = "UnnamedComponent"
	end 

	local str = ""
	-- "-- <Creation_Header>\n"
	--.."-- r2_core.CurrentFeatureName='" ..featureName.. "'\n"
	--.."-- </Creation_Header>\n\n\n" 	
	str = "r2.Features."..featureName.. " = {}\n\n"
	.."local feature = r2.Features." ..featureName.."\n\n"
	.."feature.Name=\"".. featureName.."\"\n\n"
	.."feature.Description=\"A user exported feature.\"\n"
	.."feature.Components = {}\n\n"
	--file:write(str)
	userComponentBody = userComponentBody .. str


	do
		local count = 0
		--file:write("feature.PlotItems = \n{\n")
		userComponentBody = userComponentBody .."feature.PlotItems = \n{\n"
		local k, v = next(userComponentManager.PlotItems, nil)
		while k do
			count = count + 1
			userComponentBody = userComponentBody..writePlotItemBlock(v, k, count)
			k, v = next(userComponentManager.PlotItems, k)
		end
		--file:write("}\n\n")
		userComponentBody = userComponentBody .."}\n\n"
	end
	-- component.createComponent
	str = "feature.createUserComponent = function(x, y)\n\t"
		
	.."local comp = r2.newComponent('UserComponentHolder')\n\t"
	.."assert(comp)\n\n\t"
	.."comp.Base = \"palette.entities.botobjects.user_event\"\n\t"
	.."comp.Name = r2:genInstanceName(ucstring('"..featureName.."')):toUtf8()\n\t"	
	.."comp.Position.x = x\n\t"
	.."comp.Position.y = y\n\t"
	.."comp.Position.z = r2:snapZToGround(x, y)\n\n\t"
	.."comp.Description = '" ..userComponentManager.CurrentDesc.. "'\n\n\t"
	--file:write(str)
	userComponentBody = userComponentBody .. str
	
	do
		--file:write("comp.Texts = \n\t{\n")
		userComponentBody = userComponentBody .."comp.Texts = \n\t{\n"
		local key, value = next(userComponentManager.Texts, nil)
		while key do
			--file:write("\t\t[" ..string.format("%q", key).. "]\t=\t[[" ..value.. "]],\n")
			userComponentBody = userComponentBody .. "\t\t[" ..string.format("%q", key).. "]\t=\t[[" ..value.. "]],\n"

			key, value = next(userComponentManager.Texts, key)
		end
		--file:write("\t}\n\n\t")
		userComponentBody = userComponentBody .. "\t}\n\n\t"
	end
	
	
	str = "comp.Components = {}\n\t"
	.."local tmpComponents = {}\n\t"
	
	userComponentBody = userComponentBody..str
	--file:write(str)
	return userComponentBody
end

--------------------------------------------------------------------------------------------------------------------
local generatePostCode = function(fName)
	local featureName = fName
	if featureName == nil or featureName == "" then
		featureName = "UserFeature"
	end 
	
	local str = ""

	str = str .."r2_core.UserComponentManager:insertAll(feature, comp, tmpComponents)\n\n\t"
	.."comp._Seed = os.time()\n\n\t"
	.."return comp\n"
	.."end\n\n\n"
	-- ! component.CreateComponent
		
	.."\n\nr2.Features[\""..featureName.."\"] =  feature"
	.."\n -- !"..featureName.." user exported\\o/ \n\n\n"

	return str
	--file:write(str)
end

-------------------------------------------------------------------------------------------------------------------
-- Generates the LUA code corresponding to the selected instances.
-- filename: file in which the code will be written.
-- argTbl: contains all the instanceIds selected for export.
-- instanceIdTbl: hashtable containing all clientIds (key) and their matching user component id (value). This table 
-- is returned by the getRefIdTableFrom method.
-- refPosition: reference coordinates chosen by the user on export.
function userComponentManager:componentToFile(filename, featureName, argTbl, refIdTbl, refPosition) 
	
	local instanceIdTbl = refIdTbl.InstanceIds
	local plotItemsTbl = refIdTbl.PlotItemsId

	local body = ""


	local function writeTab(file, nb)
		local i = nb
		while i > 0 do
			file:write("\t")
			i = i - 1
		end
	end

	local function writeTabInString(str, nb)
		local tmp = str
		local i = nb
		while i > 0 do
			tmp = tmp..("\t")
			i = i - 1
		end
		return tmp
	end 

	--local function writeDataToFile(file, tbl, nbTab, isTopLevel)
	local function writeDataToString(str, tbl, nbTab, isTopLevel)
		local userComponentBody = str
		
		local key,val = next(tbl, nil)
		while key do
			--writing all properties except userdatas 
			if type(val) ~= "userdata" then
				
				--writeTab(file, nbTab)
				userComponentBody = writeTabInString(userComponentBody, nbTab)
				if  isTopLevel == true and key == "InheritPos" then
					--file:write("InheritPos = 1,\n")
					userComponentBody = userComponentBody .. "InheritPos = 1,\n"
				else
					if type(key) == "number" then 
						--file:write("[" ..tostring(key + 1).. "] = ")
						userComponentBody = userComponentBody .. "[" ..tostring(key + 1).. "] = "
					else --file:write(tostring(key) .. " = ") end
						userComponentBody = userComponentBody .. tostring(key).. " = " 
					end
					
				 -- String/number value
					if type(val) == "number" then
						--file:write(val..",\n")
						userComponentBody = userComponentBody..tostring(val)..", \n"
					else
						local str = val
						if key == "InstanceId" or findUserSlotIn(val) then
							str = instanceIdTbl[val]
						end
						if str == nil then
							str = plotItemsTbl[val]
						end
						if str == nil then str = "" end --when finding a instanceId which is referring to a non exported entity
						--file:write(string.format("%q", str) ..",\n")
						userComponentBody = userComponentBody .. string.format("%q", str) ..",\n"
					end
				end
			end
			key,val = next(tbl, key)
		end

		--writing userdatas
		key, val = next(tbl, key)
		while key do
			if type(val) == "userdata" then
				--writeTab(file, nbTab)
				userComponentBody = writeTabInString(userComponentBody, nbTab)
				if type(key) ~= "number" then 
					--file:write(tostring(key) .. " = ") 
					userComponentBody = userComponentBody ..tostring(key).. " = "
				end

				--file:write("\n")
				userComponentBody = userComponentBody .."\n"
				--writeTab(file, nbTab)
				userComponentBody = writeTabInString(userComponentBody, nbTab)
				--file:write("{\n")
				userComponentBody = userComponentBody .."{\n"
				userComponentBody = writeDataToString(userComponentBody, val, nbTab + 1, false)
				--writeTab(file, nbTab)
				userComponentBody = writeTabInString(userComponentBody, nbTab)
				--file:write("},\n")
				userComponentBody = userComponentBody .."}, \n"
			end
			key,val = next(tbl, key)
		end
		
		return userComponentBody
	end
	
	--
	-- Write a code block for each component of the user feature.
	local function writeComponentBlocks(str, tbl)
		local userComponentBody = str
		local i = 1
		local nbInstanceIds = table.getn(tbl)
		
		while i <= nbInstanceIds do
			local tmpInstance = r2:getInstanceFromId(tbl[i])
			if (tmpInstance == nil) then
				debugInfo("Cannot export entity with intanceId= " ..tostring(tbl[i]))
				assert(tmpInstance)
			else
				
				local compName = "uComp" ..i --+ 1
				--file:write("do\n\t\t")
				userComponentBody = userComponentBody .. "do\n\t\t"
				--file:write("local " ..compName.. " = \n\t\t{\n")
				userComponentBody = userComponentBody .. "local " ..compName.. " = \n\t\t{\n"

				--writeDataToFile(file, tmpInstance, 3, true)			
				userComponentBody = writeDataToString(userComponentBody, tmpInstance, 3, true)
				
				--file:write("\t\t} --!" ..compName.. " \n\n\t\t")
				userComponentBody = userComponentBody .."\t\t} --!" ..compName.. " \n\n\t\t"
				--file:write("table.insert(tmpComponents, "..compName.. ")\n\t")
				userComponentBody = userComponentBody .."table.insert(tmpComponents, "..compName.. ")\n\t"
				--file:write("end\n\n\t")
				userComponentBody = userComponentBody .."end\n\n\t"
			end
			i = i + 1
		end
		return userComponentBody
	end
	
	--f = io.open(filename, "w")
	--assert(f)

	body = body..generatePreCode(featureName)
	
	body = writeComponentBlocks(body, argTbl) 	
	
	body = body .. generatePostCode(featureName)

	--f:close()

	--return res

	local headerInfo = {}
	table.insert(headerInfo, {CurrentFeatureName = featureName})
	r2.saveUserComponent(filename, headerInfo, body)
end


-------------------------------------------------------------------------------------------------------------------
-- Builds a table containing all user component ids (key) with their new instance id (ClientId_n) on import.
-- tmpComponents: contains all the user feature components, in which all instance ids are user component ids.
-- currentMaxId: max entity id in the current act (on user feature import).
local function generateReverseIdTable(tmpComponents, texts, currentMaxId)
	
	function findpattern(text, pattern, start)
		return string.sub(text, string.find(text, pattern, start))
	end

	local function findMaxId(component, maxId, maxPlotItemId)
		local maxIdLocal = maxId

		local maxItemId = maxPlotItemId

		local key, value = next(component, nil)
		while key do
			if type(value) ~= "table" then
				if key == "InstanceId" 
				or string.find(tostring(value), "UserComponent_") ~= nil then
					local tmpId = tonumber(findpattern(tostring(value), "%d+"))
					if tmpId and tmpId > maxIdLocal then maxIdLocal = tmpId end
				end
				if string.find(tostring(value), "UserItem_") ~= nil then
					local tmpItemId = tonumber(findpattern(tostring(value), "%d+"))
					if tmpItemId and tmpItemId > maxItemId then maxItemId = tmpItemId end
				end
			end
			key, value = next(component, key)
		end

		--maxIdLocal = maxIdLocal + maxItemId

		key, value = next(component, nil)
		while key do
			if type(value) == "table" then
				local tmpId, tmpItemId = findMaxId(value, maxIdLocal, maxItemId)
				if tmpId and tmpId > maxIdLocal then maxIdLocal = tmpId end
				if tmpItemId and tmpItemId > maxItemId then maxItemId = tmpItemId end
			end
			key, value = next(component, key)
		end
		return maxIdLocal, maxItemId
	end
	
	local reverseIdTable = {}
	local maxId, maxPlotItemId = findMaxId(tmpComponents, 0, 0)

	local i = 1
	local id = 0
	while i <= maxId do
		id = i + currentMaxId
		local ucName = "UserComponent_" ..i
		--reverseIdTable[ucName] = "Client1_" ..id
		reverseIdTable[ucName] = tostring(r2.getUserSlot()).."_"..id
		i = i + 1
	end

	local j = 1
	while j <= maxPlotItemId do
		id = i + currentMaxId
		local ucName = "UserItem_" ..j
		--reverseIdTable[ucName] = "Client1_" ..id
		reverseIdTable[ucName] = tostring(r2.getUserSlot()).."_"..id
		j = j + 1
		i = i + 1
	end
	
	--register component texts when a dialog has been exported
	if texts ~= nil then
		key, value = next(texts, nil)
		while key do
			local id = r2.registerText(tostring(value))
			reverseIdTable[key] = id.InstanceId
			key, value = next(texts, key)
		end
	end

	return reverseIdTable
end


--------------------------------------------------------------------------------------------------------------------
function userComponentManager:reattributeClientId(tbl, reverseIdTable)
	local key, value = next(tbl, nil)
	while key do
		if (type(value) ~= "userdata") then
			if key == "InstanceId" or string.find(tostring(value), "UserComponent_") ~= nil 
			or string.find(tostring(value), "UserText_") ~= nil 
			or string.find(tostring(value), "UserItem_") ~= nil then 
				local id = reverseIdTable[value]
				assert(id)
				tbl[key] = id 
			end
		end
		key,value = next(tbl, key)
	end
	key, value = next(tbl, nil)
	while key do
		if type(value) ~= "string" and type(value) ~= "number" then
			userComponentManager:reattributeClientId(value, reverseIdTable)	
		end
		key, value = next(tbl, key)
	end
	return tbl
end




--------------------------------------------------------------------------------------------------------------------
function userComponentManager:insertPlotItem(plotItem, reverseIdTable)
	
	local function isAlreadyUsed(plotItem)
		local k, v = next(r2.Scenario.PlotItems, nil)
		while k do
			if v.SheetId and v.SheetId == plotItem.SheetId then
				reverseIdTable[plotItem.InstanceId] = v.InstanceId
				return true
			end
			k, v = next(r2.Scenario.PlotItems, k)
		end
		userComponentManager:reattributeClientId(plotItem, reverseIdTable)
		return false
	end
	
	if isAlreadyUsed(plotItem) == false then
		--insert plot item in scenario
		r2.requestInsertNode(r2.Scenario.InstanceId, "PlotItems", -1, "", plotItem)
		--r2:setCookie(newItem.InstanceId, "SelfCreate", true)
		--TODO = delete plotItem from pitem table in scenario so that it cannot be used twice
		return true
	end
	return false
end 



--------------------------------------------------------------------------------------------------------------------
-- insertAll is called in the generated userFeature upon instanciation.
-- Inserts all userComponents into the userFeature's components table after having renamed all 
-- userComponentIds into clientIds.
--
function userComponentManager:insertAll(feature, comp, tmpComponents)
	local texts = comp.Texts
	local items = feature.PlotItems
	local components = comp.Components
	local currentMaxId = r2.getMaxId()
	local reverseIdTable = generateReverseIdTable(tmpComponents, texts, currentMaxId)
	
	local range = 0
	local nbItems = 0

	local k, v = next(items, nil)
	while k do
		local inserted = self:insertPlotItem(v, reverseIdTable)
		if inserted == true then 
			nbItems = nbItems + 1
		end
		k, v = next(items, k)
	end
	
	local key, value = next(tmpComponents, nil)
	while key do
		range= range + countUCIds(value) + 3
		userComponentManager:reattributeClientId(value, reverseIdTable)
		table.insert(components , value)
		key, value = next(tmpComponents, key)
	end
	r2.reserveIdRange(range + nbItems) 
end

---------------------------------------------------------------------------------------------------------------------
-- register the forms needed by the manager (save form, missing ids warning form).
-- Called along with the other features registerForms method.
function userComponentManager.registerForms()
	local fileListXML = 
	[[
	   <group id="tb_enclosing" sizeref="wh" w="-16" h="0" x="16" y="0" posref="TL TL">
			<instance template="inner_thin_border" inherit_gc_alpha="true"/>				
		</group>
		<group id="enclosing" sizeref="w" w="-10" h="196" x="5" y="-5" posref="TL TL">
			<group id="file_list" 
			  type="list"
			  active="true" x="16" y="0" posref="TL TL"				  
			  sizeref="w"
			  child_resize_h="true"
			  max_sizeref="h"
			  max_h="0"		  
			>			
			</group>
			<ctrl style="skin_scroll" id="scroll_bar" align="T" target="file_list" />
		</group>
	   <group id="gap" posref="BL TL" posparent="enclosing" w="1" h="6" />
	]]
	
	function getComponentNameFromFile(filename)
		local prefixedFilename = r2_core.UserComponentsPath..filename
		local f = io.open(prefixedFilename,"r")
		assert(f)
		f:read("*line")
		f:read("*line")
		f:read("*line")
		f:read("*line")
		f:read("*line")
		local line = f:read("*line")
		if string.find(line, "CurrentFeatureName") == nil then
			messageBox("Unable to load a component from this file. Please select another file.")
			return "No components found"
		end
		f:close()
		local luaString = string.sub(line, 3)
			
		loadstring(luaString)()
		local componentName = CurrentFeatureName
		CurrentFeatureName = ""
		return componentName
	end

	local function getComponentFiles(searchPath)
		local files = getPathContent(searchPath)
		local componentFiles = {}

		for k, v in pairs(files) do
			local prefixedFilename = r2_core.UserComponentsPath..nlfile.getFilename(v)
			local f = io.open(prefixedFilename,"r")
			assert(f)
			local header = r2.getFileHeader(prefixedFilename)
			--TODO : accès fichier md5
			--local line = f:read("*line")
			--if line == "-- <Creation_Header>" then
			if header.CurrentFeatureName then
				table.insert(componentFiles, v)
			end
		end

		return componentFiles
	end

	function r2_core.setCurrSelectedFile(filename, formAttr)
	   local formInstance = r2.CurrentForm.Env.FormInstance
	   --inspect(formInstance)
		if formInstance.LastFileButton then
		  formInstance.LastFileButton.pushed = false
	   end
		getUICaller().pushed = true
		formInstance.LastFileButton = getUICaller()
		r2.CurrentForm.Env.FormInstance[formAttr] = filename
		if r2.CurrentForm.Env.FormInstance["ComponentName"] then
			r2.CurrentForm.Env.FormInstance["ComponentName"] = getComponentNameFromFile(filename)
		end
		r2.CurrentForm.Env.updateAll()
		local eb =   r2.CurrentForm:find("eb")  
		setCaptureKeyboard(eb)
		eb:setSelectionAll()
	end

	function r2.setCurrSelectedComponent(compName)
		local formInstance = r2.CurrentForm.Env.FormInstance
		if formInstance.LastFileButton then
		  formInstance.LastFileButton.pushed = false
		end
		getUICaller().pushed = true
		formInstance.LastFileButton = getUICaller()
		r2.CurrentForm.Env.FormInstance.ComponentName = compName
		r2.CurrentForm.Env.updateAll()
		local eb =   r2.CurrentForm:find("eb")  
		setCaptureKeyboard(eb)
		eb:setSelectionAll()
	end

	-- called at init to fill the file list
	local function showFileList(formInstance)
		local fileGroupList = r2.CurrentForm:find('file_list')
		r2.CurrentForm.Env.FormInstance["ComponentFileName"] = "UserComponent1.lua"
		r2.CurrentForm.Env.FormInstance["Name"] = "UserComponent1"
		r2.CurrentForm.Env.updateAll()
		--local searchPath = select(config.R2ScenariiPath, "save")
		local searchPath = r2_core.UserComponentsPath
		--local files = getPathContent(searchPath)	
		local files = getComponentFiles(searchPath)
		table.sort(files)
		fileGroupList:clear()  
		for k, v in pairs(files) do
			if string.lower(nlfile.getExtension(v)) == "lua" then			
				local shortFilename = nlfile.getFilename(v)
				local entry = createGroupInstance("r2ed_filelist_entry", "", 
					{ id = tostring(k),  text = shortFilename, 
					params_l="r2_core.setCurrSelectedFile('" .. shortFilename .. "', 'ComponentFileName')" })
				fileGroupList:addChild(entry)
			end
		end
		setCaptureKeyboard(r2.CurrentForm:find("eb"))
	end

	--called at init to fill load form
	local function showUserComponentFileList(formInstance)
		local fileGroupList = r2.CurrentForm:find('file_list')
		--local searchPath = select(config.R2ScenariiPath, "save")
		local searchPath = r2_core.UserComponentsPath
		local files = getPathContent(searchPath)	
		table.sort(files)
		local defaultValue = ""
		fileGroupList:clear()  
		for k, v in pairs(files) do
			if string.lower(nlfile.getExtension(v)) == "lua" then			
				local shortFilename = nlfile.getFilename(v)
				if defaultValue == "" then defaultValue = shortFilename end
				local entry = createGroupInstance("r2ed_filelist_entry", "", 
					{ id = tostring(k),  text = shortFilename, 
					params_l="r2_core.setCurrSelectedFile('" .. shortFilename .. "', 'FileName')" })
				fileGroupList:addChild(entry)
			end
		end
	   setCaptureKeyboard(r2.CurrentForm:find("eb"))
	end


	--called at init to fill Unload Form
	local function showUserComponentList(formInstance)
		local fileGroupList = r2.CurrentForm:find('file_list')
		local featureNameTable = r2.FeatureTree.getUserComponentList()
		for k, v in pairs(featureNameTable) do
			local entry = createGroupInstance("r2ed_filelist_entry", "",
				{id = tostring(v), text=tostring(v),
				params_l="r2.setCurrSelectedComponent('"..tostring(v).."')" })
			fileGroupList:addChild(entry)
		end
		setCaptureKeyboard(r2.CurrentForm:find("eb"))
	end

	local function showMissingComponents()
		local formUI = r2:getForm("MissingIdsForm")
		local text = "The following objects are referenced in exported objects but will not be exported:\n"
		local k, v = next(formUI.Env.FormInstance.Value, nil)
		while k do
			text = text .."# " ..v.."\n"
			k, v = next(formUI.Env.FormInstance.Value, k)
		end
		text = text .."Continue anyway?"
		formUI:find('name_list').hardtext = text
		formUI.Env:updateAll()
		formUI.Env.updateSize()
		formUI:updateCoords()
		formUI:center()	
		formUI:updateCoords()
	end
	
	r2.Forms.MissingIdsForm =
	{
		
		Caption = "uiR2EdMissingRefsWarning",
		PropertySheetHeader = 
		[[
			<view type="text" id="name_list" multi_line="true" sizeref="w" w="-36" x="4" y="-2" posref="TL TL" global_color="true" fontsize="14" shadow="true" hardtext=""/>
		]],

		Prop =
		{
			{Name="Value", Type="Table", Visible=false}
		},
		onShow = showMissingComponents
	}

	r2.Forms.SaveUserComponent =
	{
		Caption = "uiR2EDExportUserComponent",
		PropertySheetHeader = fileListXML,
		Prop =
		{
			{Name="Name", Type="String", Category="uiR2EDRollout_Save", ValidateOnEnter = true }, 
			{Name="ComponentFileName", Type="String", Category="uiR2EDRollout_Save"},
			{Name="ComponentName", Type="Table", Visible = false}		
		},
		onShow = showFileList
	}

	r2.Forms.LoadUserComponent =
	{
		Caption = "uiR2EDExportLoadUserComponent",
		PropertySheetHeader = fileListXML,
		Prop =
		{
			{Name="FileName", Type="String", Category="uiR2EDRollout_Load", ValidateOnEnter = true },
			{Name="ComponentName", Type="String", Category="uiR2EDRollout_Load", WidgetStyle="StaticText"},
		},
		onShow = showUserComponentFileList
	}

	r2.Forms.UnloadUserComponent =
	{
		Caption = "uiR2EDExportUnloadUserComponent",
		PropertySheetHeader = fileListXML,
		Prop =
		{
			{Name="ComponentName", Type="String", Category="uiR2EDRollout_Unload", ValidateOnEnter = true },
		},
		onShow = showUserComponentList
	}

	
end

--------------------------------------------------------------------------------------------------------------------
function userComponentManager:buildCurrentExportList(holder)
	table.clear(self.CurrentExportList)
	local components = holder.Components
	local k, v = next(components, nil)
	while k do
		if v and v.InstanceId then
			table.insert(self.CurrentExportList, v.InstanceId)
		end
		k, v = next(components, k)
	end
end

--------------------------------------------------------------------------------------------------------------------
-- Tbl is a table containing all the instanceIds selected for export.
function userComponentManager:export(list, refX, refY, refZ)
	
	--local exportList = self.CurrentExportList
	local exportList = list
	assert(exportList)
	--builds a table containing all instanceIds and their corresponding userComponentId
	local refIdTable = userComponentManager:getRefIdTableFrom(exportList)
	
	local missingIds = refIdTable.RefIds
	local missingIdsCount = 0
	--XXXXXX
	
	--TODO = reattribute UC ids for plotitems + container

	-- User component filename confirmation
	local function onFileOk(form)
		if (form.ComponentFileName ~= nil and type(form.ComponentFileName) == "string" and form.RefPosition ~= nil) then
			if form.ComponentFileName == "" or string.len(form.ComponentFileName) < 2 then
				messageBox(i18n.get("uiR2EDInvalidName"))
				return
			end		
			if  string.find(form.ComponentFileName, '.lua', -4) == nil then
				form.ComponentFileName = form.ComponentFileName .. ".lua"
			end
			local refPosition = form.RefPosition
			local filename = form.ComponentFileName
			local prefixedFilename = r2_core.UserComponentsPath..filename
			local featureName = form.Name
			
			userComponentManager:computeAllPositions(exportList, refPosition, refIdTable.InstanceIds)
			userComponentManager:componentToFile(prefixedFilename, featureName, exportList, refIdTable, refPosition)
			userComponentManager:clear()
			--the component is automatically loaded on receive save callback 
			--r2.loadUserComponentFile(prefixedFilename)

			
		end
	end

	local function onFileCancel(form)
	
	end

	-- Position confirmation
	local function posOk(x, y, z)
		debugInfo(string.format("Validate export with reference position (%d, %d, %d)", x, y, z))
		local refPosition = {}
		refPosition["x"] = x
		refPosition["y"] = y
		refPosition["z"] = z
		refPosition["InstanceId"] = ""
		refPosition["Class"] = "Position"
		r2:doForm("SaveUserComponent", {RefPosition=refPosition}, onFileOk, onFileCancel)
	end

	local function posCancel()
		debugInfo("Export canceled.")
	end	

	-- Export confirmation when missing some ids
	local function confirmExport()
		debugInfo("Export confirmed.")
		r2:choosePos("", posOk, posCancel, "")
	end

	local function cancelExport()
		debugInfo("Export canceled.")
	end

	local missingTbl = {}
	local key, value = next(missingIds, nil)
	while key do
		if value == "Not exported" then
			local tmpInstance = r2:getInstanceFromId(key)
			if tmpInstance ~= nil then
				missingIdsCount = missingIdsCount + 1
				table.insert(missingTbl, tmpInstance.Name)
				debugInfo("Object named '" ..tmpInstance.Name .."' is referenced in an exported object but will not be exported.")
			end
		end
		key, value = next(missingIds, key)
		
	end
	
	if missingIdsCount ~= 0 then
		debugInfo(tostring(missingIdsCount) .." object(s) referenced but not exported. Continue anyway?y/n")
		r2:doForm("MissingIdsForm", {Value=missingTbl}, confirmExport, cancelExport)
	else
		--r2:choosePos("", posOk, posCancel, "")
		posOk(refX, refY, refZ)
	end
		
end -- !export


--------------------------------------------------------------------------------------------------------------------
function userComponentManager:getCurrentExportList()
	return userComponentManager.CurrentExportList
end

--------------------------------------------------------------------------------------------------------------------

function userComponentManager:isInExportList(target)
	local targetId = target.InstanceId
	local k, v = next(self.CurrentExportList, nil)
	while k do
		if v == targetId then
			return true
		end
		-- test if not a son of an already exported element
		local currParent = target.ParentInstance
		while currParent ~= nil do
			if currParent.InstanceId == v then
				return true
			end
			currParent = currParent.ParentInstance
		end
		k, v = next(self.CurrentExportList, k)
	end
	return false
end


--------------------------------------------------------------------------------------------------------------------
function userComponentManager:getCurrentExportList()
	return userComponentManager.CurrentExportList
end

--------------------------------------------------------------------------------------------------------------------

function userComponentManager:isInExportList(target)
	local targetId = target.InstanceId
	local k, v = next(self.CurrentExportList, nil)
	while k do
		if v == targetId then
			return true
		end
		-- test if not a son of an already exported element
		local currParent = target.ParentInstance
		while currParent ~= nil do
			if currParent.InstanceId == v then
				return true
			end
			currParent = currParent.ParentInstance
		end
		k, v = next(self.CurrentExportList, k)
	end
	return false
end

---------------------------------------------------------------------------------------------------------------------
function userComponentManager:addToExportList(instanceId)
	local instance = r2:getInstanceFromId(instanceId)
	if instance == nil then 
		debugInfo("UserComponentManager:AddToExportList : no instance")
		return false
	end

	table.insert(self.CurrentExportList, instanceId)
	
	return true

end

--------------------------------------------------------------------------------------------------------------------
function userComponentManager:clear()
	table.clear(userComponentManager.InstanceIds)
	table.clear(userComponentManager.Texts)
	table.clear(userComponentManager.PlotItems)
	table.clear(userComponentManager.Positions)
	table.clear(userComponentManager.CurrentExportList)
	userComponentManager.CurrentExportList = {}
	userComponentManager.CurrentDesc = ""
	debugInfo("UserComponentManager tables cleared.")
end


--------------------------------------------------------------------------------------------------------------------
function userComponentManager:computeAllPositions(argTbl, refPosition, refIdTable)
	
	local function buildPosition(instance, isTopLevel, refPosition)
		local position = {}
		if not instance.Position then return nil end
		if instance ~= nil and (instance.InheritPos == 0 or isTopLevel == true) then			
			position["x"] = r2.getWorldPos(instance).x - refPosition["x"]
			position["y"] = r2.getWorldPos(instance).y - refPosition["y"]
			position["z"] = 0				
		else
			position["x"] = instance.Position.x
			position["y"] = instance.Position.y
			position["z"] = instance.Position.z
		end
		position["InstanceId"] = refIdTable[instance.Position.InstanceId]
		position["Class"] = "Position"
		return position
	end


	local function computePositions(instance, isTopLevel, refPosition)		
		assert(instance)
		
		local localRefPos = nil
		
		localRefPos = buildPosition(instance, isTopLevel, refPosition)
		if localRefPos ~= nil and not userComponentManager.Positions[instance.Position.InstanceId] then
			userComponentManager.Positions[instance.Position.InstanceId] = localRefPos
		elseif localRefPos == nil then
			localRefPos = refPosition
		end

		local key, value = next(instance, nil)
		while key do
			if type(value) == "userdata" then
				local subInstance = value
				computePositions(subInstance, false, localRefPos)	
			end
			key, value = next(instance, key)
		end
	end

	
	if argTbl == nil then
	else
		local i = 1
		local nbExportedInstances = table.getn(argTbl)
		while i <= nbExportedInstances do
			local tmpInstance = r2:getInstanceFromId(argTbl[i])
			if tmpInstance ~= nil then
				computePositions(tmpInstance, true, refPosition)
			end
			i = i + 1 
		end
	end
end

---------------------------------------------------------------------------------------------------------------
function userComponentManager:removeIdFromExportList(id)
	local exportList = self.CurrentExportList
	assert(exportList)
	
	local k, v = next(exportList, nil)
	while k do
		if v == id then
			break
		end
		k, v = next(exportList, k)
	end
	if k ~= nil then
		table.remove(exportList,k)
		if table.getn(exportList) == 0 then
			self.CurrentExportList = {}
		end
		debugInfo(tostring(v) .." removed from exportList")
	end
end

--------------------------------------------------------------------------------------------------------------

function userComponentManager:registerUserComponentData(fileName)
	
	local function checkEntry(entry)
		local ucTable = r2_core.UserComponentTable
		local k, v = next(ucTable, nil)
		while k do
			if v[1] == entry[1] and self:isInitialized() == true then
				messageBox("A UserComponent called '"..entry[1].."' is already loaded. Unload it before loading another component with the same name.")
				return false
			end
			if v[2] == entry[2] and self:isInitialized() == true then
				messageBox("This file has already been loaded.")
				return false
			end
			k, v = next(ucTable, k)
		end
		return true
	end

	local f = io.open(fileName,"r")
	assert(f)
	f:read("*line")
	f:read("*line")
	f:read("*line")
	f:read("*line")
	f:read("*line")
	local line = f:read("*line")
	f:close()
	local luaString = string.sub(line, 3)
		
	loadstring(luaString)()
	
	local currentFeatureName = tostring(CurrentFeatureName)
	
	local entry = { currentFeatureName, fileName }
	if checkEntry(entry) == false then return end

	table.insert(r2_core.UserComponentTable, entry)

	local userComponentTable = r2_core.UserComponentTable
	local userComponentTableFile = io.open("save/r2_core_user_component_table.lua", "w")
	
	userComponentTableFile:write("r2_core.UserComponentTable = \n{\n")

	local k, v = next(userComponentTable , nil)
	while k do
		if v then
			userComponentTableFile:write("\t{")
			userComponentTableFile:write(string.format("%q", v[1]) ..", ")
			userComponentTableFile:write(string.format("%q", v[2]) ..", ")
			userComponentTableFile:write("},\n")
		end
		k, v = next(userComponentTable, k)
	end

	userComponentTableFile:write("} --!UserComponentTable")
	
	userComponentTableFile:close()

end
--------------------------------------------------------------------------------------------------------------------
function userComponentManager:unregisterComponent(featureName)
	local userComponentTable = r2_core.UserComponentTable
	local k, v = next(userComponentTable, nil)
	while k do
		if v[1] == featureName then
			break
		end
		k, v = next(userComponentTable, k)
	end

	if k ~= nil then
		table.remove(userComponentTable,k)
		if table.getn(userComponentTable) == 0 then
			r2_core.UserComponentTable = {}
		end
		debugInfo(tostring(v[1]) .." removed from loaded usercomponent table.")
	end
	local userComponentTableFile = io.open("save/r2_core_user_component_table.lua", "w")
	
	userComponentTableFile:write("r2_core.UserComponentTable = \n{\n")

	local k, v = next(userComponentTable , nil)
	while k do
		if v then
			userComponentTableFile:write("\t{")
			userComponentTableFile:write(string.format("%q", v[1]) ..", ")
			userComponentTableFile:write(string.format("%q", v[2]) ..", ")
			userComponentTableFile:write("},\n")
		end
		k, v = next(userComponentTable, k)
	end

	userComponentTableFile:write("} --!UserComponentTable")
	
	userComponentTableFile:close()

end

--------------------------------------------------------------------------------------------------------------------
function userComponentManager:unloadUserComponent()
	
	local function checkFeatureName(name)
		local tbl = r2_core.UserComponentTable
		local k, v = next(tbl, nil)
		while k do
			if v[1] == name then
				return true
			end
			k, v = next(tbl, k)
		end
		return false
	end

	local function onChoiceOk(form)
		local featureName = form.ComponentName
		if featureName == "" or checkFeatureName(featureName) == false then
			messageBox("This User Component is not loaded.")
			return
		end

		userComponentManager:unregisterComponent(featureName)
		r2.FeatureTree.removeUCFromTree(featureName)
		local featureGroupList = r2:getForm("UnloadUserComponent"):find('file_list'):clear()
		--inspect(featureGroupList)
		--local featureNode = featureGroupList:getRootNode():getNodeFromId(featureName)
		--featureNode:getFather():deleteChild(featureNode)
		
	end

	local function onChoiceCancel()
		local featureGroupList = r2:getForm("UnloadUserComponent"):find('file_list'):clear()
	end

	r2:doForm("UnloadUserComponent", {}, onChoiceOk, onChoiceCancel)
end

--------------------------------------------------------------------------------------------------------------------
function userComponentManager:loadUserComponent(fileName, body, header)
	--TODO : loading md5
	--local ok = loadfile(fileName)
	local ok = loadstring(body)
	if not ok then
		messageBox("UserComponentManager: Couldn't load file '" ..fileName.."'.")
		return false
	end
	ok()

	assert(header["CurrentFeatureName"])
	local currentFeatureName = header["CurrentFeatureName"]
		
	local userFeature = r2.Features[currentFeatureName]

	local componentId, component = next(userFeature.Components, nil)
	while (component ~= nil)
	do
		debugInfo("Registering user component " .. component.Name)
		r2.registerComponent(component)
		local class = r2.Classes[component.Name]
		class.NameToProp = {}
		for k, prop in pairs(class.Prop) do
			if prop.Name == nil then
				debugInfo("Found a property in class " .. tostring(class.Name) .. " with no field 'Name'")
			end
			class.NameToProp[prop.Name] = prop
		end
		assert(class)
		r2.Subclass(class)
		r2.registerGenerator(class)
		componentId, component = next(userFeature.Components, componentId)
	end
	
	r2.FeatureTree.addUserComponentNode(currentFeatureName)
	if self.IsInitialized == false then
		self.InitialComponentToLoad = self.InitialComponentToLoad - 1
	else
		userComponentManager:registerUserComponentData(fileName)
	end
end
--------------------------------------------------------------------------------------------------------------------



function userComponentManager:loadUserComponentFile()

	local function onChoiceOk(form)
		local filename = form.FileName
		local prefixedFilename = r2_core.UserComponentsPath..filename

		r2.loadUserComponentFile(prefixedFilename)

	end

	local function onChoiceCancel()
	end


	r2:doForm("LoadUserComponent", {}, onChoiceOk, onChoiceCancel)
end
---------------------------------------------------------------------------------------------------------------

function userComponentManager:computeNewPosition(instanceId, refPosition)
	
	local function buildPosition(instance, isTopLevel, refPosition)
		--local position = {}
		
		local position =  r2.newComponent("Position")
		if not instance.Position then return nil end
		if instance ~= nil and (instance.InheritPos == 0 or isTopLevel == true) then			
			position["x"] = r2.getWorldPos(instance).x - refPosition["x"]
			position["y"] = r2.getWorldPos(instance).y - refPosition["y"]
			position["z"] = r2:snapZToGround(r2.getWorldPos(instance).x, r2.getWorldPos(instance).y) - refPosition["z"]				
		else
			position["x"] = instance.Position.x
			position["y"] = instance.Position.y
			position["z"] = instance.Position.z
		end
		--position["InstanceId"] = instance.Position.InstanceId
		--position["Class"] = "Position"
		return position
	end


	local function computePositions(instance, isTopLevel, refPosition)		
		assert(instance)
		
		local localRefPos = nil
		
		localRefPos = buildPosition(instance, isTopLevel, refPosition)
		if localRefPos ~= nil and not userComponentManager.Positions[instance.Position.InstanceId] then
			userComponentManager.Positions[instance.Position.InstanceId] = localRefPos
		elseif localRefPos == nil then
			localRefPos = refPosition
		end

		local key, value = next(instance, nil)
		while key do
			if type(value) == "userdata" then
				local subInstance = value
				computePositions(subInstance, false, localRefPos)	
			end
			key, value = next(instance, key)
		end
	end	

	local tmpInstance = r2:getInstanceFromId(instanceId)
	if tmpInstance ~= nil then
		computePositions(tmpInstance, true, refPosition)
	end
	--table.clear(userComponentManager.Positions)
	
end

--------------------------------------------------------------------------------------------------------------------
function userComponentManager:insertNewPosition(instanceId)
	local instance = r2:getInstanceFromId(instanceId)
	local newPosition = userComponentManager.Positions[instance.Position.InstanceId]
	if not newPosition then
		debugInfo("Can't retrieve new position for '"..tostring(instance.Position.InstanceId))
		assert(0)
	end
	r2.requestSetNode(instanceId, "Position", newPosition)
end

--------------------------------------------------------------------------------------------------------------------
function userComponentManager:replacePosition(instanceId)
	local currentHolder = r2:getInstanceFromId(r2_core.CurrentHolderId)
	assert(currentHolder)
	
	local refPosition = currentHolder.Position
	
	self:computeNewPosition(instanceId, refPosition)
	
	self:insertNewPosition(instanceId)

end

--------------------------------------------------------------------------------------------------------------------
function userComponentManager:restorePosition(instanceId, refPosition)
	local instance = r2:getInstanceFromId(instanceId)
	local newPosition =  r2.newComponent("Position")
	if not instance.Position then return nil end
	if instance ~= nil then			
		newPosition["x"] = r2.getWorldPos(instance).x - refPosition["x"]
		newPosition["y"] = r2.getWorldPos(instance).y - refPosition["y"]
		newPosition["z"] = r2:snapZToGround(r2.getWorldPos(instance).x, r2.getWorldPos(instance).y) - refPosition["z"]
	end
	r2.requestSetNode(instanceId, "Position", newPosition)
end
--------------------------------------------------------------------------------------------------------------------

function r2.loadUserComponentCallback(filename, body, header)
	r2_core.UserComponentManager:loadUserComponent(filename, body, header)
	if r2_core.UserComponentManager.IsInitialized == false and r2_core.UserComponentManager.InitialComponentToLoad == 0 then
		debugInfo("# UserComponentManager init done.")
		r2_core.UserComponentManager.IsInitialized = true
	end
end

--------------------------------------------------------------------------------------------------------------------
function r2.displayModifiedUserComponentFileError()
	messageBox("You cannot load a UserComponent file which has been externally modified")
end
