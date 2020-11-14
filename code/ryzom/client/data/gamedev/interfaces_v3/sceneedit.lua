--- Parse interface of ark_scene_editor_edit_menu ---
local script = [[<interface_config>
<root id="interface" x="0" y="0" w="800" h="600" active="true"/>
<group type="menu" id="ark_scene_editor_edit_menu" extends="base_menu" mouse_pos="true">
</group>
</interface_config>]]
parseInterfaceFromString(script)


if SceneEditor == nil then
	SceneEditor = {
		Shapes = {},
		Groups = {},
		LastEditedGroup = nil,
		HaveUpdate = nil
		
	};
end


function debug(text)
		local message  = ucstring()
		message:fromUtf8(tostring(text))
		displaySystemInfo(message, "SYS")
end


function SceneEditor:init(scene_id, form_url, translations, icons_url)
	self.sceneId = scene_id
	self.baseUrl = form_url
	self.iconsUrl = icons_url
	self.T = translations
end

function SceneEditor:reset(no_get_html)
	self.Shapes = {}
	self.Groups = {}
	self.LastEditedGroup = nil
	self.HaveUpdate = nil
	runAH(nil, "remove_shapes", "")
	if no_get_html == true then
		self:get_html("Reseted")
	end
end

function SceneEditor:show_menu()
	if (rightClick) then
		SceneEditor:launch_menu()
	end
end


function SceneEditor:launch_menu(id)
	-- SelectedInstanceId can be set by client application
	if id ~= nil then
		SelectedInstanceId = id
	end
	local menu = getUI("ui:interface:ark_scene_editor_edit_menu")
	menu:setMinW(85)
	menu:updateCoords()
	menu = menu:getRootMenu()
	menu:reset()
	menu:addLine(ucstring("-- SHAPE EDITION --"), "", "", "shape_header")
	menu:addLine(ucstring("Move"), "", "", "shape_move")
	menu:addSubMenu(1)
	local subMenu = menu:getSubMenu(1)
	subMenu:addIconLine(ucstring("Axe X"), "lua", "setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:move_x()')", "shape_move_x", "ark_move_x.tga")
	subMenu:addIconLine(ucstring("Axe Y"), "lua", "setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:move_y()')", "shape_move_y", "ark_move_y.tga")
	subMenu:addIconLine(ucstring("Axe Z"), "lua", "x, ARK_SHAPE_LATEST_Y = getMousePos(); setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:move_z()')", "shape_move_z", "ark_move_z.tga")
	subMenu:addIconLine(ucstring("Axes X & Y"), "lua", "setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:move_xy()')", "shape_move_xy", "ark_move_xy.tga")
	subMenu:addIconLine(ucstring("Axes X & Y Snap to ground"), "lua", "setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:move_xysnap()')", "shape_move_xy_snap", "ark_move_xysnap.tga")
	subMenu:addSeparator()
	subMenu:addIconLine(ucstring("Move to player"), "lua", "SceneEditor:move_player()", "shape_move_player", "ark_move_player.tga")

	menu:addLine(ucstring("Rotate"), "", "", "shape_rotate")
	menu:addSubMenu(2)
	subMenu = menu:getSubMenu(2)
	subMenu:addIconLine(ucstring("Axe X"), "lua", "ARK_SHAPE_LATEST_X, ARK_SHAPE_LATEST_Y = getMousePos(); setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:rotate(SelectedInstanceId, \"x\")')", "shape_rotate_x", "ark_rotate_x.tga")
	subMenu:addIconLine(ucstring("Axe Y"), "lua", "ARK_SHAPE_LATEST_X, ARK_SHAPE_LATEST_Y = getMousePos(); setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:rotate(SelectedInstanceId, \"y\")')", "shape_rotate_y", "ark_rotate_y.tga")
	subMenu:addIconLine(ucstring("Axe Z"), "lua", "ARK_SHAPE_LATEST_X, ARK_SHAPE_LATEST_Y = getMousePos(); setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:rotate(SelectedInstanceId, \"z\")')", "shape_rotate_z", "ark_rotate_z.tga")

	menu:addLine(ucstring("Scale"), "", "", "shape_scale")
	menu:addSubMenu(3)
	subMenu = menu:getSubMenu(3)
	subMenu:addIconLine(ucstring("Axe X"), "lua", "ARK_SHAPE_LATEST_X, ARK_SHAPE_LATEST_Y = getMousePos(); setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:scale(SelectedInstanceId, \"x\")')", "shape_scale_x", "ark_scale_x.tga")
	subMenu:addIconLine(ucstring("Axe Y"), "lua", "ARK_SHAPE_LATEST_X, ARK_SHAPE_LATEST_Y = getMousePos(); setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:scale(SelectedInstanceId, \"y\")')", "shape_scale_y", "ark_scale_y.tga")
	subMenu:addIconLine(ucstring("Axe Z"), "lua", "ARK_SHAPE_LATEST_X, ARK_SHAPE_LATEST_Y = getMousePos(); setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:scale(SelectedInstanceId, \"z\")')", "shape_scale_z", "ark_scale_z.tga")
	
	menu:addLine(ucstring("-- COLLISION EDITION --"), "", "", "col_header")
	menu:addLine(ucstring("Move"), "", "", "col_move")
	menu:addSubMenu(5)
	subMenu = menu:getSubMenu(5)
	subMenu:addIconLine(ucstring("Axe X"), "lua", "setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:col_move_x()')", "col_move_x", "ark_move_x.tga")
	subMenu:addIconLine(ucstring("Axe Y"), "lua", "setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:col_move_y()')", "col_move_y", "ark_move_y.tga")
	subMenu:addIconLine(ucstring("Axe Z"), "lua", "ARK_SHAPE_LATEST_X, ARK_SHAPE_LATEST_Y = getMousePos(); setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:col_move_z()')", "col_move_z", "ark_move_xy.tga")
	subMenu:addIconLine(ucstring("Axe X & Y"), "lua", "setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:col_move_xy()')", "col_move_xy", "ark_move_xy.tga")
	subMenu:addSeparator()
	subMenu:addIconLine(ucstring("Move to Shape"), "lua", "SceneEditor:col_move_to_shape()", "col_move_to_shape", "ark_move_player.tga")

	menu:addIconLine(ucstring("Rotate"), "lua", "ARK_SHAPE_LATEST_X, ARK_SHAPE_LATEST_Y = getMousePos(); setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:col_rotate(SelectedInstanceId, \"x\")')", "col_rotate_x", "ark_rotate_x.tga")

	menu:addLine(ucstring("Scale"), "", "", "col_scale")
	menu:addSubMenu(7)
	subMenu = menu:getSubMenu(7)
	subMenu:addIconLine(ucstring("Axe X"), "lua", "ARK_SHAPE_LATEST_X, ARK_SHAPE_LATEST_Y = getMousePos(); setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:col_scale(SelectedInstanceId, \"x\")')", "col_scale_x", "ark_scale_x.tga")
	subMenu:addIconLine(ucstring("Axe Y"), "lua", "ARK_SHAPE_LATEST_X, ARK_SHAPE_LATEST_Y = getMousePos(); setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:col_scale(SelectedInstanceId, \"y\")')", "col_scale_y", "ark_scale_y.tga")
	subMenu:addIconLine(ucstring("Axe Z"), "lua", "ARK_SHAPE_LATEST_X, ARK_SHAPE_LATEST_Y = getMousePos(); setOnDraw(getUI('ui:interface:ark_scene_editor'), 'SceneEditor:col_scale(SelectedInstanceId, \"z\")')", "col_scale_z", "ark_scale_z.tga")

	launchContextMenuInGame("ui:interface:ark_scene_editor_edit_menu")
end


function arcc_tools_check_rclick()
	root = getUI("ui:interface")
	local rx, ry = getMousePos()
	i_id = getShapeIdAt(rx, ry)
	if i_id >= 0 then
		setOnDraw(getUI("ui:interface:ark_scene_editor"), "")
	end
end

function SceneEditor:move(id, axe)	
	local d, mx, my = getMouseDown()
	if d then
		setOnDraw(getUI("ui:interface:ark_scene_editor"), "")
		SceneEditor:set_modified(id)
		self:get_html("Moved")
	else
		local x,y,z = getGroundAtMouse()
		if axe == "x" then moveShape(id, tostring(x), "+0", "+0") end
		if axe == "y" then moveShape(id, "+0", tostring(y), "+0") end
		if axe == "z" then
			mx, my = getMousePos()
			moveShape(id, "+0", "+0", "+"..tostring((my-ARK_SHAPE_LATEST_Y)/100))
			ARK_SHAPE_LATEST_Y = my
		end
		if axe == "xysnap" then moveShape(id, tostring(x), tostring(y), tostring(z)) end
		if axe == "xy" then moveShape(id, tostring(x), tostring(y), "+0") end
		if axe == "player" then
			x, y, z = getPlayerPos()
			moveShape(id, tostring(x), tostring(y), tostring(z))
			SceneEditor:set_modified(id)
			self:get_html("Moved to player")
		end
	   
	end
end

function SceneEditor:rotate(id, axe)
	local d, mx, my = getMouseDown()
	if d then
		setOnDraw(getUI("ui:interface:ark_scene_editor"), "")
		SceneEditor:set_modified(id)
		self:get_html("Rotate")
	else
		mx, my = getMousePos()
		if axe == "x" then rotateShape(id, "+"..tostring((my-ARK_SHAPE_LATEST_Y)/100), "+0", "+0") end
		if axe == "y" then rotateShape(id, "+0", "+"..tostring((my-ARK_SHAPE_LATEST_Y)/100), "+0") end
		if axe == "z" then rotateShape(id, "+0", "+0", "+"..tostring((mx-ARK_SHAPE_LATEST_X)/100)) end
		ARK_SHAPE_LATEST_X = mx
		ARK_SHAPE_LATEST_Y = my
	end
end

function SceneEditor:scale(id, axe)
	local d, mx, my = getMouseDown()
	if d then
		setOnDraw(getUI("ui:interface:ark_scene_editor"), "")
		SceneEditor:set_modified(id)
		self:get_html("Rotate")
	else
		mx, my = getMousePos()
		local setup = {}
		if axe == "x" then setup["scale x"]="+"..tostring((mx-ARK_SHAPE_LATEST_X)/100) end
		if axe == "y" then setup["scale y"]="+"..tostring((mx-ARK_SHAPE_LATEST_X)/100) end
		if axe == "z" then setup["scale z"]="+"..tostring((my-ARK_SHAPE_LATEST_Y)/100) end
		setupShape(id, setup)
		ARK_SHAPE_LATEST_X = mx
		ARK_SHAPE_LATEST_Y = my
	end
end


function SceneEditor:move_x()
	self:move(SelectedInstanceId, "x")
end

function SceneEditor:move_y()
	self:move(SelectedInstanceId, "y")
end

function SceneEditor:move_xy()
	self:move(SelectedInstanceId, "xy")
end

function SceneEditor:move_xysnap()
	self:move(SelectedInstanceId, "xysnap")
end

function SceneEditor:move_z()
	self:move(SelectedInstanceId, "z")
end

function SceneEditor:move_player()
	self:move(SelectedInstanceId, "player")
end


function SceneEditor:col_move(id, axe)
	local d, mx, my = getMouseDown()
	
	if d then
		setOnDraw(getUI("ui:interface:ark_scene_editor"), "")
		self:set_modified(id)
		self:get_html("Updated")
	else
		local x,y,z = getGroundAtMouse()
		local setup = {}
		if axe == "x" then setup["col pos x"]=tostring(x) end
		if axe == "y" then setup["col pos y"]=tostring(y) end
		if axe == "z" then
			mx, my = getMousePos()
			setup["col pos z"]="+"..tostring((my-ARK_SHAPE_LATEST_Y)/100)
			ARK_SHAPE_LATEST_X = mx
			ARK_SHAPE_LATEST_Y = my
		end
		if axe == "xy" then setup["col pos x"]=tostring(x); setup["col pos y"]=tostring(y)  end
		if axe == "shape" then
			x, y, z = getShapePos(id)
			setup["col pos x"]=tostring(x)
			setup["col pos y"]=tostring(y)
			self:set_modified(id)
			setupShape(id, setup)
			self:get_html("Updated")
		else
			setupShape(id, setup)
		end
	end
end

function SceneEditor:col_rotate(id, axe)
	local d, mx, my = getMouseDown()
	if d then
		setOnDraw(getUI("ui:interface:ark_scene_editor"), "")
		SceneEditor:set_modified(id)
		self:get_html("Rotate")
	else
		mx, my = getMousePos()
		local setup = {}
		setup["col orientation"]="+"..tostring((mx-ARK_SHAPE_LATEST_X)/100)
		setupShape(id, setup)
		ARK_SHAPE_LATEST_X = mx
		ARK_SHAPE_LATEST_Y = my
	end
end



function SceneEditor:col_scale(id, axe)
	local d, mx, my = getMouseDown()
	if d then
		setOnDraw(getUI("ui:interface:ark_scene_editor"), "")
		SceneEditor:set_modified(id)
		self:get_html("Rotate")
	else
		mx, my = getMousePos()
		local setup = {}
		if axe == "x" then setup["col size x"]="+"..tostring((mx-ARK_SHAPE_LATEST_X)/100) end
		if axe == "y" then setup["col size y"]="+"..tostring((mx-ARK_SHAPE_LATEST_X)/100) end
		if axe == "z" then setup["col size z"]="+"..tostring((my-ARK_SHAPE_LATEST_Y)/100) end
		setupShape(id, setup)
		ARK_SHAPE_LATEST_X = mx
		ARK_SHAPE_LATEST_Y = my
	end
end


function SceneEditor:set_modified(id)
	self.Groups[self.Shapes[id].group].props.modified=true
	self.Shapes[id].modified = "modified"
	self.HaveUpdate = true
end


function SceneEditor:col_move_x()
	self:col_move(SelectedInstanceId, "x")
end

function SceneEditor:col_move_y()
	self:col_move(SelectedInstanceId, "y")
end
function SceneEditor:col_move_z()
	self:col_move(SelectedInstanceId, "z")
end


function SceneEditor:col_move_xy()
	self:col_move(SelectedInstanceId, "xy")
end


function SceneEditor:col_move_to_shape()
	self:col_move(SelectedInstanceId, "shape")
end


function SceneEditor:setup_shape(shape_id, setup)
	final_setup = self.Shapes[new_shape.id].setup
	if final_setup == nil then
		final_setup = {}
	end
	for k,v in pairs(setup) do
		final_setup[k] = v
	end
	self.Shapes[new_shape.id].setup = final_setup
	setupShape(shape_id, setup)
end

function SceneEditor:add(shape)
	if self.LastEditedGroup == nil then
		self:get_html('<font color="#aa00000">'..self.T["no_selected_group"]..'</font>', '000000')
	end
	local new_shape = {}
	new_shape.file = shape
	new_shape.group = self.LastEditedGroup
	self.Groups[new_shape.group].props.modified=true
	new_shape.db_id = self.Groups[new_shape.group].props.count + 1
	new_shape.modified = "added"
	new_shape_id = addShape(shape, 0, 0, 0, "user", 1, true, "", "SceneEditor:show_menu()")
	table.insert(self.Groups[new_shape.group], new_shape_id)
	self.Groups[new_shape.group].props.count = self.Groups[new_shape.group].props.count + 1
	self.Shapes[new_shape_id] = new_shape
	self:get_html("Added")
end


function SceneEditor:removeShape(shape_id)
	deleteShape(shape_id)
	local group = self.Shapes[shape_id].group
	for k,g_shape_id in pairs(self.Groups[group]) do
		if  shape_id == g_shape_id then
			self.Groups[group][k] = nil
		end
	end
	self:set_modified(shape_id)
	self.Shapes[shape_id] = nil
	self:get_html("Removed")
end

function SceneEditor:addGroup(name, count, show, edit)
	if name == nil then
		return
	end
	if self.Groups[name] == nil then
		self.Groups[name] = {}
		self.Groups[name].props = {}
		self.Groups[name].props.count = count
		self.Groups[name].props.show = show
		self.Groups[name].props.edit = edit
		self.Groups[name].props.modified = false
	else
		self.Groups[name].props.show = show
		self.Groups[name].props.edit = edit
	end
end

function SceneEditor:editGroup(group)
	if self.LastEditedGroup then
		self:removeGroup(self.LastEditedGroup, true)
		self:addGroup(self.LastEditedGroup, 0, true, false)
	end
	self:removeGroup(group, true);
	self:addGroup(group, 0, true, true)
	self.LastEditedGroup = group
end

function SceneEditor:addFromDb(group, db_id, json_shape, edit)
	shape = Json.decode(json_shape)
	shape.db_id = db_id

	shape.group = group
	shape.modified = ""
	if edit then
		shape_id = addShape(shape.file, shape.pos[1], shape.pos[2], shape.pos[3], "user", 1, true, "", "SceneEditor:show_menu()")
	else
		shape_id = addShape(shape.file, shape.pos[1], shape.pos[2], shape.pos[3], "user", 1, true, "", "")
	end
	rotateShape(shape_id, tostring(shape.rot[1]), tostring(shape.rot[2]), tostring(shape.rot[3]))
	setupShape(shape_id, shape.setup)
	self.Shapes[shape_id] = shape
	table.insert(self.Groups[group], shape_id)
	if db_id > self.Groups[group].props.count then
		self.Groups[group].props.count = db_id
	end
end


function SceneEditor:removeGroup(group, no_get_html)
	if self.Groups[group] == nil then
		return
	end
	
	for k,shape_id in pairs(self.Groups[group]) do
		if k ~= "props" then
			self.Shapes[shape_id] = nil
			deleteShape(shape_id)
		end
	end
	
	self.Groups[group] = nil
	if self.LastEditedGroup == group then
		self.LastEditedGroup = nil
		local ui = getUI("ui:interface:ark_list_of_shapes")
		if ui then
			ui.active=false
		end
	end
	if no_get_html == nil then
		self:get_html("Group Removed")
	end
end

function SceneEditor:enc64(data)
	local b='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
    return ((data:gsub('.', function(x) 
        local r,b='',x:byte()
        for i=8,1,-1 do r=r..(b%2^i-b%2^(i-1)>0 and '1' or '0') end
        return r;
    end)..'0000'):gsub('%d%d%d?%d?%d?%d?', function(x)
        if (#x < 6) then return '' end
        local c=0
        for i=1,6 do c=c+(x:sub(i,i)=='1' and 2^(6-i) or 0) end
        return b:sub(c+1,c+1)
    end)..({ '', '==', '=' })[#data%3+1])
end

function SceneEditor:get_vector(x, y, z)
	local vector = {}
	table.insert(vector, x)
	table.insert(vector, y)
	table.insert(vector, z)
	
	return vector
end

function SceneEditor:get_random_color()
	local r = math.random(44, 66);
	local g = math.random(44, 66);
	local b = math.random(44, 66);
	return '#'..tostring(r)..tostring(g)..tostring(b)
	
end

function pairsByKeys(t, f)
	local a = {}
	for n in pairs(t) do table.insert(a, n) end
		table.sort(a, f)
		local i = 0      -- iterator variable
		local iter = function ()   -- iterator function
		i = i + 1
		if a[i] == nil then
			return nil
		else
			return a[i], t[a[i]]
		end
	end
	return iter
end

function SceneEditor:show_shape_list()
	local ui = getUI("ui:interface:ark_list_of_shapes")
		local need_setup = ui == nil
		if need_setup then
			WebBrowser:openWindow("ark_list_of_shapes", self.baseUrl..'_ListShapes')
			ui = getUI("ui:interface:ark_list_of_shapes")
			ui.pop_min_w = 400
			ui.w = 400
			getUI("ui:interface:ark_list_of_shapes:browser:header_opened:browse_redo").active=false
			getUI("ui:interface:ark_list_of_shapes:browser:header_opened:browse_undo").active=false
			getUI("ui:interface:ark_list_of_shapes:browser:header_opened:browse_refresh").active=false
			getUI("ui:interface:ark_list_of_shapes:browser:header_opened:browse_home").active=false
		else
			ui.active = true
		end
end

function SceneEditor:getShapesByGroups()
	local groups = {}
	for shape_id, shape in pairs(self.Shapes) do
		if shape.group == nil then
			shape.group = ""
		end
		
		if groups[shape.group] == nil then
			groups[shape.group] = {}
		end
		table.insert(groups[shape.group], shape_id)
	end
	return groups
end


function SceneEditor:get_html_section(message, color)
	return '<table width="100%" cellspacing="0" cellpadding="0"><tr bgcolor="'..color..'"><td align="center" valign="middle"><font color="#FFFFFF" size="12">'..message..'</font></td></tr></table>'
end

function SceneEditor:get_html(message, message_bg)
	debug("get_html :"..message)
	local new_group = '&nbsp;&nbsp;<a class="ryzom-ui-button" href="'..self.baseUrl..'_AddGroup&amp;add_new_group=1&amp;scene_id='..self.sceneId..'"><img src="'..self.iconsUrl..'/32/chart_organisation_add.png" alt="'..self.T["add_new_group"]..'" /></a>'
	local show_hide_cols = '&nbsp;&nbsp;<a class="ryzom-ui-button" href="ah:ark_pacs_borders"><img src="'..self.iconsUrl..'/32/show_hide_cols.png" alt="'..self.T["show_hide_cols"]..'" /></a>'
	local reset_scene = '</td><td align="center" bgcolor="#502020" width="40px"><a class="ryzom-ui-button" href="'..self.baseUrl..'_SaveShapes&amp;reset_scene=1&amp;scene_id='..self.sceneId..'"><img src="'..self.iconsUrl..'/32/bin.png" alt="'..self.T["reset_scene"]..'" /></a>'

	local html = '<header><title>'..self.T["sceno_editor"]..'</title></header>'..self:get_html_section(message..'</td><td bgcolor="#202020" align="center" height="40px" width="140px" valign="middle">'..new_group..show_hide_cols..reset_scene, (message_bg or SceneEditor:get_random_color()))

	html = html .. '<form action="'..self.baseUrl..'_SaveShapes" method="POST"><input type="hidden" name="group" value="'..(self.LastEditedGroup or "")..'" /><input type="hidden" name="scene_id" value="'..self.sceneId..'" />\
	<table width="100%" cellspacing="0" cellpadding="0">'
	
	local groups = self:getShapesByGroups()
	
	for group, shapes in pairsByKeys(self.Groups) do
		debug("Group : "..group)
		local groupname = group
		html = html .. '<tr bgcolor="#444444"><td height="20px"><table width="100%"><tr><td>&nbsp;'..groupname..' ('..(self.Groups[group].props.count or '0')..') </td><td align="right"><input type="hidden" name="shape[]", value="#"/>'
		
		
		if self.Groups[group].props.show then
			debug("Group : show")
			if self.Groups[group].props.edit then
				html = html .. '<a href="ah:lua:SceneEditor:show_shape_list()"><img src="'..self.iconsUrl..'/16/box_add.png" alt="'..self.T["add_shape"]..'"/></a></td><td align="right">'
				if self.HaveUpdate then
					html = html .. '<a class="ryzom-ui-button" href="'..self.baseUrl..'_SaveShapes&amp;hide_group='..group..'&amp;edit_group='..group..'">'..self.T["cancel"]..'</a>'
				else
					html = html .. '<a class="ryzom-ui-button" href="'..self.baseUrl..'_SaveShapes&amp;hide_group='..group..'">'..self.T["hide"]..'</a>'
				end
			else
				html = html .. '<a class="ryzom-ui-button" href="'..self.baseUrl..'_SaveShapes&amp;hide_group='..group..'">'..self.T["hide"]..'</a>'
			end
		else
			html = html .. '<a class="ryzom-ui-button" href="'..self.baseUrl..'_SaveShapes&amp;show_group='..group..'">'..self.T["show"]..'</a>'
		end
		
		local shapes_html = ""
		local show = self.Groups[group].props.show
		if self.Groups[group].props.edit then
			shapes_id = groups[group]
			if shapes_id then
				for k,shape_id in pairs(shapes_id) do
					shape = {}
					if self.Shapes[shape_id] then
						shape["db_id"] = self.Shapes[shape_id].db_id
						shape["file"] = self.Shapes[shape_id].file
						shape["pos"] = self:get_vector(getShapePos(shape_id))
						scale_x, scale_y, scale_z = getShapeScale(shape_id)
						shape["rot"] = self:get_vector(getShapeRot(shape_id))
						colpos_x, colpos_y, colpos_z = getShapeColPos(shape_id)
						colscale_x, colscale_y, colscale_z = getShapeColScale(shape_id)
						shape["setup"] = {}
						shape["setup"]["scale x"] = scale_x
						shape["setup"]["scale y"] = scale_y
						shape["setup"]["scale z"] = scale_z
						shape["setup"]["col pos x"] = shape["pos"][1]+colpos_x
						shape["setup"]["col pos y"] = shape["pos"][2]+colpos_y
						shape["setup"]["col size x"] = colscale_x
						shape["setup"]["col size y"] = colscale_y
						shape["setup"]["col size z"] = colscale_z
						local color = "202020"
						if k % 2 == 0 then
							color = "101010"
						end
						local text_color = "ef9b64"
						if self.Shapes[shape_id].modified == "modified" then
							 text_color = "aa5555"
						else
							if self.Shapes[shape_id].modified == "added" then
								 text_color = "55aa55"
							end
						end
						shapes_html = shapes_html .. "<tr bgcolor='#"..color.."'><td height='20px'>&nbsp;<input type='hidden' name='shape[]', value='"..SceneEditor:enc64((shape.db_id or '')..":"..Json.encode(shape)).."' />"..'#'..(shape.db_id or '0').." <a href='ah:lua:SceneEditor:launch_menu("..tostring(shape_id)..")'><font color='#"..text_color.."'>"..shape.file.."</font></a></td>\
						<td width='16px'><a href='ah:lua:SceneEditor:removeShape("..tostring(shape_id)..")'><img src='"..self.iconsUrl.."/16/cross.png' /></a></td>\
						</tr>"
					end
				end
			end
		else
			if self.HaveUpdate == nil then
				html = html .. '&nbsp;&nbsp;<a class="ryzom-ui-button" href="'..self.baseUrl..'_SaveShapes&amp;edit_group='..group..'">'..self.T["edit"]..'</a>'
				html = html .. '</td><td align="right"><a class="ryzom-ui-button" href="'..self.baseUrl..'_SaveShapes&amp;reset_scene=1&amp;del_group='..group..'">'..self.T["remove"]..'</a>'
			end
		end
		
		if self.Groups[group].props.modified then
			html = html .. '&nbsp;&nbsp;<input type="submit" value="'..self.T["save"]..'" />'
		end
		html = html .. '</td></tr></table></td><td></td></tr>'..shapes_html
		
	end
	
	html = html .. '</table></form>'
	ui = getUI("ui:interface:ark_scene_editor:browser:content:html", false)
	if ui then
		ui:renderHtml(html)
	end
end

