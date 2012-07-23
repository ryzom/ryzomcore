-- console & printing related functions

---------------
-- FUNCTIONS --
---------------

------------------------------------------------------------------------------------------------------------
-- alias for 'debugInfo'
log = debugInfo	


------------------------------------------------------------------------------------------------------------
-- Build color tag for use with debugInfo
function colorTag(r, g, b, a)	
	local function compToLetter(comp)
		if comp == nil then
			return 'F'
		end
		comp = math.floor(clamp(comp, 0, 255) / 16) + 1
		--debugInfo("Comp = " .. tostring(comp))
		return ComponentToLetter[comp]
	end			
	return "@{" .. compToLetter(r) .. compToLetter(g) .. compToLetter(b) .. compToLetter(a) .. "}"
end

-------------------------------------------------------------------------------------------------
-- Display a string, splitting it when too long 
function dumpSplittedString(str)	
	local splitSize = 50
	local numParts = math.floor(string.len(str) / splitSize) + 1	
	for i = 0, numParts do		
		debugInfo(string.sub(str, i * splitSize, i * splitSize + splitSize - 1))
	end	
end

-------------------------------------------------------------------------------------------------
-- display debug info with warning color
function debugWarning(msg)
	debugInfo(warningTag .. msg)
end

-------------------------------------------------------------------------------------------------
-- dump content of a lua object
function luaObject(obj, maxDepth)		
	dumpCallStack(0)
	if runCommand == nil
	then
		r2.print(obj)
	else
		__tmpInstance = obj
		runCommand("luaObject", "__tmpInstance", select(maxDepth, maxDepth, 10))
		__tmpInstance = nil
	end
end

-------------------------------------------------------------------------------------------------
-- dump content of a lua object (other version)
-- output : object with a "write" function that display the result. if nil, 'output' default to the global 'io'

function writeTable(t, output)	
	if output == nil then output = io end
	function writeSpace(n)
		for i = 1, n do
			output.write("\t")
		end
	end

	function writeTableR(t, n)
		if (type(t) == "table")
		then

			output.write("{\n")
			for key, value in pairs(t) do
				if ( type(value) == "table")
				then
					writeSpace(n+1)
					output.write (key)
					output.write ("=")
					writeTableR(value, n+1)
					
				elseif (type(value) == "string")
				then
					value = "\"" ..value .. "\""
					writeSpace(n+1)
					output.write(key, "=", value,", \n")
				elseif (type(value) == "number")
				then
					writeSpace(n+1)
					output.write(key, "=", value,", \n")
				end
			end
			writeSpace(n)
			output.write("},\n");
		end
	end
	
	writeTableR(t, 0)
end


--function loadTable(fileName)

--	local file = io.open(fileName, "r")

--	function loadTableR(file)

--		local line
--		while (line=file:read("*l")) ~= "}" then

--			if line == "{" then
--				loadTableR(file)
--			else

--			end
--		end
--	end

--	if file:read("*l") ~= "{" then
--		debugInfo("file doesn't store a valid table")
--		return
--	end

--	local resultT = loadTableR(file)
--	io.close(file)
--	return resultT
--end


-------------
-- STATICS --
-------------


ComponentToLetter = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' }
warningTag		 = colorTag(255, 127, 0)

----------
-- INIT --
----------


-- for vianney's tests : if debugInfo function wasn't registered externally, then use the standard 'print' instead
if debugInfo == nil then
	debugInfo = r2.print
else
	function print(a0, a1, a2, a3, a4, a5)		
		local result = ""
		if a0 ~= nil then result = result .. tostring(a0) end
		if a1 ~= nil then result = result .. "  " ..tostring(a1) end
		if a2 ~= nil then result = result .. "  " ..tostring(a2) end
		if a3 ~= nil then result = result .. "  " ..tostring(a3) end
		if a4 ~= nil then result = result .. "  " ..tostring(a4) end
		if a5 ~= nil then result = result .. "  " ..tostring(a5) end
		if result ~= nil then debugInfo(result) end
	end
end


function gotoFile(name, line)
	local path = fileLookup(name)
	if path ~= "" then	
		local luaEditorPath = os.getenv("R2ED_LUA_EDITOR_PATH")
		if luaEditorPath == nil then
			debugInfo([[ Can't launch editor to edit lua file, please set ]] ..
			          [[ the environment variable R2ED_LUA_EDITOR_PATH    ]] ..
					  [[ with the path of your editor. ]])

		else
			launchProgram(luaEditorPath, path .. "/" .. tostring(line))
		end
	end
end

