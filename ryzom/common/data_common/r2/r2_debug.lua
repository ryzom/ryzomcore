-- debugging aid

---------------
-- FUNCTIONS --
---------------


------------------------------------------------------------------------------------------------------------
-- function tracing asked in config file ?
-- if traceFunctions == true  then 
-- 	local function traceFunction(what)
-- 		debug.sethook()
--		local di = debug.getinfo(2)
--		--luaObject(di)
--		if di.name ~= nil and di.name ~= 'getmetatable' then
--			debugInfo(what .. " : " .. tostring(di.name) .. " : " .. tostring(di.short_src) .. " : " .. tostring(di.currentline))
--		end
--		debug.sethook(traceFunction, "cr")
--	end
--	debugInfo("turning debug hook on")
--	debug.sethook(traceFunction, "c")
--else
--	--debugInfo("turning debug hook off")
--	debug.sethook()
--end

------------------------------------------------------------------------------------------------------------
-- dump objects cycles for the given object
function dumpCycles(base, visitedSet, infos)
	local function fullName(infos)
		local result = ""
		while infos do
			result = infos.Name .. "." .. result
			infos = infos.Parent
		end
		return result
	end
	if visitedSet == nil then
		visitedSet = {}
	end
	if infos == nil then
		infos = { Name = "root", Parent = nil }
	end
	for k, v in pairs(base) do
		if v ~= _G._G then
			if type(v) == "table" then
				local newInfos =  { Name = tostring(k), Parent = infos }
				if visitedSet[v] then
					debugInfo(fullName(visitedSet[v]) .. "is referenced by " .. fullName(newInfos))
				else				
					visitedSet[v] = newInfos -- mark as visited
					dumpCycles(v, visitedSet, newInfos)
					visitedSet[v] = nil -- only intersted in cycles
				end	
			end
		end
	end
end

------------------------------------------------------------------------------------------------------------
-- display time taken to execute a function
function profileFunction(func, name)	
	assert(type(func) == "function")	
	if name == nil then
		name = debug.getinfo(func).name	
	end	
	local startTime = nltime.getPreciseLocalTime()
	func()
	local endTime = nltime.getPreciseLocalTime()
	--debugInfo(string.format("time for %s is %d", tostring(name), endTime - startTime))
end

-- display time taken to execute a function
function profileMethod(table, funcName, name)
	assert(table)
	assert(type(funcName) == "string")
	assert(type(table[funcName]) == "function")	
	if name == nil then
		name = select(debug.getinfo(table[funcName]).name, funcName)
	end
	local startTime = nltime.getLocalTime()
	table[funcName](table)
	local endTime = nltime.getLocalTime()
	debugInfo(string.format("time for %s is %f", tostring(name), (endTime - startTime) / 1000))
end

------------------------------------------------------------------------------------------------------------
-- add a break that is triggered when a value in a table has been changed
function addBreakOnChange(table, watchedKey)
	assert(type(table) == "table")	
	local value = table[watchedKey]
	assert(value ~= nil)
	table[watchedKey] = nil
	debugInfo("Adding break on change of key " .. tostring(watchedKey))
	local mt = getmetatable(table)
	local oldNewIndex
	if mt then
		oldNewIndex = mt.__newindex
	end
	local newMT
	if mt then newMT = clone(mt) else newMT = {} end
	-- WRITE
	newMT.__newindex = function(table, key, newValue)
		debugInfo('write')
		if key == watchedKey then
			value = newValue
			debugInfo(debug.traceback())
			debugWarning("<addBreakOnChange> Key " .. tostring(watchedKey) .. " changed to "	.. tostring(value))			
			assert(nil)
		elseif mt and mt.__newindex then
			mt.__newindex(table, key, newValue)
		else
			rawset(table, key, newValue)
		end		
	end
	-- READ
	newMT.__index = function(table, key)		
		debugInfo('read')
		if key == watchedKey then
			return value		
		elseif mt and mt.__index then
			return mt.__index(table, key, value)
		else
			return rawget(table, key)
		end		
	end
	--
	setmetatable(table, newMT)
end	 

----------
-- INIT --
----------


-- replace the assert function with a more verbose one
if oldAssert == nil then
	oldAssert = assert
end

function assert(cond)
	if not cond then
		-- rawDebugInfo(colorTag(255, 0, 255) .. "ASSERTION FAILED !! ")
		rawDebugInfo("@{FOFF}ASSERTION FAILED !! ")
		dumpCallStack(2);
		error("")
	end
end


local gcStartTime

-- special nico stuff : modified lua version calls "__notify_debug" when a garbage collection cycle occurs
function __notify_gc()	
	gcStartTime = nltime.getLocalTime()		
end	
function __notify_post_gc()
	local deltaTime = nltime.getLocalTime() - gcStartTime
	local used, threshold = gcinfo()
	debugInfo(colorTag(255, 0, 0) .. string.format("** GC ** (%d ms) (used = %d kb, threshold = %d kb", deltaTime, used, threshold))
end


--testTable = { tata = 1, toto = 2 }

--addBreakOnChange(testTable, "tata")

--testTable.tutu = 2
--testTable.tata = testTable.tata + 20

