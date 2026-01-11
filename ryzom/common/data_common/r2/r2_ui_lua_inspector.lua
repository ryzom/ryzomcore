

------------------------------------------------------------------------------------------------------------
-- Create interface for inspecting a lua table
-- The resultat is displayedin a tree control in the UI
-- Also support display of reflected objects (interface group & widgets etc.)
-- TODO place this elsewhere because useful for other stuffs than r2

local maxEnumerationSize = 4000 -- if a enumeration id done for more than this elements, than
                                -- this value, then a possible infinite loop may have happened so abort the inspection

function inspect(object, maxdepth, alreadySeen)

	if (object ~= nil and object.isNil) then
		debugInfo("Inspecting nil object")
		object = nil
	end

	local container = getUI("ui:interface:lua_inspector") 
	local treeCtrl = getUI("ui:interface:lua_inspector:content:sbtree:tree_list")


	local inspectedTableToTreeNode = {} -- for each table / user data, give a pointer to the tree node that is displayed

	if treeCtrl == nil or container == nil then
		debugWarning("Cannot find inspector ui")
		return
	end

	if alreadySeen == nil then
		alreadySeen = {}
	end

	if (maxdepth == nil) then maxdepth = 1 end


	-- color for each type
	local typeColor =
	{
		number = CRGBA(255, 255, 255, 255),
		string = CRGBA(0, 255, 255, 255),
		boolean = CRGBA(255, 255, 0, 255),
		thread = CRGBA(128, 128, 128, 255),
		table = CRGBA(255, 128, 32, 255),
		userdata = CRGBA(255, 128, 64, 255)	
		
	}

	typeColor["function"] = CRGBA(255, 0, 255, 255) -- separate code here because 
	typeColor["nil"] = CRGBA(255, 0, 0, 255)


	local id = 0

	-- generate a new ID for a tree node
	local function newId() id = id + 1; return tostring(id) end


	-- return xml code for a leaf node
	local function node(content, color, opened)	
		if opened == nil then opened = true end
		--debugInfo(colorTag(255, 255, 0) .. "node")	
		if color == nil then color = CRGBA(127, 255, 127, 0) end
		local result = SNode()
		result.Id = newId()
		result.Text = content
		if type(color) ~= "userdata" then
			debugInfo(tostring(color))
		end
		result.Color = color
		result.YDecal = -1
		result.FontSize = 15	
		result.Opened = opened
		-- debugInfo(result)
		return result
	end




	local function objectStr(Name, value) 
		--return tostring(Name).. " (" .. type(value) .. ")"  .. " = " .. tostring(value) 
		return tostring(Name) .. " = " .. tostring(value) 
	end


	-- for each type, gives the code that generate the tree for the inspector
	local typeTable = 
	{
		number = function(key, val) return node(objectStr(key, val), typeColor[type(val)]) end,
		string = function(key, val) return node(objectStr(key, val), typeColor[type(val)]) end,
		boolean = function(key, val) return node(objectStr(key, val), typeColor[type(val)]) end,	
		thread = function(key, val) return node(objectStr(key, "thread"), typeColor[type(val)]) end,	
	}

	-- 'function' is a declared keyword, so must declare it this way
	typeTable["function"] = 
	function(key, val) 
		local caption = "function (" .. debug.getinfo(val).short_src .. ", " .. tostring(debug.getinfo(val).linedefined) .. ")"		
		return node(objectStr(key, caption), typeColor[type(val)]) 
	end
	typeTable["nil"] = function(key, val) return node(objectStr(key, "nil"), typeColor[type(val)]) end

	-- recursive code to generate the code for a table
	typeTable.table = function(key, val, depth)
		local mt = getmetatable(val)
		if type(val) == "userdata" and (mt == nil or type(mt.__next) ~= "function") then
			-- this is a user data with no 'next' method, so can't enumerate
			return node(tostring(key) .. "(key type = " .. type(key) .. ")", typeColor[type(val)], false);			
		end
		-- if type(val) == "userdata" then
		--	debugInfo(colorTag(255, 255, 0) .. "found user data with '__next' metamethod")
		-- end
		if (alreadySeen[val] == true) then -- avoid cyclic graph
			return node("!CYCLE!", CRGBA(0, 255, 0, 255))
		end
		alreadySeen[val] = true
		-- create a temp table sorted by type
		local sortedTable = {}
		local index = 1			
		for k, v in specPairs(val) do	
			sortedTable[index] = { key = k, value = v }; 
			index = index + 1
			if index - 1 > maxEnumerationSize then				
				error(colorTag(255, 0, 0) .. "inspect : table enumeration is over " .. tostring(maxEnumerationSize) .. " elements, possible infinite loop, aborting.")				
			end
		end
		local function comp(val1, val2) 				
			-- sort by type, then by key
			if not (type(val1.value) == type(val2.value)) then return type(val1.value) < type(val2.value) end
			return tostring(val1.key) < tostring(val2.key)
		end
		table.sort(sortedTable, comp)		
		--debugInfo("VAL")
		--luaObject(val)
		--debugInfo("SORTED")
		--luaObject(sortedTable)
		local rootNode = node(tostring(key) .. "(key type = " .. type(key) .. ")", typeColor[type(val)], depth < maxdepth);
		inspectedTableToTreeNode[val] = rootNode
		local elemCount = 0
		for key, value in specPairs(sortedTable) do				
			if (typeTable[type(value.value)] == nil) then
				rootNode.addChild(node("?"))				
			else
				-- add node for a field of the table
				rootNode:addChild(typeTable[type(value.value)](value.key, value.value, depth + 1))
			end
			elemCount = elemCount + 1
			if elemCount > maxEnumerationSize then				
				error(colorTag(255, 0, 0) .. "inspect : table enumeration is over " .. tostring(maxEnumerationSize) .. " elements, possible infinite loop, aborting.")				
			end			
		end				
		return rootNode
	end

	typeTable.userdata = typeTable.table


	-- generate the tree
	local rootNode = typeTable[type(object)]("#object#", object, 0)


	treeCtrl:setRootNode(rootNode)
	treeCtrl:forceRebuild()
	if container.active == false then
		container.active = true
		container:center()
	end

	setOnDraw(container, "onLuaInspectorDraw()")

	-- store the inspected object in the container lua environment for further refresh
	container.Env.InspectedObject = object
	container.Env.InspectedTableToTreeNode = inspectedTableToTreeNode
	container.Env.AutoRefreshWidget = container:find("auto_refresh_button")
	if container.Env.AutoRefreshWidget == nil then
		debugInfo("lua inspector : Can't find auto_refresh button")
	end
end

------------------------------------------------------------------------------------------------------------
function refreshLuaInspector()
	local container = getUI("ui:interface:lua_inspector") 	
	if container == nil then
		debugWarning("Cannot find inspector ui")
		return
	end
	local inspectedObject = container.Env.InspectedObject
	if inspectedObject ~= nil then
		-- memorize open / close state of each node
		local openTreeNode = {}
		-- special cases for objects with an instance id

		--
		for k, v in pairs(container.Env.InspectedTableToTreeNode) do
			if not v.isNil then -- if tree node has not been deleted ...
				openTreeNode[k] = v.Opened
			end
		end
		inspect(inspectedObject)
		-- restore the open state for each node
		for k, v in pairs(openTreeNode) do
			local newTreeNode = container.Env.InspectedTableToTreeNode[k]
			if newTreeNode ~= nil and not newTreeNode.isNil then
				newTreeNode.Opened = v
			end
		end
	end
end

------------------------------------------------------------------------------------------------------------
function onLuaInspectorDraw()
	local container = getUI("ui:interface:lua_inspector") 	
	if container == nil then		
		return
	end
	if not container.active then
		return
	end
	local autoRefreshWidget = container.Env.AutoRefreshWidget
	if  autoRefreshWidget == nil then		
		return
	end
	if autoRefreshWidget.pushed == true then
		refreshLuaInspector()
	end
end

