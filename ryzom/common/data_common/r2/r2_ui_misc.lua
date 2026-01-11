-- misc ui helper functions

------------------------------------------------------------------------------------------------------------
-- backup window coordinates
function r2:backupWndCoords(wnd)
	if wnd == nil then
		return nil
	end
	local coords = { x = wnd.x, y = wnd.y, w = wnd.w, h = wnd.h }	
	return coords
end

------------------------------------------------------------------------------------------------------------
-- restore window coordinates
function r2:restoreWndCoords(wnd, coords)	
	if wnd == nil or coords == nil then
		debugInfo("Not restauring coordinates, wnd = " .. tostring(wnd) .. ", coords = " .. tostring(coords))
		return
	end	
	wnd.x = coords.x
	wnd.y = coords.y
	wnd.w = coords.w
	wnd.h = coords.h
	wnd:invalidateCoords()
end

------------------------------------------------------------------------------------------------------------
-- clean content of a tree node
function r2:cleanTreeNode(tree, nodeId)
	local node = tree:getRootNode():getNodeFromId(nodeId)
	if not node or node.isNil then
		debugInfo("r2:cleanTreeNode : Can't find node " .. nodeId)
		return
	end
	while node:getNumChildren() ~= 0 do
		node:deleteChild(node:getChild(0))
	end		
end

------------------------------------------------------------------------------------------------------------
-- clean content of a tree node
function r2:cleanTreeRootNode(tree)
	local node = tree:getRootNode()
	if not node or node.isNil then
		debugInfo("r2:cleanTreeNode : Can't find node " .. nodeId)
		return
	end
	while node:getNumChildren() ~= 0 do
		node:deleteChild(node:getChild(0))
	end		
end

------------------------------------------------------------------------------------------------------------
-- make an instance and its sons blink
-- TODO nico : put elsewhere (not really ui, but 3D view)
function r2:blink(instance)	
	if type(instance) == "userdata" then
		local vd = instance.DisplayerVisual
		if vd ~= nil then
			vd:blink()
		end
		forEach(instance, function (k, v) self:blink(v) end)
		-- TODO : pairs do not called the redefined 'next' function
		--for k, v in pairs(instance) do
		--	self:blink(v)
		--end
	end
end

------------------------------------------------------------------------------------------------------------
-- clear the content of a menu
function r2:clearMenu(menu)
	local numLine = menu:getNumLine()
	for k = 1, numLine do
		menu:removeLine(0)
	end
end

------------------------------------------------------------------------------------------------------------
-- add a menu entry with an icon on the left
function r2:addMenuLine(menu, ucText, ah, ahParams, id, iconName, size)
	menu:addLine(ucText, ah, ahParams, id)
	if iconName ~= "" and iconName ~= nil then
		local menuButton = createGroupInstance("r2_menu_button", "", { bitmap = iconName, size = tostring(size)})
		if menuButton then
				menu:setUserGroupLeft(menu:getNumLine() - 1, menuButton)
		end
	end
end

------------------------------------------------------------------------------------------------------------
-- enclose a ui xml script with the necessary header and ending
function r2:encloseXmlScript(script)
	return [[ <interface_config>
		   <root id="interface" x="0" y="0" w="800" h="600" active="true" /> ]] .. script .. [[</interface_config>]]
end

------------------------------------------------------------------------------------------------------------
-- create interface for inspecting a lua table
-- TODO place this elsewhere because useful for other stuffs
function oldInspect(object, maxdepth, alreadySeen)

if (object == nil) then
	debugWarning("Cannot inspect a nil value")
	return
end

if alreadySeen == nil then
	alreadySeen = {}
end

if (maxdepth == nil) then maxdepth = 1 end


local x = 0
local y = 0
local w = 150
local h = 150

-- backup position
prevGroup = getUI("ui:interface:lua_inspector")
if not (prevGroup == nil) then	
	x = prevGroup.x
	y = prevGroup.y
	w = prevGroup.w
	h = prevGroup.h
end


-- window header
local script = 
[[
<interface_config>
<root id="interface" x="0" y="0" w="800" h="600" active="true" />
<group type="container" id="old_lua_inspector" w="150" title="uiLuaInspector" global_color="false" line_at_bottom="false"
	movable="true" active="true" opened="true" openable="false" resizer="true" header_color="UI:SAVE:WIN:COLORS:OPT"	
	pop_min_w="150" pop_min_h="150" pop_max_w="800" pop_max_h="600"	
>	
	<group id="content" x="0" y="0" sizeref="wh" w="0" h="0" posref="TL TL" >		
		<group id="sbtree" posref="TL TL" sizeref="wh" x="0" w="-14" y="-8" h="-16" >
			<!-- group of entity instances	-->
			<group id="tree_list" type="tree" posref="TL TL" x="14" y="0" col_over="255 255 255 48" col_select="255 255 255 80"
				max_sizeparent="parent" max_sizeref="wh" max_w="-10" max_h="0">											
]]

local id = 0

-- generate a new ID for a tree node
local function newId() id = id + 1; return tostring(id) end

-- return xml code to open a new tree node with the given content
local function nodeOpen(content, color, opened)	
	return [[<node id="]] .. newId() .. [[" name="]] .. content .. [[" color="]] .. color .. [[" global_color="false" opened="]] .. tostring(opened) .. [[" fontsize="15" y_decal="-1" >]] .. "\n"
end

-- return xml code to close a tree node
local function nodeClose()
	return "</node>\n"
end

-- color for each type
local typeColor =
{
	number = "255 255 255 255",
	string = "0 255 255 255",
	boolean = "255 255 0 255",
	thread = "128 128 128 255",
	table = "255 128 32 255",
	userdata = "255 128 64 255"
}

typeColor["function"] = "255 0 255 255"


-- return xml code for a leaf node
local function node(content, val)	
	--debugInfo(colorTag(255, 255, 0) .. "node")
	local color = typeColor[type(val)]
	if (color == nil) then color = "127 255 127 0" end
	local result =  [[<node id="]] .. newId() .. [[" name="]] .. content .. [[" global_color="false" color="]] .. color .. [[" opened="true" fontsize="15" y_decal="-1" />]] .. "\n"	
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
	number = function(key, val) return node(objectStr(key, val), val) end,
	string = function(key, val) return node(objectStr(key, val), val) end,
	boolean = function(key, val) return node(objectStr(key, val), val) end,	
	thread = function(key, val) return node(objectStr(key, "thread"), val) end,	
}

-- 'function' is a declared keyword, so must declare it this way
typeTable["function"] = function(key, val) return node(objectStr(key, "function"), val) end

-- recursive code to generate the code for a table
typeTable.table = function(key, val, depth)
	if (alreadySeen[val] == true) then -- avoid cyclic graph
		return node("!CYCLE!", "0 255 0 255")
	end
	alreadySeen[val] = true
	-- create a temp table sorted by type
	local sortedTable = {}
	local index = 1
	forEach(val, function (k, v) sortedTable[index] = { key = k, value = v }; index = index + 1 end)	
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
	local content = nodeOpen(tostring(key) .. "(key type = " .. type(key) .. ")", typeColor[type(val)], depth < maxdepth);
	local function tableElement(key, value)
		if (typeTable[type(value.value)] == nil) then
			content = content .. node("?")
			return
		end
		-- add node for a field of the table
		content = content .. typeTable[type(value.value)](value.key, value.value, depth + 1)
	end
	forEach(sortedTable, tableElement)
	return content .. nodeClose()
end

typeTable.userdata = typeTable.table


-- generate the tree
script = script .. typeTable[type(object)]("#object#", object, 0)



-- window end
script = script .. 
[[</group>						
			<ctrl style='skin_scroll' id='scroll_bar' align='T' target='tree_list' />							
		</group>		
	</group>
</group>

<tree node="old_lua_inspector">
</tree>
</interface_config>
]] .. "\n"

--for w in string.gfind(script, "node(.*)") do
--	debugInfo(w)
--end
--debugInfo(script)

if true then	
	parseInterfaceFromString(script)

	-- restore position
	newGroup = getUI("ui:interface:old_lua_inspector")
	if not (newGroup == nil) then
		newGroup.x = x
		newGroup.y = y
		newGroup.w = w
		newGroup.h = h
	end


	updateAllLocalisedElements()
end

end

------------------------------------------------------------------------------------------------------------
-- inspect current instance content
function r2:inspectCurrInstance()
debugInfo("Inspect current instance")
local instanceTable = r2:getSelectedInstance()
if (instanceTable == nil) then
	debugWarning("Can't found instance")
	--runCommand("luaObject","r2.instances")
	return;
end
inspect(instanceTable, 4, { instanceTable.parent })
end
