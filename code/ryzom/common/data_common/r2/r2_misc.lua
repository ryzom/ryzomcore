-- This file contains a set of miscellanous functions that don't fit in any other place
-- TODO nico : could be useful outside r2 -> export this


---------------
-- FUNCTIONS --
---------------


------------------------------------------------------------------------------------------------------------
-- equivalent of the ? :  C operator, execpt that the 2 sides are evaluated before assignement
function select(cond, valueForTrue,  valueForFalse)
	if cond then
		return valueForTrue
	else
		return valueForFalse
	end
end


------------------------------------------------------------------------------------------------------------
-- execute a function for each key pair in a table
function forEach(table, fn)	
	local i, v = next(table,nil)   -- get first index of "o" and its value
    while i do
       fn(i, v)	   
       i, v = next(table,i)         -- get next index and its value	   
    end
end


------------------------------------------------------------------------------------------------------------
-- whatever
table.setn = function(table, n)
	assert(table)
	local mt = getmetatable(table)
	if mt ~= nil then
		if mt.__next ~= nil then
			table.Size = n
		end
	end
end

------------------------------------------------------------------------------------------------------------
-- extension to table library : remove all content of a table without deleting the table object
function table.clear(tbl)
	while next(tbl) do
		tbl[next(tbl)] = nil
	end
	table.setn(tbl, 0)
end

------------------------------------------------------------------------------------------------------------
-- extension to table library : merge the content of two table remove the element, remove fields with  duplicated keys (except for number)
function table.merge(tbl1, tbl2)
	local k, v = next(tbl2)
	while k do
		if (type(k) == "number") then 
			table.insert(tbl1, v)
		else
			tbl1[k] = v
		end
		k, v = next(tbl2, k)		
	end
end

------------------------------------------------------------------------------------------------------------
-- Addtion to the string library : test wether a string match with the given pattern (returns true is so)
function string.match(str, pattern)
	assert( type(str) == "string")
	if (str == nil) then
		debugInfo(debug.traceback())
		assert(0)
	end
	local startPos, endPos = string.find(str, pattern)
	if startPos == nil then return false end	
	return startPos == 1 and endPos == string.len(str)
end

------------------------------------------------------------------------------------------------------------
-- clone content of a table
function clone(t)
	local new = {}
	local i, v = next(t, nil)
	while i do
		if (type(v)=="table") then v= clone(v) end
		new[i] = v
		i, v = next(t, i)
	end
	return new
end

------------------------------------------------------------------------------------------------------------
-- Test if 2 values are equal
-- If values are table, then a member wise comparison is done
function isEqual(lhs, rhs)
	if type(lhs) ~= type(rhs) then return false end
	if type(lhs) == "table" then
		local lk, lv = next(lhs) -- keys
		local rk, rv = next(rhs) -- values
		while lk and rk do			
			if not isEqual(lk, rk) then
				return false
			end
			if not isEqual(lv, rv) then
				return false
			end
			lk, lv = next(lhs, lk)
			rk, rv = next(rhs, rk)
		end
		if lk ~= nil or rk ~= nil then
			return false
			-- not same table length
		end
		return true
	else
		return lhs == rhs
	end
end

------------------------------------------------------------------------------------------------------------
-- Test if 2 values are equal
-- If values are table, then a member wise comparison is done
-- special : function pointer are ignored and considered equals
function isEqualIgnoreFunctions(lhs, rhs)
	if type(lhs) ~= type(rhs) then return false end
	if type(lhs) == "table" then
		local lk, lv = next(lhs) -- keys
		local rk, rv = next(rhs) -- values
		while lk and rk do			
			if not isEqualIgnoreFunctions(lk, rk) then
				return false
			end
			if not isEqualIgnoreFunctions(lv, rv) then
				return false
			end
			lk, lv = next(lhs, lk)
			rk, rv = next(rhs, rk)
		end
		if lk ~= nil or rk ~= nil then
			return false
			-- not same table length
		end
		return true
	elseif type(lhs) == "function" then 
		return true
	else
		return lhs == rhs
	end
end


------------------------------------------------------------------------------------------------------------
-- clone of a table, but with a depth of 1 ...
function shallowClone(t)
	local new = {}
	local i, v = next(t, nil)
	while i do		
		new[i] = v
		i, v = next(t, i)
	end
	return new
end

-------------------------------------------------------------------------------------------------
-- If args 'value' is nil then the arg 'default' is returned, else the actual 'value' is return
function defaulting(value, default)
	if value == nil then 
		return default
	else
		return value
	end
end


-------------------------------------------------------------------------------------------------
-- return clamped value. Min and/or max are ignotred if null
function clamp(value, min, max)
	local result = value
	if min then  result = math.max(min, result) end
	if max then  result = math.min(max, result) end
	return result
end	
		
-------------------------------------------------------------------------------------------------
-- enclose a string by double quotes
function strify(str) 
	return [["]] .. tostring(str) .. [["]]
end	
		
-------------------------------------------------------------------------------------------------
-- enclose a string by double quotes
function strifyXml(str)
	local strxml = string.gsub(tostring(str), ">", "&gt;")
	strxml = string.gsub(strxml, "<", "&lt;")
	strxml = string.gsub(strxml, "&", "&amp;")
	strxml = string.gsub(strxml, "'", "&apos;")
	strxml = string.gsub(strxml, '"', "&quot;")
	return [["]] .. strxml .. [["]]
end	

------------------------------------------------------------------------------------------------------------
-- snap a position to ground, returning the z snapped coordinate
function r2:snapZToGround(x, y)
	local x1, y1, z1 = r2:snapPosToGround(x, y)
	return z1
end

-------------------------------------------------------------------------------------------------
--
--built an ordered table from a table whose index are strings
--example :
--
--table = 
--{
--"bar" = test(),
--"foo" = { "hello" },
--"abc" = 10,
--}
--
--result = sortAlphabeticaly(table)
--
--
--result is an integer indexed table :
--{
--    -- index = { sorted key, value }
--    1 = { "abc", 10 },
--    2 = { "bar", test() },
--    3 = { "foo", { "hello" } }
--}
--

function sortAlphabetically(src)
	local sortedTable = {}
	local index = 1
	for k, v in pairs(src) do
		sortedTable[index] = { key = k, value = v }
		index = index + 1 
	end	
	local function comp(val1, val2)		
		return val1.key < val2.key
	end
	table.sort(sortedTable, comp)
	return sortedTable
end

----------
-- INIT --
----------

-- redefine the 'next' function of lua to use a "__next" function in the metatable
-- useful to traverse C++ objects that are exposed to lua through the use of the metatable 
assert(next ~= nil) -- default lib should have been opened

if oldNextFunction == nil then
	oldNextFunction = next
end
next = function(table, key)
	assert(table)
	local mt = getmetatable(table)
	if mt ~= nil then
		if mt.__next ~= nil then
			return mt.__next(table, key)
		end
	end
	-- tmp	
	--if type(table) ~= "table" then
	--	debugInfo(debug.traceback())
	--	debugInfo("'next' expect a table (or user data with __next metamethod) as its first parameter")
	--	return
	--end
	-- else use default 'next' function
	return oldNextFunction(table, key)
end



-- assert(table.getn ~= nil) -- default lib should have been opened

--if oldTableGetnFunction == nil then
--	oldTableGetnFunction = table.getn
--end
--
--table.getn = function(table)
--	assert(table)
--	local mt = getmetatable(table)
--	if mt ~= nil then
--		if mt.__next ~= nil then
--			return table.Size 
--		end
--	end
--	return oldTableGetnFunction(table)
--end


table.getn = function(table)
	assert(table)
	local mt = getmetatable(table)
	if mt ~= nil then
		if mt.__next ~= nil then
			return table.Size
		end
	end
	return #table
end



-- redefine the hardcoded 'pairs' function to use the redefined 'next'
-- hardcoded version uses the C version of next, not the lua one if it has been redefined

if oldPairsFunction ~= nil then
	pairs = oldPairsFunction
end

if oldPairsFunction == nil then
	oldPairsFunction = pairs
end

if true then
	-- TODO nico : bad init of editor if I name this 'pairs' directly (don't know why), so named it 'specPairs' and used
	-- 'specPairs' when I must iterate over C++ objects ...
	specPairs = function(table)
		local function iterFunc(table, key)
			return next(table, key)
		end
		return iterFunc, table
	end
end

function r2.assert (param)
	if not param then assert(nil) end
	return param
end

function r2.isTable(node)
	if not node then return false end

	if type(node) == "table" then
		return true
	elseif type(node) == "userdata" then
		local mt = getmetatable(node)
		if mt~= nil and mt.__next ~= nil then
			return true
		end			
	end
	return false
end
