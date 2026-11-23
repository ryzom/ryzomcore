-- Calls from the interface.
--
-- A menu line, a tree node or a button gives its action to the "lua" action handler as Lua source text, which is
-- compiled when the player clicks. Nothing of a scenario, of a file or of another player may be part of such a text:
-- instance ids, names and file names come from the other editors of the scenario and from scenario files, and a value
-- that closes the string it was put in would be run as code.
--
-- r2:uiCall(target, ...) is what such an action is made with. It keeps the name of the function and its arguments in
-- a table, and returns the source of a call that names them by a number, "r2:runUICall(12)". The arguments are given
-- to the function as values when the call is run; they are never compiled.
--
--   menu:addLine(dialog.Name, "lua", r2:uiCall("r2:setSelectedInstanceId", dialog.InstanceId), dialog.InstanceId)
--
-- The target is the path of a global function as a call writes it, "r2.events:setLogicEntity" or
-- "r2_core.setCurrSelectedFile": names joined by dots, with a colon before the last one for a method. It is a name
-- from the code of the editor, never a value, and it is looked up name by name when the call is run, not compiled.
-- The arguments are strings, numbers, booleans or nil, at most four; another value is given as its text.
--
-- The function gets exactly the arguments the call was made with, up to the last one that is not nil: a function of
-- the client that counts its arguments gets the count it expects.
--
-- The same call asked for again gets the same number, so a menu that is built each time it opens does not grow the
-- table. A number is never given to another call. The table is a global that outlives r2, as the caches of the
-- property sheets and of the forms do (r2ClassDescCache, r2FormsCache): r2 is made anew each time the editor starts,
-- while a window kept from before still holds the numbers of its calls.
--
-- r2:callByPath(target, ...) runs such a target at once, for the code that has the name of a function in a class
-- definition and used to compile a call of it.

if r2UICalls == nil then
	r2UICalls =
	{
		Calls = {},			-- number -> { Target = , N = , [1..N] = arguments }
		Numbers = {},		-- key of a call -> its number
		Count = 0,
	}
end

r2.UICalls = r2UICalls
r2.UICalls.MaxArgs = 4
-- the tables a target may start from
r2.UICalls.Roots = { r2 = true, r2_core = true }

-- unpack of Lua 5.1, table.unpack of the later ones
local unpackArgs = unpack or table.unpack

------------------------------------------------------------------------------------------------------------
-- The names of a target: the path as a list of names, and the name of the method or nil.
-- nil when the target is not names joined by dots with at most one colon before the last name.
local function splitTarget(target)
	if type(target) ~= "string" then return nil end
	local names = {}
	local method = nil
	local start = 1
	local len = string.len(target)
	local colonSeen = false
	local i = 1
	while i <= len + 1 do
		local c = string.sub(target, i, i)
		if i == len + 1 or c == "." or c == ":" then
			local name = string.sub(target, start, i - 1)
			if string.find(name, "^[%a_][%w_]*$") == nil then return nil end
			if colonSeen then
				-- the name after the colon is the last one
				if i ~= len + 1 then return nil end
				method = name
			else
				table.insert(names, name)
			end
			if c == ":" then colonSeen = true end
			start = i + 1
		elseif string.find(c, "^[%w_]$") == nil then
			return nil
		end
		i = i + 1
	end
	if colonSeen and method == nil then return nil end
	return names, method
end

------------------------------------------------------------------------------------------------------------
-- The function of a target, and the object it is a method of (nil for a plain function).
-- An error when the target is not a path, does not start from one of the roots, or names no function.
local function resolveTarget(target)
	local names, method = splitTarget(target)
	if names == nil then
		error("r2 interface call: '" .. tostring(target) .. "' is not the path of a function")
	end
	if not r2.UICalls.Roots[names[1]] then
		error("r2 interface call: '" .. target .. "' does not start from r2 or r2_core")
	end
	local value = _G
	for k = 1, table.getn(names) do
		if type(value) ~= "table" and type(value) ~= "userdata" then
			error("r2 interface call: '" .. target .. "' has nothing named " .. names[k])
		end
		value = value[names[k]]
		if value == nil then
			error("r2 interface call: '" .. target .. "' has nothing named " .. names[k])
		end
	end
	if method ~= nil then
		if type(value) ~= "table" and type(value) ~= "userdata" then
			error("r2 interface call: '" .. target .. "' has no method " .. method)
		end
		local fn = value[method]
		if type(fn) ~= "function" then
			error("r2 interface call: '" .. target .. "' has no method " .. method)
		end
		return fn, value
	end
	if type(value) ~= "function" then
		error("r2 interface call: '" .. target .. "' is not a function")
	end
	return value, nil
end

------------------------------------------------------------------------------------------------------------
-- An argument as it is kept: strings, numbers, booleans and nil as they are, another value as its text
-- (an instance id or a translated text given as an object used to be put into the source as its text too)
local function keptArgument(value)
	local t = type(value)
	if t == "string" or t == "number" or t == "boolean" or t == "nil" then return value end
	return tostring(value)
end

------------------------------------------------------------------------------------------------------------
-- The key of a call: the target and each argument with its type and its length, so that no two calls share one.
-- A number is written with all its digits (tostring rounds it), and as an integer or a float where Lua has both.
local function callKey(target, args, n)
	local key = target
	for k = 1, n do
		local v = args[k]
		local t = type(v)
		local text = ""
		if t == "number" then
			text = string.format("%.17g", v)
			if math.type ~= nil then text = math.type(v) .. " " .. text end
		elseif t ~= "nil" then
			text = tostring(v)
		end
		key = key .. "\n" .. string.sub(t, 1, 2) .. tostring(string.len(text)) .. ":" .. text
	end
	return key
end

------------------------------------------------------------------------------------------------------------
-- The source of a call of 'target' with these arguments, for the parameters of a "lua" action handler.
-- See the head of this file.
function r2:uiCall(target, a1, a2, a3, a4, tooMany)
	if tooMany ~= nil then
		error("r2:uiCall: '" .. tostring(target) .. "' is given more than " .. tostring(r2.UICalls.MaxArgs) .. " arguments")
	end
	local names = splitTarget(target)
	if names == nil then
		error("r2:uiCall: '" .. tostring(target) .. "' is not the path of a function")
	end
	if not r2.UICalls.Roots[names[1]] then
		error("r2:uiCall: '" .. target .. "' does not start from r2 or r2_core")
	end
	local args = { keptArgument(a1), keptArgument(a2), keptArgument(a3), keptArgument(a4) }
	-- the arguments up to the last one that is not nil
	local n = 0
	for k = 1, r2.UICalls.MaxArgs do
		if args[k] ~= nil then n = k end
	end
	local calls = r2.UICalls
	local key = callKey(target, args, n)
	local number = calls.Numbers[key]
	if number == nil then
		calls.Count = calls.Count + 1
		number = calls.Count
		calls.Numbers[key] = number
		calls.Calls[number] = { Target = target, N = n, args[1], args[2], args[3], args[4] }
	end
	return "r2:runUICall(" .. tostring(number) .. ")"
end

------------------------------------------------------------------------------------------------------------
-- Run the call that r2:uiCall gave this number to. What the function returns is returned.
function r2:runUICall(number)
	local call = r2.UICalls.Calls[number]
	if call == nil then
		error("r2:runUICall: there is no call " .. tostring(number))
	end
	local fn, object = resolveTarget(call.Target)
	if object ~= nil then
		return fn(object, unpackArgs(call, 1, call.N))
	end
	return fn(unpackArgs(call, 1, call.N))
end

------------------------------------------------------------------------------------------------------------
-- Run the function of a target at once with these arguments. What the function returns is returned.
function r2:callByPath(target, ...)
	local fn, object = resolveTarget(target)
	if object ~= nil then
		return fn(object, ...)
	end
	return fn(...)
end
