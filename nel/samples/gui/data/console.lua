-- Lua console / UI inspector for the NeL GUI showcase.
--
-- A REPL over the live interface: everything the reflection system exports
-- can be read and written from here while the sample runs. The log is a
-- fixed stack of plain text views repainted from a history table; the input
-- is a stock edit box whose onenter handler evaluates the line. No C++.

console = {}

console.WINDOW = "ui:sample:console"
console.PREFIX = console.WINDOW .. ":content:"
console.LINES = 8
console.KEEP = 200 -- history entries kept for scrollback via repaint window

console.C_ECHO = "170 200 255 255"
console.C_RESULT = "255 255 255 255"
console.C_ERROR = "255 110 110 255"
console.C_HINT = "255 255 255 140"

console.history = {}

-- The stock 'next' only walks tables; route it through the __next
-- metamethod so pairs() also enumerates reflected widgets (children
-- groups/views/ctrls first, then every exported property).
if oldNextFunction == nil then oldNextFunction = next end
next = function(t, k)
	local mt = getmetatable(t)
	if mt ~= nil and mt.__next ~= nil then return mt.__next(t, k) end
	return oldNextFunction(t, k)
end
if oldPairsFunction == nil then oldPairsFunction = pairs end
pairs = function(t)
	return next, t
end

function console.repaint()
	local n = #console.history
	local first = math.max(0, n - console.LINES)
	for i = 0, console.LINES - 1 do
		local line = getUI(console.PREFIX .. "log:l" .. i)
		local entry = console.history[first + i + 1]
		if entry then
			line.hardtext = entry.text
			line.color = entry.color
		else
			line.hardtext = ""
		end
	end
end

function console.log(text, color)
	for s in string.gmatch(tostring(text) .. "\n", "([^\n]*)\n") do
		table.insert(console.history, { text = s, color = color or console.C_RESULT })
	end
	while #console.history > console.KEEP do
		table.remove(console.history, 1)
	end
	console.repaint()
end

rawprint = rawprint or print
function print(...)
	local parts = {}
	for i = 1, select('#', ...) do
		parts[i] = tostring(select(i, ...))
	end
	console.log(table.concat(parts, "  "))
end

-- Widget lookup with the sample master group as default scope
function ui(path)
	if type(path) ~= "string" then return path end
	if string.sub(path, 1, 3) ~= "ui:" then path = "ui:sample:" .. path end
	return getUI(path)
end

-- List the children of a group (pairs() yields _Group/_View/_Ctrl keys for
-- children and plain names for reflected properties)
function ls(x)
	local e = ui(x)
	if not e then
		console.log("ls: element not found", console.C_ERROR)
		return
	end
	console.log(e.id, console.C_ECHO)
	local buckets = { _Group = {}, _View = {}, _Ctrl = {} }
	for k, v in pairs(e) do
		local kind = string.match(tostring(k), "^(_%a+) ")
		if kind and buckets[kind] then
			local id = v.id
			table.insert(buckets[kind], string.match(id, "[^:]+$") or id)
		end
	end
	local none = true
	for label, bucket in oldPairsFunction({ groups = buckets._Group, views = buckets._View, ctrls = buckets._Ctrl }) do
		if #bucket > 0 then
			console.log("  " .. label .. ": " .. table.concat(bucket, " "))
			none = false
		end
	end
	if none then console.log("  (no children)") end
end

-- Dump the reflected properties of any widget
function props(x)
	local e = ui(x)
	if not e then
		console.log("props: element not found", console.C_ERROR)
		return
	end
	console.log(e.id, console.C_ECHO)
	local line = ""
	for k, v in pairs(e) do
		local key = tostring(k)
		if string.sub(key, 1, 1) ~= "_" then
			local tv = type(v)
			local sv
			if tv == "userdata" or tv == "table" or tv == "function" then
				sv = "<" .. tv .. ">"
			else
				sv = tostring(v)
			end
			local item = key .. "=" .. sv
			if line == "" then
				line = "  " .. item
			elseif string.len(line) + string.len(item) + 2 <= 96 then
				line = line .. "  " .. item
			else
				console.log(line)
				line = "  " .. item
			end
		end
	end
	if line ~= "" then console.log(line) end
end

function console.enter()
	local eb = getUI(console.PREFIX .. "input:eb")
	local cmd = eb.input_string
	if cmd == "" then return end
	eb.input_string = ""
	console.log("> " .. cmd, console.C_ECHO)
	-- expression first so plain '1+2' prints its value, statement otherwise
	local chunk, err = loadstring("return " .. cmd, "console")
	if not chunk then
		chunk, err = loadstring(cmd, "console")
	end
	if not chunk then
		console.log(err, console.C_ERROR)
		return
	end
	local res = { pcall(chunk) }
	if not res[1] then
		console.log(tostring(res[2]), console.C_ERROR)
		return
	end
	if #res > 1 then
		local parts = {}
		for i = 2, #res do
			parts[i - 1] = tostring(res[i])
		end
		console.log(table.concat(parts, "  "))
	end
end

console.log("Lua 5.1 REPL over the live widget tree. Try:", console.C_HINT)
console.log("  ui('minesweeper:content:mines').color = '150 255 150 255'", console.C_HINT)
console.log("  ls('console')   props('console:content:input:eb')   math.pi * 2", console.C_HINT)
