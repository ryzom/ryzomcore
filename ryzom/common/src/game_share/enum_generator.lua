--[[ @file enum_generator.lua

This script can be used to generate an enum in game_share.
Minimal usage:
	require("enum_generator")
	enum_generator.build("name", { "VAL" })

Name parameter can be a table of strings, in which case each string will be
treated as a word and concatenated in various ways. Example:
	enum_generator.build({ "composed", "name" }, { "VAL" })

The name of the generated file will be composed_name.h and composed_name.cpp,
and thus your lua script should be composed_name.lua.

Default templates for h and cpp files are respectively in
enum_generator_template_h.lua and enum_generator_template_cpp.lua. You should
only modify these templates in a generic way to avoid breaking existing
generated enums. If you need specific templates you can provide your own to
the build method. Example:
	enum_generator.build(name, enum, "int %name1%[];", "int %name1%[] = { %enum1% };" )

--]]

enum_generator = {}

local function format_name(name, sep, formater)
	if type(name)=="string" then
		return formater(name)
	elseif type(name)=="table" and table.getn(name)>0 then
		local str = formater(name[1])
		for i=2, table.getn(name) do
			str = str..sep..formater(name[i])
		end
		return str
	else
		return nil
	end
end

-- this one take a formater for first word and another for following words
local function format_name2(name, sep, formater1, formater2)
	if type(name)=="string" then
		return formater1(name)
	elseif type(name)=="table" and table.getn(name)>0 then
		local str = formater1(name[1])
		for i=2, table.getn(name) do
			str = str..sep..formater2(name[i])
		end
		return str
	else
		return nil
	end
end

local word_formater = {}
function word_formater.lower(word)
	return string.lower(word)
end
function word_formater.upper(word)
	return string.upper(word)
end
function word_formater.initial(word)
	local head = string.sub(word, 1, 1)
	local tail = string.sub(word, 2)
	return string.upper(head)..string.lower(tail)
end

local name_formater = {}
name_formater["name1"] = function(name)
	return format_name(name, "_", word_formater.lower)
end
name_formater["name2"] = function(name)
	return format_name(name, "_", word_formater.upper)
end
name_formater["name3"] = function(name)
	return format_name(name, "", word_formater.initial)
end
name_formater["name4"] = function(name)
	return format_name(name, " ", word_formater.lower)
end
name_formater["name5"] = function(name)
	return format_name2(name, "", word_formater.lower, word_formater.initial)
end

local enum_formater = {}
enum_formater["enum1"] = function(enum)
	local str = ""
	for i,val in pairs(enum) do
		str = str..string.gsub("\t\t<val>,", "<val>", val).."\n"
	end
	return str
end
enum_formater["enum2"] = function(enum)
	local str = ""
	for i,val in pairs(enum) do
		str = str..string.gsub("\t\t { \"<val>\", <val> },", "<val>", val).."\n"
	end
	return str
end

require("enum_generator_template_h")
require("enum_generator_template_cpp")

function enum_generator.build(name, enum, template_h, template_cpp)

	local files = {}
	files[".h"] = template_h or enum_generator.template_h
	files[".cpp"] = template_cpp or enum_generator.template_cpp

	local names = {}
	for k,formater in pairs(name_formater) do
		names[k] = formater(name)
	end

	local enums = {}
	for k,formater in pairs(enum_formater) do
		enums[k] = formater(enum)
	end

	for ext,str in pairs(files) do
		for k,v in pairs(names) do
			str = string.gsub(str, "%%"..k.."%%", v)
		end
		for k,v in pairs(enums) do
			str = string.gsub(str, "%%"..k.."%%", v)
		end
		local file = assert(io.open(names["name1"]..ext, "w"))
		file:write(str)
		file:close()
	end
end


