-- Some pathes to configure

old_zone_path				= "w:\\database\\landscape\\zones\\"
ligo_root_path				= "w:\\database\\landscape\\ligo\\max\\"
ligo_material_path			= "materials\\"
ligo_transition_path		= "transitions\\"
-- ligo_export_path path where to put (*.zone,*ligozone and *.tga)
-- 3 pathes are created under this one :(zones, zoneLigos and zoneBitmaps)
ligo_export_path			= "w:\\database\\landscape\\ligo\\"

-- Some extensions

ligo_material_ext		= ".ligomat"
ligo_transition_ext		= ".ligotrans"
ligo_zone_ext			= ".ligozone"
nel_zone_ext			= ".zone"

-- Some filters

ligo_material_filter	= "Ligo Material (*.ligomat)|*.ligomat|All Files (*.*)|*.*|"
ligo_transition_filter	= "Ligo Transition (*.ligotrans)|*.ligotrans|All Files (*.*)|*.*|"
ligo_zone_filter		= "Ligo Zone (*.ligozone)|*.ligozone|All Files (*.*)|*.*|"

-- Some prefix

ligo_material_prefix	= "material-"
ligo_transition_prefix	= "transition-"
ligo_zonematerial_prefix = "zonematerial-"
ligo_zonetransition_prefix = "zonetransition-"
ligo_zonespecial_prefix = "zonespecial-"

-- Some globals

rootpath				= (getdir #maxroot)
title_msgbox			= "NeL Ligo tools"
NoError					= 1

-- Some structures

struct zone_transformation ( symmetry, rotation )

-- Structures methods

fn xor b1 b2 = (not (b1 and b2)) and (b1 or b2) 

fn compose transfo0 transfo1 =
(
	ret = zone_transformation false 0

	-- Compose the symmetry
	ret.symmetry = xor transfo0.symmetry transfo1.symmetry

	-- Compose the rotation
	ret.rotation = transfo0.rotation
	if transfo1.symmetry == true then
		ret.rotation = 4 - ret.rotation
	ret.rotation = mod (ret.rotation + transfo1.rotation) 4

	return ret
)

-- NoError					- color 0 255 0
-- NoEdgeVertices			- color 0 0 0
-- OpenedEdge				- color 255 0 0
-- MultipleEdge				- color 0 0 0
-- VertexList				- color 255 255 0
-- NotInserted				- color 0 0 255
-- Inserted					- color 255 0 255
-- FlatZone					- color 0 0 0
-- MustHave4Edges			- color 0 0 0
-- NotSymetrical			- color 0 255 255
-- NotSameVerticesNumber	- color 0 0 0
-- NotSameVertex			- color 128 0 0
-- NoCornerFound			- color 0 0 0
-- UnknownError				- color 255 255 255

ErrorColor				= #(color 0 255 0, color 0 0 0, color 255 0 0, color 0 0 0, color 255 255 0, color 0 0 255, color 255 0 255, color 0 0 0, color 0 0 0, color 0 255 255, color 0 0 0, color 128 0 0, color 0 0 0, color 128 0 128, color 128 128 0, color 0 128 128, color 255 255 255)

TransitionIds			= #( #( 1, 2 ), #( undefinded, 3 ), #( 5, 4 ), #( 6, 7, 8, 9 ) )
EdgeType				= #( #(2, 2, -4, 4), #(4, 2, 2, -3), #(1, 3, 2, -3), #(-3, 3, 2, 2), #(1, 1, 3, -4), #(1, 4, -4, 1), #(4, 2, -4, 1), #(4, 2, -3, 1), #(3, -3, 1, 1))
TransitionPosition		= #( [0,0,0], [1,0,0], [1,1,0], [1,2,0], [0,2,0], [0,3,0], [1,3,0], [2,3,0], [3,3,0] )

-- Neigbor data

NeighborDeltaRot		= #( 180, 270, 0, 90 )
NeighborDeltaPos		= #( [-1,0,0], [0,-1,0], [1,0,0], [0,1,0] )
NeighborName			= #( "[NELLIGO] Left help zone", "[NELLIGO] Bottom help zone", "[NELLIGO] Right help zone", "[NELLIGO] Top help zone" )

-- Transition transform

TransitionScale			= #( false,		false,		false,			false,			true,		false,		false,			false,			false)
TransitionRot			= #( 2,			1,			3,				0,				1,			3,			0,				0,				0)
TransitionPos			= #( [0, 0, 0], [-1, 0, 0], [-1, -1, 0],	[-1, -2, 0],	[0, -2, 0], [0, -3, 0], [-1, -3, 0],	[-2, -3, 0],	[-3, -3, 0])
TransitionType			= #( "CornerA", "CornerA",	"Flat",			"CornerA",		"CornerB",	"CornerB",	"Flat",			"Flat",			"CornerB")
TransitionNum			= #( 22,		12,			22,				11,				12,			11,			11,				12,				22)
TransitionNumBis		= #( 5,			4,			2,				3,				7,			6,			0,				1,				8)

-- Some appdata

ligo					= 1304892483
ligo_passable_app		= ligo
ligo_rotate_app			= 1266703978
ligo_symmetry_app		= 1266703979
ligo_instance_app		= 1266703980
ligo_use_boundingbox	= 1342141818

-- Project file list

project_file_list		= #()
project_cur_file		= ""

-- Functions

-- Add properties
fn addProperties node propsArray =
(
	-- The passable properties
	if (getappdata node ligo_passable_app) != undefined then
		append propsArray #( "passable", "yes")
	else
		append propsArray #( "passable", "no")
)

-- Select all zone
fn selectAllPatch =
(
	clearselection ()
	for i in geometry do 
	(
		if (classof i) == RklPatch then 
		(
			if (i.isFrozen == false) then 
			(
				selectmore i
			)
		)
	)
)


-- Minimise
fn minimise =
(
	if (viewport.numViews == 1) then
		max tool maximize
)

-- Is a material
fn isMaterial name =
(
	-- Get token
	tokenArray = filterString (getFilenameFile name) "-"

	-- Valid name ?
	return ((tokenArray.count == 2) and (tokenArray[1] == "material"))
)

-- Is a transition
fn isTransition name =
(
	-- Get token
	tokenArray = filterString (getFilenameFile name) "-"

	-- Valid name ?
	return ((tokenArray.count == 3) and (tokenArray[1] == "transition"))
)

-- Refresh project list
fn refresh_project_list =
(
	project_file_list = getFiles (ligo_root_path + "*.max")
	-- Refresh rollouts
	-- material template
	if material_rollout != undefined then
	(
		material_rollout.refresh_list ()
	)
	-- material zone
	if material_zone_rollout != undefined then
	(
		material_zone_rollout.refresh_mat_list ()
		material_zone_rollout.refresh_zone_list material_zone_rollout.MaterialDDL.selected
	)
	-- transition template
	if transition_rollout != undefined then
	(
		transition_rollout.refresh_mat_list ()
		transition_rollout.refresh_list ()
	)
	-- transition zone
	if transition_zone_rollout != undefined then
	(
		transition_zone_rollout.refresh_trans_list ()
		transition_zone_rollout.refresh_list transition_zone_rollout.TransitionDDL.selected
	)
	-- special zone
	if special_zone_rollout != undefined then
	(
		special_zone_rollout.refresh_zone_list ()
	)
)

-- Is this node a debug markers ?
fn isDebugMarker node = 
(
	return matchPattern node.name pattern:"[NELLIGO]*"
)

-- Delete debug markers
fn DeleteDebugMarkersFn =
(
	-- Array to delete
	arrayToDelete = #()

	-- For each error names
	for i = 1 to rootNode.children.count do
	(
		-- Node id
		id = rootNode.children.count-i+1

		if (isDebugMarker rootNode.children[id]) == true then
		(
			append  arrayToDelete rootNode.children[id]
		)
	)

	-- For each node to delete
	for i = 1 to arrayToDelete.count do
	(
		-- Node id
		delete arrayToDelete[i]
	)
)

-- Add a painter modifier to visible objects
fn AddPainterModToVisible =
(
	-- Unselect non NeL patch mesh
	max select none

	-- For each error names
	for node in geometry do
	(
		-- Is a NeL zone ?
		if (classof node) == RklPatch then
		(
			-- Is visible ?
			if node.isHidden == false then
			(
				-- Is not an instance ?
				if (getappdata node ligo_instance_app) == undefined then
				(
					-- Select it
					selectmore node
				)
			)
		)
	)

	-- For each node to delete
	modPanel.addModToSelection (NeL_Painter ())
)

-- Get the material names in a transition file name
fn get_material_name_in_transition_name transName result =
(
	-- Decompose in token
	tokenArray = filterString (getFilenameFile transName) "-"

	-- Look for token "transition"
	if tokenArray.count== 3 then
	(
		if tokenArray[1] == "transition" then
		(
			-- Get the two names
			result[1] = tokenArray[2]
			result[2] = tokenArray[3]

			-- Ok
			return true
		)
	)

	-- Error
	Messagebox (transName+" is not a valid transition project name. ( should be : transition-materialName[1]-materialName[2].max )") title:title_msgbox beep:true

	-- Error
	return false
)

-- Get the material names in a material project file name
fn get_material_name_in_material_name matProjName matName =
(
	-- Decompose in token
	tokenArray = filterString (getFilenameFile matProjName) "-"

	-- Look for token "transition"
	if tokenArray.count == 2 then
	(
		if tokenArray[1] == "material" then
		(
			-- Get the two names
			matName[1] = tokenArray[2]

			-- Ok
			return true
		)
	)

	-- Error
	Messagebox (matProjName+" is not a valid material project name. ( should be : material-materialName.max )") title:title_msgbox beep:true

	-- Error
	return false
)

-- Get the material and zone name in a zone project file name
fn get_material_and_zone_name_in_zone_project_name zoneNameSrc result =
(
	-- Decompose in token
	tokenArray = filterString (getFilenameFile zoneNameSrc) "-"

	-- Look for token "zone"
	if tokenArray.count == 3 then
	(
		if tokenArray[1] == "zonematerial" then
		(
			-- Get the two names
			result[1] = tokenArray[2]
			result[2] = tokenArray[3]

			-- Ok
			return true
		)
	)

	-- Error
	Messagebox (zoneNameSrc+" is not a valid zone material project name. ( should be : zonematerial-materialName-zoneName.max )") title:title_msgbox beep:true

	-- Error
	return false
)

-- Get the material and zone name in a zone project file name
fn get_material_and_zone_name_in_special_project_name zoneNameSrc result =
(
	-- Decompose in token
	tokenArray = filterString (getFilenameFile zoneNameSrc) "-"

	-- Look for token "zone"
	if tokenArray.count == 2 then
	(
		if tokenArray[1] == "zonespecial" then
		(
			-- Get the name
			result[1] = tokenArray[2]

			-- Ok
			return true
		)
	)

	-- Error
	Messagebox (zoneNameSrc+" is not a valid zone special project name. ( should be : zonespecial-zoneName.max )") title:title_msgbox beep:true

	-- Error
	return false
)

-- Get the transition and zone name in a transition zone project file name
fn get_transition_and_zone_name_in_zone_project_name zoneNameSrc result =
(
	-- Decompose in token
	tokenArray = filterString (getFilenameFile zoneNameSrc) "-"

	-- Look for token "zone"
	if tokenArray.count == 4 then
	(
		if tokenArray[1] == "zonetransition" then
		(
			-- Get the two names
			result[1] = tokenArray[2]
			result[2] = tokenArray[3]
			result[3] = tokenArray[4]

			-- Ok
			return true
		)
	)

	-- Error
	Messagebox (zoneName+" is not a valid zone transition project name. ( should be : zonetransition-materialName[1]-materialName[2]-zoneName.max )") title:title_msgbox beep:true

	-- Error
	return false
)

-- Show errors
fn show_errors node debug id =
(
	-- Get the error message
	code_array = #()
	id_array = #()
	message_array = #()
	error_msg = NeLLigoGetErrorZoneTemplate code_array id_array message_array id

	-- No error ?
	if error_msg == NoError then
	(
		return false
	)
	else
	(
		-- Show an error message
		messageBox (NeLLigoGetErrorString error_msg) title:title_msgbox beep:true

		-- Debug ?
		if debug == true then
		(
			-- Number of patch vertices
			vertCount = GetRykolVertexCount node 

			-- Create a box for each vertex error
			error_array = #()
			for i=1 to vertCount do
			(
				-- Set the error code
				error_array[i] = NoError
			)

			-- Set the code error
			for i=1 to code_array.count do
			(
				-- Set the error code
				error_array[id_array[i]] = code_array[i]
			)

			-- Get the snap
			snap = NeLLigoGetSnap ()

			-- The node tm
			tm = node.objectTransform

			-- Draw a box for each errors
			for i=1 to vertCount do
			(
				-- Error ?
				if error_array[i] != NoError then
				(
					-- Get a color
					boxColor = color 0 0 0
					if error_array[i] <= ErrorColor.count then
						boxColor = ErrorColor[error_array[i]]

					-- Create a box
					Box name:("[NELLIGO] " + NeLLigoGetErrorString error_array[i]) lengthsegs:1 widthsegs:1 heightsegs:1 length:snap width:snap height:snap pos:((GetRykolVertexPos node i) * tm) isSelected:off wirecolor:boxColor
				)
			)
		)

		-- Error
		return false
	)

	return true
)

-- Export a ligo material
fn export_and_check_and_debug_material checkOnly debug displayMB = 
( 
	-- Select a file ?
	if $ == undefined then
	(
		if displayMB == true then
			messageBox "Please select a 3ds NeL patch mesh object" title:title_msgbox beep:true
	)
	else
	(
		if $ == $selection then
		(
			if displayMB == true then
				messageBox "Please select only one 3ds NeL patch mesh object" title:title_msgbox beep:true
		)
		else
		(
			-- Set the current directory

			filename = ""
			tmpArray = #()
			-- Get the material name
			if (get_material_name_in_material_name maxFileName tmpArray) == true then
			(
				-- Final path
				filename = ligo_root_path+ligo_material_path+tmpArray[1]+ligo_material_ext
			)

			if filename != "" then
			(
				if (NeLLigoExportMaterial $ filename checkOnly true) == false then
				(
					-- Backup selection
					backupSelect = $selection as array

					res = show_errors $ debug 1

					-- Restaure
					select backupSelect

					return res
				)
				else
				(
					-- Success
					if checkOnly == true then
					(
						if displayMB == true then
							messageBox "No error" title:title_msgbox beep:true
					)
					else
					(
						if displayMB == true then
							messageBox (filename+" export success") title:title_msgbox
					)

					return true
				)
			)
		)
	)

	-- Error
	return false
)

-- Get the transition zone coordinate
fn getTransitionZoneCoordinates node x y = 
(
	-- Get nelsize
	cellSize = NeLLigoGetCellSize ()

	-- Object center
	center = node.center

	-- Get X and Y coordinates
	x[1] = (center.x / cellSize) as Integer
	y[1] = (center.y / cellSize) as Integer

	-- Valid Y coordinate ?
	if (y[1] < 0) or (y[1] >= TransitionIds.count) then
	(
		messageBox ("The zone "+nodeArray[zone].name+" is not well positionned. Check the transition scheme.") title:title_msgbox beep:true
		return false
	)
	else
	(
		-- Valid X coordinate ?
		if (x[1] < 0) or (x[1] >= TransitionIds[y[1]+1].count) then
		(
			messageBox ("The zone "+nodeArray[zone].name+" is not well positionned. Check the transition scheme.") title:title_msgbox beep:true
			return false
		)
	)

	return true
)

-- Export a ligo transition
fn export_and_check_and_debug_transition_template checkOnly debug displayMB = 
( 
	-- Get nelsize
	cellSize = NeLLigoGetCellSize ()

	-- Select a file ?
	if $ == undefined then
	(
		if displayMB == true then
			messageBox "Please select one or several 3ds NeL patch mesh object" title:title_msgbox beep:true
	)
	else
	(
		-- Array of selection
		nodeArray = #()
		if $ == $selection then
		(
			nodeArray = $selection as array
		)
		else
		(
			append nodeArray $
		)

		-- Ok flag
		ok = true
		findOne = false

		-- Cell Size
		if cellSize != undefined then
		(
			-- Array of transitions zones
			transitionZone = #()
			for zone = 1 to 9 do
				append transitionZone undefined
			for zone = 1 to nodeArray.count do
			(
				-- Not a debug object ?
				if (isDebugMarker nodeArray[zone]) == false then
				(
					-- Find one
					findOne = true;

					-- Get the zone coordinates
					x = #()
					y = #()
					ok = getTransitionZoneCoordinates nodeArray[zone] x y

					-- No error ?
					if ok == true then
					(
						-- Transition number
						id = TransitionIds[y[1]+1][x[1]+1]

						-- Store it
						transitionZone[id] = nodeArray[zone]
					)
				)
			)

			-- Continue ?
			if (findOne == true) then
			(
				if (ok == true) then
				(
					-- Select the two materials
					materialName = #()
					ok = get_material_name_in_transition_name maxFileName materialName

					-- Make material file names with
					materialName1 = ligo_root_path+ligo_material_path+materialName[1]+ligo_material_ext
					materialName2 = ligo_root_path+ligo_material_path+materialName[2]+ligo_material_ext

					-- Two material selected ?
					if ok == true then
					(
						filename = ""

						-- Get the destination file name
						materialName = #()
						if (get_material_name_in_transition_name maxFileName materialName) == true then
						(
							filename = ligo_root_path+ligo_transition_path+materialName[1]+"-"+materialName[2]+ligo_transition_ext
						)
						else
						(
							filename = undefined
						)

						-- Selected ?
						if filename != undefined then
						(
							if (NeLLigoExportTransition transitionZone filename materialName1 materialName2 checkOnly true) == false then
							(
								-- Backup the selection
								backupSel = $selection as array

								returnError = true
								for zoneError = 1 to 9 do
								(
									if (transitionZone[zoneError] != undefined) then
									(
										-- Select the node
										if (show_errors transitionZone[zoneError] debug zoneError) == false then
											returnError = false
									)
								)

								-- Re select
								select backupSel

								return returnError
							)
							else
							(
								-- Success
								if checkOnly == true then
								(
									if displayMB == true then
										messageBox "No error" title:title_msgbox beep:true
								)
								else
								(
									if displayMB == true then
										messageBox (filename+" export success") title:title_msgbox
								)

								return true
							)
						)
					)
				)
			)
			else
			(
				if displayMB == true then
					messageBox "No zone selected" title:title_msgbox beep:true
			)
		)
	)

	-- Error
	return false
)

-- Export a ligo zone
fn export_and_check_and_debug_zone checkOnly debug displayMB = 
( 
	-- Select a zone
	if $ == undefined then
	(
		if displayMB == true then
			messageBox "Please select a 3ds NeL patch mesh object" title:title_msgbox beep:true
	)
	else
	(
		if $ == $selection then
		(
			if displayMB == true then
				messageBox "Please select only one 3ds NeL patch mesh object" title:title_msgbox beep:true
		)
		else
		(

			-- Get the material name with the project name
			result = #()
			if (get_material_and_zone_name_in_zone_project_name maxFileName result) == true then
			(
				matname = result[1]
				zonename = result[2]

				-- Make a full path
				filename = ligo_root_path+ligo_material_path+matname+ligo_material_ext

				-- Don't check flag ?
				ligo_use_boundingbox_data = getAppData $ ligo_use_boundingbox
				if (ligo_use_boundingbox_data != undefined) and (ligo_use_boundingbox_data == "1") then
					dontCheck = true
				else
					dontCheck = false

				if (dontCheck == false) and ( (NeLLigoCheckZoneWithMaterial $ filename true) == false ) then
				(
					-- Backup selection
					backupSelect = $selection as array

					res = show_errors $ debug 1

					-- Restaure
					select backupSelect

					return res
				)
				else
				(
					-- Success

					if checkOnly == true then
					(
						if displayMB == true then
							messageBox "No error" title:title_msgbox beep:true

						return true
					)
					else
					(
						-- Select an output filename
						-- filename = ligo_root_path+ligo_zone_path+matname+"-"+zonename+ligo_zone_ext
						filename = ligo_export_path + matname + "-" + zonename + ligo_zone_ext
						
						-- Make an array of properties
						props = #( #("zone", (matname + "-" + zonename)), #("material", result[1]) ) 

						-- Add the properties
						addProperties $ props

						-- Export the files ( The ligozone (*.ligozone), the NeL zone (*.zone), and the snap shot (*.tga) )
						if (NeLLigoExportZone $ filename props true true) == true then
						(
							-- Success message
							if displayMB == true then
								messageBox (filename+" export success") title:title_msgbox

							return true
						)
					)
				)
			)
		)
	)

	-- Error
	return false
)

-- Hide all zones
fn hideAllZone =
(
	for node in geometry do
	(
		if (classof node) == RklPatch then
		(
			hide node
		)
	)
)

-- Export a ligo transition
fn export_and_check_and_debug_transition_zone checkOnly debug displayMB = 
( 
	-- Get nelsize
	cellSize = NeLLigoGetCellSize ()

	-- Select a file ?
	if $ == undefined then
	(
		if displayMB == true then
			messageBox "Please select one or several 3ds NeL patch mesh object" title:title_msgbox beep:true
	)
	else
	(
		-- Array of selection
		nodeArray = #()
		if $ == $selection then
		(
			nodeArray = $selection as array
		)
		else
		(
			append nodeArray $
		)

		-- Ok flag
		ok = true
		findOne = false

		-- Cell Size
		if cellSize != undefined then
		(
			-- Array of transitions zones
			transitionZone = #()
			for zone = 1 to 9 do
				append transitionZone undefined
			for zone = 1 to nodeArray.count do
			(
				-- Not a debug object ?
				if (isDebugMarker nodeArray[zone]) == false then
				(
					-- Find one
					findOne = true;

					-- Get the zone coordinates
					x = #()
					y = #()
					ok = getTransitionZoneCoordinates nodeArray[zone] x y

					-- No error ?
					if ok == true then
					(
						-- Transition number
						id = TransitionIds[y[1]+1][x[1]+1]

						-- Store it
						transitionZone[id] = nodeArray[zone]
					)
				)
			)

			-- Continue ?
			if (findOne == true) then
			(
				if (ok == true) then
				(
					-- Select the two materials
					names = #()
					ok = get_transition_and_zone_name_in_zone_project_name maxFileName names

					-- Two material selected ?
					if ok == true then
					(
						-- Make transition file with them
						transitionName = ligo_root_path + ligo_transition_path + names[1] + "-"
						transitionName = transitionName + names[2] + ligo_transition_ext

						-- Export success
						result = true

						-- For each zone selected
						for zone = 1 to 9 do
						(
							-- Defined ?
							if transitionZone[zone] != undefined then
							(
								-- Check the zone
								if (NeLLigoCheckZoneWithTransition transitionZone[zone] transitionName (zone-1) true) == true then
								(
									-- Not check only ?
									if checkOnly == false then
									(
										-- Zone name
										zoneBaseName = names[1] + "-" + names[2] + "-" + names[3] + "-" + ((zone-1) as string)

										-- Get the destination filename for export
										filename = ligo_export_path + zoneBaseName + ligo_zone_ext

										-- Make an array of properties
										props = #( #("zone", zoneBaseName), #("transname", (names[1]+"-"+names[2]) ), #("transtype", TransitionType[zone] ), #("transtype", TransitionType[zone] ), #("transnum", (TransitionNumBis[zone] as string) ) ) 

										-- Add the properties
										addProperties transitionZone[zone] props

										-- Transform the zone
										c = instance transitionZone[zone]

										-- Hide all zones
										hideAllZone ()

										-- Show the zone
										unhide c

										-- Change the name
										c.name = "[NELLIGO] tmp export"

										-- The matrix
										mt = transitionZone[zone].transform
										backupPos = mt.pos
										mt.pos = [0,0,0]

										-- Scale ?
										if TransitionScale[zone] == true then
										(
											mt = scale mt [-1,1,1]
											setappdata c ligo_symmetry_app "1"
										)

										-- Rotate
										if TransitionRot[zone] != 0 then
										(
											mt = rotateZ mt (90*TransitionRot[zone])
											setappdata c ligo_rotate_app (TransitionRot[zone] as string)
										)

										-- Translation
										mt = translate mt ( (TransitionPos[zone] * cellSize) + backupPos)

										-- Transform the node
										c.transform = mt

										-- Export the zone
										if (NeLLigoExportZone c filename props true true) == true then
										(
										)
										else
										(
											result = false
										)

										-- Delete
										delete c
									)
								)
								else
								(
									-- Backup selection
									backupSelect = $selection as array

									res = show_errors transitionZone[zone] debug 1

									-- Restaure
									select backupSelect

									result = false
								)
							)
						)

						-- Success message
						if (result == true) then
						(
							if checkOnly == true then
							(
								-- Success
								if displayMB == true then
									messageBox "No error" title:title_msgbox beep:true
							)
							else
							(
								-- Success message
								if displayMB == true then
									messageBox ("Export success") title:title_msgbox
							)

							-- Success
							return true
						)
					)
				)
			)
		)
	)

	-- Error
	return false
)


-- Export a special zone
fn export_and_check_and_debug_special_zone checkOnly debug displayMB = 
( 
	-- Select a zone
	if $ == undefined then
	(
		if displayMB == true then
			messageBox "Please select a 3ds NeL patch mesh object" title:title_msgbox beep:true
	)
	else
	(
		if $ == $selection then
		(
			if displayMB == true then
				messageBox "Please select only one 3ds NeL patch mesh object" title:title_msgbox beep:true
		)
		else
		(

			-- Get the material name with the project name
			result = #()
			if (get_material_and_zone_name_in_special_project_name maxFileName result) == true then
			(
				zonename = result[1]

				if (NeLLigoCheckZoneWithMaterial $ "" true) == false then
				(
					-- Backup selection
					backupSelect = $selection as array

					res = show_errors $ debug 1

					-- Restaure
					select backupSelect

					return res
				)
				else
				(
					-- Success

					if checkOnly == true then
					(
						if displayMB == true then
							messageBox "No error" title:title_msgbox beep:true

						return true
					)
					else
					(
						-- Select an output filename
						-- filename = ligo_root_path+ligo_zone_path+matname+"-"+zonename+ligo_zone_ext
						filename = ligo_export_path + zonename + ligo_zone_ext
						
						-- Make an array of properties
						props = #( #("zone", zonename), #("material", "special") )

						-- Add the properties
						addProperties $ props

						-- Export the files ( The ligozone (*.ligozone), the NeL zone (*.zone), and the snap shot (*.tga) )
						if (NeLLigoExportZone $ filename props true true) == true then
						(
							-- Success message
							if displayMB == true then
								messageBox (filename+" export success") title:title_msgbox

							return true
						)
					)
				)
			)
		)
	)

	-- Error
	return false
)

-- Get a transition zone
fn getTransition id =
(
	-- Gor all in geometry
	for node in geometry do
	(
		-- Get its coordinates
		x = #()
		y = #()
		if (getTransitionZoneCoordinates node  x y) == true then
		(
			-- Good id ?
			if (id == TransitionIds[y[1]+1][x[1]+1]) then
				return node
		)
	)

	-- Return nothing
	return undefined
)

-- Compare 2 dates
fn date_older_or_equal_than date1 date2 =
(
	tabDate1 = filterstring date1 "/ :"
	tabDate2 = filterstring date2 "/ :"

	-- Year comparison
	if ((tabDate1[3] as integer) < (tabDate2[3] as integer)) then
		return true
	if ((tabDate1[3] as integer) > (tabDate2[3] as integer)) then
		return false

	-- Month comparison
	-- French Date ?
	if (tabDate1.count == 6) then
	(
		if ((tabDate1[2] as integer) < (tabDate2[2] as integer)) then
			return true
		if ((tabDate1[2] as integer) > (tabDate2[2] as integer)) then
			return false
	)
	-- Not french date so this is an english date
	else
	(
		if ((tabDate1[1] as integer) < (tabDate2[1] as integer)) then
			return true
		if ((tabDate1[1] as integer) > (tabDate2[1] as integer)) then
			return false
	)

	-- Day comparison
	-- French Date ?
	if (tabDate1.count == 6) then
	(
		if ((tabDate1[1] as integer) < (tabDate2[1] as integer)) then
			return true
		if ((tabDate1[1] as integer) > (tabDate2[1] as integer)) then
			return false
	)
	-- Not french date so this is an english date
	else
	(
		if ((tabDate1[2] as integer) < (tabDate2[2] as integer)) then
			return true
		if ((tabDate1[2] as integer) > (tabDate2[2] as integer)) then
			return false
	)

	-- MidDay comparison if english date
	if (tabDate1.count == 7) then
	(
		if ((tabDate1[7] == "AM") and (tabDate2[7] == "PM")) then
			return true
		if ((tabDate1[7] == "PM") and (tabDate2[7] == "AM")) then
			return false
	)

	-- Hour
	if ((tabDate1[4] as integer) < (tabDate2[4] as integer)) then
		return true
	if ((tabDate1[4] as integer) > (tabDate2[4] as integer)) then
		return false

	-- Minute
	if ((tabDate1[5] as integer) < (tabDate2[5] as integer)) then
		return true
	if ((tabDate1[5] as integer) > (tabDate2[5] as integer)) then
		return false

	-- Seconds
	if ((tabDate1[6] as integer) < (tabDate2[6] as integer)) then
		return true
	if ((tabDate1[6] as integer) > (tabDate2[6] as integer)) then
		return false

	-- equal date
	return true
)

-- Put in upper case
fn uppercase instring =
(  
	local upper, lower, outstring
	upper="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	lower="abcdefghijklmnopqrstuvwxyz" 

	outstring = copy instring 

	for i = 1 to outstring.count do 
	(  
		j = findString lower outstring[i]
		if (j != undefined) then
			outstring[i] = upper[j] 
		else
			outstring[i] = instring[i]
	)
	outstring     -- value of outstring will be returned as function result 
)

-- Roll out

rollout material_rollout "Material Template"
(
group "Check, debug and export"
	(

	Button		ExportAllMaterial		"Export all material"			width:140 align:#center
	Button		ExportAllMaterialNonOk	"Export all material non-ok"	width:140 align:#center
	Button		ExportMaterial			"Export material template"		width:140 align:#center
	Button		ShowHelpZones			"Show help zones"				width:140 align:#center
	Button		DeleteDebugMarkers		"Delete debug markers"			width:140 align:#center
	Checkbox	CheckOnly				"Check only"					checked:false enabled:true
	Checkbox	Debug					"Debug"							checked:false enabled:true

	)

group "Projects management"
	(

	Listbox		MaterialProject			"Material projects"				height:8 align:#center
	Button		DelMaterial				"Delete selected material"		width:140 align:#center
	Label		MaterialName			"Material name:"				align:#left
	EditText	NewMaterialName			""								width:140 align:#left
	Button		AddMaterial				"Add material"					width:140 align:#center

	)

	fn refresh_list =
	(
		-- Array
		tmpArray = #()

		-- Make an array of material project
		for curFileName in project_file_list do
		(
			-- Valid name ?
			if (isMaterial curFileName) == true then
			(
				curFileDate = getFileModDate curFileName
				expFileDate = ""
				expFileNameBase = #()
				if (get_material_name_in_material_name curFileName expFileNameBase) == true then
				(
					-- Exported File Name
					expFileName = ligo_root_path + ligo_material_path + expFileNameBase[1] + ligo_material_ext
					if (getFiles expFileName).count != 0 then					
						expFileDate = getFileModDate expFileName
				)

				-- Get token
				tokenArray = filterString (getFilenameFile curFileName) "-"
				if (expFileDate != "") and (date_older_or_equal_than curFileDate expFileDate) then
				(
					append tmpArray ("[ok]"+tokenArray[2])
				)
				else
				(
					append tmpArray ("[..]"+tokenArray[2])
				)
			)
		)

		MaterialProject.items = tmpArray
	)

	on ExportAllMaterial pressed do
	(
		if (project_cur_file != "") then
			saveMaxFile project_cur_file
		
		refresh_list ()
		for curMat in MaterialProject.items do
		(
			minimise ()
			curFileName = ligo_root_path + ligo_material_prefix + (substring curMat 5 100) + ".max"
			loadMaxFile curFileName
			selectAllPatch ()
			export_and_check_and_debug_material false false false
			refresh_list ()
		)

		if (project_cur_file != "") then
			loadMaxFile project_cur_file
		Messagebox "Export All Material Template End" title:title_msgbox beep:true
	)

	on ExportAllMaterialNonOk pressed do
	(
		if (project_cur_file != "") then
			saveMaxFile project_cur_file
		
		refresh_list ()
		for curMat in MaterialProject.items do
		(
			minimise ()
			if (substring curMat 1 4) == "[..]" then
			(
				curFileName = ligo_root_path + ligo_material_prefix + (substring curMat 5 100) + ".max"
				loadMaxFile curFileName
				selectAllPatch ()
				export_and_check_and_debug_material false false false
				refresh_list ()
			)
		)

		if (project_cur_file != "") then
			loadMaxFile project_cur_file
		Messagebox "Export All Material Template Non-Ok End" title:title_msgbox beep:true
	)

	on ExportMaterial pressed do 
	(
		minimise ()
		if (project_cur_file != "") then
		(
			saveMaxFile project_cur_file
		)
		export_and_check_and_debug_material CheckOnly.checked Debug.checked true
		refresh_list ()
	)

	on DeleteDebugMarkers pressed do 
	(
		DeleteDebugMarkersFn ()
	)

	on ShowHelpZones pressed do
	(
		-- Get nelsize
		cellSize = NeLLigoGetCellSize ()

		-- Select a zone
		if ($ == undefined) or ($ == $selection) then
		(
			messageBox "Please select a 3ds NeL patch mesh object" title:title_msgbox beep:true
		)
		else
		(
			-- Make a refenrence
			r = instance $
			setappdata r ligo_instance_app "1"
			r.transform = translate (rotateZ (translate r.transform -r.pos) 0) r.pos
			r.pos += [cellSize,0,0]
			setappdata r ligo_rotate_app "0"
			l = instance $
			setappdata l ligo_instance_app "1"
			l.pos += [-cellSize,0,0]
			l.transform = translate (rotateZ (translate l.transform -l.pos) 180) l.pos
			setappdata l ligo_rotate_app "2"
			t = instance $
			setappdata t ligo_instance_app "1"
			t.pos += [0,cellSize,0]
			t.transform = translate (rotateZ (translate t.transform -t.pos) 90) t.pos
			setappdata t ligo_rotate_app "1"
			b = instance $
			setappdata b ligo_instance_app "1"
			b.pos += [0,-cellSize,0]
			b.transform = translate (rotateZ (translate b.transform -b.pos) -90) b.pos
			setappdata b ligo_rotate_app "3"

			r.name = "[NELLIGO] Right help zone"
			l.name = "[NELLIGO] Left help zone"
			t.name = "[NELLIGO] Top help zone"
			b.name = "[NELLIGO] Bottom help zone"
			freeze r
			freeze l
			freeze t
			freeze b
		)
	)

	on material_rollout open do
	(
		refresh_list ()
	)

	on DelMaterial pressed do
	(
		if (MaterialProject.selected != undefined) then
		(
			matName = substring MaterialProject.selected 5 100
			fileName = ligo_root_path + ligo_material_prefix + matName + ".max"
			if queryBox ("Do you want to delete "+fileName+" ?") beep:no then
			(
				deleteFile filename
				print ("deleting "+filename)
				-- delete the .ligomat file associated
				filename = ligo_root_path + ligo_material_path + matName + ligo_material_ext
				deleteFile filename
				print ("deleting "+filename)
				-- delete the .zone file associated
				filename = ligo_root_path + ligo_material_path + matName + nel_zone_ext
				deleteFile filename
				print ("deleting "+filename)
				refresh_project_list ()
				refresh_list
			)
		)
	)

	on AddMaterial pressed do
	(
		-- Get nelsize
		cellSize = NeLLigoGetCellSize ()

		-- Valid name ?
		if (NewMaterialName.text == "") then
		(
			Messagebox ("Choose a name for your new material") title:title_msgbox beep:true
		)
		else
		(
			-- Check for save
			if (checkForSave ()) == true then
			(
				-- Open a new project
				max file new

				-- Project name
				projectName = ligo_root_path+ligo_material_prefix+NewMaterialName.text+".max"

				-- Open the project
				if (loadMaxFile projectName) == false then
				(
					-- Create a patchgrid
					Quadpatch lengthsegs:5 widthsegs:5 length:cellSize width:cellSize pos:[cellSize/2,cellSize/2,0] isSelected:on

					-- Add a modifer to convert it
					max modify mode
					modPanel.addModToSelection (PatchObjectToNelPatchObject ())

					-- Collapse the node
					collapseStack $

					-- Set the tile step
					SetRykolTileSteps $ -2

					-- Save the max file
					saveMaxFile projectName
					loadMaxFile projectName
					project_cur_file = projectName

					-- Refresh max project
					refresh_project_list ()
					refresh_list ()
				)
			)
		)
	)

	on MaterialProject doubleClicked id do
	(
		-- Save current file
		--if (project_cur_file != "") then
		--	saveMaxFile project_cur_file

		if (checkForSave ()) == true then
		(
			-- Open the new file
			project_cur_file = substring MaterialProject.items[id] 5 100
			material_zone_rollout.MaterialDDL.selection = finditem material_zone_rollout.MaterialDDL.items project_cur_file
			material_zone_rollout.refresh_zone_list project_cur_file
			project_cur_file = ligo_root_path + ligo_material_prefix + project_cur_file + ".max"
			loadMaxFile (project_cur_file)
			refresh_list ()
		)
	)
)


rollout transition_rollout "Transition Template"
(
group "Check, debug and export"
	(

	Button		ExportAllTransition		"Export all transition"			width:140 align:#center
	Button		ExportAllTransNonOk		"Export all transition non-ok"	width:140 align:#center
	Button		ExportTransition		"Export transition template"	width:140 align:#center
	Button		HideOtherZones			"Hide other zones"				width:140 align:#center
	Button		UnhideOtherZones		"Unhide other zones"			width:140 align:#center
	Button		ShowHelpZones			"Show help zones"				width:140 align:#center
	Button		DeleteDebugMarkers		"Delete debug markers"			width:140 align:#center
	Button		AddPaintMod				"Add paint mod to visible"		width:140 align:#center
	Checkbox	CheckOnly				"Check only"					checked:false enabled:true
	Checkbox	Debug					"Debug"							checked:false enabled:true

	)

group "Projects management"
	(

	Listbox			TransitionProject		"Transition projects"			height:8 align:#center
	Button			DelTransition			"Delete selected transition"	width:140 align:#center
	Dropdownlist	NewTransitionName0DDL	"Material 0:"					width:140 align:#left
	Dropdownlist	NewTransitionName1DDL	"Material 1:"					width:140 align:#left
	Button			AddTransition			"Add transition"				width:140 align:#center

	)

	fn refresh_mat_list =
	(
		tmpArray = #()
		for proj in project_file_list do
		(
			tokenArray = filterString (getFilenameFile proj) "-"
			if (isMaterial proj) then
				append tmpArray tokenArray[2]
		)
		NewTransitionName0DDL.items = tmpArray
		NewTransitionName1DDL.items = tmpArray
	)

	fn refresh_list =
	(
		tmpArray = #()

		-- Make an array of zonematerial project
		for curFileName in project_file_list do
		(
			if (isTransition curFileName) == true then
			(
				-- Get token
				tokenArray = filterString (getFilenameFile curFileName) "-"
				curFileDate = getFileModDate curFileName

				-- exported file date
				expFileDate = ""
				-- expFileName = ligo_export_path + "zoneLigos\\" + tokenArray[1] + "-" + tokenArray[2] + ligo_zone_ext
				expFileName = ligo_root_path + ligo_transition_path + tokenArray[2] + "-" + tokenArray[3] + ligo_transition_ext

				if (getFiles expFileName).count != 0 then
					expFileDate = getFileModDate expFileName

				-- Get token
				tokenArray = filterString (getFilenameFile curFileName) "-"
				if (expFileDate != "") and (date_older_or_equal_than curFileDate expFileDate) then
				(
					append tmpArray ("[ok]" + tokenArray[2] + "-" + tokenArray[3])
				)
				else
				(
					append tmpArray ("[..]" + tokenArray[2] + "-" + tokenArray[3])
				)
			)
		)

		TransitionProject.items = tmpArray
	)

	on ExportAllTransition pressed do 
	(
		if (project_cur_file != "") then
			saveMaxFile project_cur_file

		refresh_list ()
		for curTrans in TransitionProject.items do
		(
			minimise ()
			curFileName = ligo_root_path + ligo_transition_prefix + (substring curtrans 5 100) + ".max"
			loadMaxFile curFileName
			selectAllPatch ()
			export_and_check_and_debug_transition_template false false false
			refresh_list ()
		)

		if (project_cur_file != "") then
			loadMaxFile project_cur_file
		Messagebox "Export All Transition Template End" title:title_msgbox beep:true
	)

	on ExportAllTransNonOk pressed do 
	(
		if (project_cur_file != "") then
			saveMaxFile project_cur_file

		refresh_list ()
		for curTrans in TransitionProject.items do
		(
			minimise ()
			if (substring curTrans 1 4) == "[..]" then
			(
				curFileName = ligo_root_path + ligo_transition_prefix + (substring curtrans 5 100) + ".max"
				loadMaxFile curFileName
				selectAllPatch ()
				export_and_check_and_debug_transition_template false false false
				refresh_list ()
			)
		)

		if (project_cur_file != "") then
			loadMaxFile project_cur_file
		Messagebox "Export All Transition Template Non-Ok End" title:title_msgbox beep:true
	)

	on ExportTransition pressed do 
	(
		minimise ()
		if (project_cur_file != "") then
			saveMaxFile project_cur_file
		export_and_check_and_debug_transition_template CheckOnly.checked Debug.checked true
		refresh_list ()
	)

	on HideOtherZones pressed do
	(
		-- For each node
		for node in geometry do
		(
			-- Is a NeL patch mesh ?
			if (classof node) == RklPatch then
			(
				-- Is not a debug marker ?
				if (isDebugMarker node) == false then
				(
					-- It is node selected ?
					if node.isSelected == false then
					(
						-- Hide it
						hide node
					)
				)
			)
		)
	)

	on UnhideOtherZones pressed do
	(
		-- For each node
		for node in geometry do
		(
			-- Is a NeL patch mesh ?
			if (classof node) == RklPatch then
			(
				-- Is not a debug marker ?
				if (isDebugMarker node) == false then
				(
					-- It is node selected ?
					if node.isSelected == false then
					(
						-- Hide it
						unhide node
					)
				)
			)
		)
	)

	on ShowHelpZones pressed do
	(
		-- Get nelsize
		cellSize = NeLLigoGetCellSize ()

		-- Select a file ?
		if $ == undefined then
		(
			messageBox "Please select a 3ds NeL patch mesh object" title:title_msgbox beep:true
		)
		else
		(
			if $ == $selection then
			(
				messageBox "Please select only one 3ds NeL patch mesh object" title:title_msgbox beep:true
			)
			else
			(
				-- Select the two materials
				materialName = #()
				ok = get_material_name_in_transition_name maxFileName materialName

				-- Make material file names with
				materialName[1] = ligo_root_path+ligo_material_path+materialName[1]+ligo_material_ext
				materialName[2] = ligo_root_path+ligo_material_path+materialName[2]+ligo_material_ext

				-- Selected ?
				if ok == true then
				(
					-- Get the zone coordinate
					x = #()
					y = #()
					if (getTransitionZoneCoordinates $ x y) == true then
					(
						-- Copy the pos
						posX = x[1]
						posY = y[1]

						-- Transition number
						id = TransitionIds[posY+1][posX+1]

						-- For the 4 edges
						for edge = 1 to 4 do
						(
							-- Our zone
							ref = undefined
							zone = undefined

							-- First transfor, to place good edge on the left
							firstTransfo = zone_transformation false 0

							-- Second transfor, to place good edge on the good destination edge
							secondTransfo = zone_transformation false 0

							-- Reference painter ?
							refPainter = false

							-- Translation to do
							trans= point3 0 0 0

							-- This is an instance
							instan = false

							-- What kind of edge
							if (EdgeType[id][edge]==1) or (EdgeType[id][edge]==-1) then
							(
								-- Load material one
								ref = NeLImportzone ((getFilenamePath materialName[1])+(getFilenameFile materialName[1])+nel_zone_ext) true
								refPainter = true
							)
							else if (EdgeType[id][edge]==2) or (EdgeType[id][edge]==-2) then
							(
								-- Load material two
								ref = NeLImportzone ((getFilenamePath materialName[2])+(getFilenameFile materialName[2])+nel_zone_ext) true
								refPainter = true
							)
							else if (EdgeType[id][edge]==3) or (EdgeType[id][edge]==-3) then
							(
								-- Reference transition 2
								zone = getTransition 2

								-- Rotation
								firstTransfo.symmetry = true
								firstTransfo.rotation = 1

								-- Translation
								trans = [-cellSize,0,0]

								-- The same ?
								if id == 2 then
									instan = true
							)
							else if (EdgeType[id][edge]==4) or (EdgeType[id][edge]==-4) then
							(
								-- Reference transition 1
								zone = getTransition 1

								-- Rotation
								firstTransfo.symmetry = true
								firstTransfo.rotation = 0

								-- The same ?
								if id == 1 then
									instan = true
							)

							-- Defined ?
							if zone != undefined then
							(
								-- Reference it
								ref = instance zone
								if instan == true then
									setappdata ref ligo_instance_app "1"
							)
							if ref != undefined then
							(
								-- New name
								ref.name = NeighborName[edge]

								-- Show it
								unhide ref

								-- Freeze it
								freeze ref
								SetRykolTileSteps ref -2

								-- Get an identity matrix
								mt = matrix3 1

								-- Rotation
								mt = translate mt ([-cellSize/2,-cellSize/2,0] + trans)

								if EdgeType[id][edge] >= 3 then
								(
									-- Destintation edge in inverted ? Symmetry
									secondTransfo = compose (zone_transformation false 3) (zone_transformation true 1) 
								)

								-- Final transformation
								secondTransfo = compose secondTransfo (zone_transformation false (NeighborDeltaRot[edge] / 90))
								finalTransfo = compose firstTransfo secondTransfo

								-- Final symmetry
								if finalTransfo.symmetry == true then
								(
									mt = scale mt [-1,1,1] true
								)

								-- Final rotation
								mt = rotateZ mt (90*finalTransfo.rotation)

								-- Set appdata
								if finalTransfo.symmetry == true then
									symmetryValue = 1
								else
									symmetryValue = 0
								setappdata ref ligo_symmetry_app (symmetryValue as string)
								setappdata ref ligo_rotate_app (finalTransfo.rotation as string)

								-- Final translation
								pos = (NeighborDeltaPos[edge] + [posX,posY,0]) * cellSize
								mt = translate mt ([cellSize/2,cellSize/2,0] + pos)

								-- Final rotation and position
								ref.transform = ref.transform * mt
							)
						)
					)
				)
			)
		)
	)

	on DeleteDebugMarkers pressed do 
	(
		DeleteDebugMarkersFn ()
	)

	on AddPaintMod pressed do 
	(
		AddPainterModToVisible ()
	)

	on transition_rollout open do
	(
		refresh_mat_list ()
		refresh_list ()
	)

	fn isValidTransition matName1 matName2 =
	(
		if (matName1 == "") or (matName2 == "") then
			return false
		if (matName1 == matName2) then
		(
			Messagebox ("Choose different names for first and second material.") title:title_msgbox beep:true
			return false
		)
		-- Does the transition already exist ?
		for i = 1 to 2 do
		(
			if (i == 1) then
				testName = uppercase (matName1 + "-" + matName2)
			else
				testName = uppercase (matName2 + "-" + matName1)
			for trans in TransitionProject.items do
			(
				testName2 = uppercase (substring trans 5 100)
				if (testName == testName2) then
				(
					Messagebox ("The transition already exists.") title:title_msgbox beep:true
					return false
				)
			)
		)
		return true
	)
	
	on DelTransition pressed do
	(
		if (TransitionProject.selected != undefined) then
		(
			transName = substring TransitionProject.selected 5 100
			fileName = ligo_root_path + ligo_transition_prefix + transName + ".max"
			if queryBox ("Do you want to delete "+fileName+" ?") beep:no then
			(
				deleteFile filename
				print ("deleting "+filename)
				-- delete the .ligotrans associated
				fileName = ligo_root_path + ligo_transition_path + transName + ligo_transition_ext
				deleteFile filename
				print ("deleting "+filename)
				-- delete the 9 .zone files
				for i = 0 to 8 do
				(
					fileName = ligo_root_path + ligo_transition_path + transName + "-" + (i as string) + nel_zone_ext
					deleteFile filename
					print ("deleting "+filename)
				)
				refresh_project_list ()
				refresh_list
			)
		)
	)

	on AddTransition pressed do
	(
		-- Get nelsize
		cellSize = NeLLigoGetCellSize ()

		-- Valid name ?
		if (isValidTransition NewTransitionName0DDL.selected NewTransitionName1DDL.selected) == true then
		(
			-- Check for save
			if (checkForSave ()) == true then
			(
				-- Open a new project
				max file new

				-- Project name
				projectName = ligo_root_path + ligo_transition_prefix + NewTransitionName0DDL.selected
				projectName = projectName + "-" + NewTransitionName1DDL.selected + ".max"

				-- Open the project
				if (loadMaxFile projectName) == false then
				(
					for i = 1 to 9 do
					(
						-- Zone transition
						position = (TransitionPosition[i]*cellSize) + [cellSize/2,cellSize/2,0]

						-- Create some patchgrid
						Quadpatch lengthsegs:5 widthsegs:5 length:cellSize width:cellSize pos:position isSelected:on

						-- Add a modifer to convert it
						max modify mode
						modPanel.addModToSelection (NeLConvert ())

						-- Collapse the node
						collapseStack $

						-- Set the tile step
						SetRykolTileSteps $ -2
					)

					-- Save the max file
					saveMaxFile projectName
					loadMaxFile projectName
					project_cur_file = projectName

					-- Refresh max project
					refresh_project_list ()
					refresh_list ()
				)
			)
		)
	)

	on TransitionProject doubleClicked id do
	(
		-- Check for save
		if (checkForSave ()) == true then
		(
			-- Open the max project
			project_cur_file = substring TransitionProject.items[id] 5 100
			transition_zone_rollout.TransitionDDL.selection = finditem transition_zone_rollout.TransitionDDL.items project_cur_file
			transition_zone_rollout.refresh_list project_cur_file
			project_cur_file = ligo_root_path + ligo_transition_prefix + project_cur_file + ".max"
			loadMaxFile project_cur_file
			refresh_list ()
		)
	)
)

rollout material_zone_rollout "Material Zone"
(
group "Check, debug and export"
	(

	Button		ExportAllZone			"Export all zone"				width:140 align:#center
	Button		ExportAllZoneNonOk		"Export all zone non-ok"		width:140 align:#center
	Button		ExportZone				"Export zone"					width:140 align:#center
	Button		ShowHelpZones			"Show help zones"				width:140 align:#center
	Button		DeleteDebugMarkers		"Delete debug markers"			width:140 align:#center
	Button		AddPaintMod				"Add paint mod to visible"		width:140 align:#center
	Checkbox	CheckOnly				"Check only"					checked:false enabled:true
	Checkbox	Debug					"Debug"							checked:false enabled:true

	)

group "Projects management"
	(

	Dropdownlist	MaterialDDL				"Material"				height:8 align:#center
	Listbox			ZoneProject				"Zones"					height:8 align:#center
	Button			DelZone					"Delete selected zone"	width:140 align:#center
	Label			ZoneName				"Zone name:"			align:#left
	EditText		NewZoneName				""						width:140 align:#left
	Button			AddZone					"Add zone"				width:140 align:#center
	Button			CopyZone				"Copy selected zone"	width:140 align:#center

	)

	fn refresh_mat_list =
	(
		tmpArray = #()
		append tmpArray "-- ALL --"
		for proj in project_file_list do
		(
			-- Get token
			tokenArray = filterString (getFilenameFile proj) "-"
			if (isMaterial proj) then
				append tmpArray tokenArray[2]
		)
		MaterialDDL.items = tmpArray
	)

	fn refresh_zone_list currentMaterial =
	(
		tmpArray = #()

		-- Make an array of zonematerial project
		for curFileName in project_file_list do
		(
			-- Get token
			tokenArray = filterString (getFilenameFile curFileName) "-"

			-- Valid name ?
			if (currentMaterial == "-- ALL --") or (tokenArray[2] == currentMaterial) then
			if (tokenArray.count == 3) and (tokenArray[1] == "zonematerial")  then
			(
				curFileDate = getFileModDate curFileName

				-- exported file date
				expFileDate = ""
				expFileNameBase = #()
				if (get_material_and_zone_name_in_zone_project_name curFileName expFileNameBase) == true then
				(					
					expFileName = ligo_export_path + "zoneLigos\\" + expFileNameBase[1] + "-" + expFileNameBase[2] + ligo_zone_ext
					if (getFiles expFileName).count != 0 then
						expFileDate = getFileModDate expFileName
				)

				-- Get token
				tokenArray = filterString (getFilenameFile curFileName) "-"
				if (expFileDate != "") and (date_older_or_equal_than curFileDate expFileDate) then
				(
					if (currentMaterial == "-- ALL --") then
						append tmpArray ("[ok]"+tokenArray[2]+"-"+tokenArray[3])
					else
						append tmpArray ("[ok]"+tokenArray[3])
				)
				else
				(
					if (currentMaterial == "-- ALL --") then
						append tmpArray ("[..]"+tokenArray[2]+"-"+tokenArray[3])
					else
						append tmpArray ("[..]"+tokenArray[3])
				)
			)
		)
		ZoneProject.items = tmpArray
	)

	on ExportAllZone pressed do
	(
		if (project_cur_file != "") then
			saveMaxFile project_cur_file

		refresh_zone_list MaterialDDL.selected
		for curZone in ZoneProject.items do
		(
			minimise ()
			curFileName = ligo_root_path + ligo_zonematerial_prefix
			if (MaterialDDL.selected != "-- ALL --") then
				curFileName = curFileName + MaterialDDL.selected + "-" 
			curFileName = curFileName + (substring curZone 5 100) + ".max"
			loadMaxFile curFileName
			selectAllPatch ()
			export_and_check_and_debug_zone false false false
			refresh_zone_list MaterialDDL.selected
		)

		if (project_cur_file != "") then
			loadMaxFile project_cur_file
		Messagebox "Export All Zone End" title:title_msgbox beep:true
	)

	on ExportAllZoneNonOk pressed do
	(
		if (project_cur_file != "") then
			saveMaxFile project_cur_file

		refresh_zone_list MaterialDDL.selected
		for curZone in ZoneProject.items do
		(
			minimise ()
			if (substring curZone 1 4) == "[..]" then
			(
				curFileName = ligo_root_path + ligo_zonematerial_prefix
				if (MaterialDDL.selected != "-- ALL --") then
					curFileName = curFileName + MaterialDDL.selected + "-" 
				curFileName = curFileName + (substring curZone 5 100) + ".max"
				loadMaxFile curFileName
				selectAllPatch ()
				export_and_check_and_debug_zone false false false
				refresh_zone_list MaterialDDL.selected
			)
		)

		if (project_cur_file != "") then
			loadMaxFile project_cur_file
		Messagebox "Export All Zone Non-Ok End" title:title_msgbox beep:true
	)

	on ExportZone pressed do 
	(
		minimise ()
		if (project_cur_file != "") then
			saveMaxFile project_cur_file
		export_and_check_and_debug_zone CheckOnly.checked Debug.checked true
		refresh_zone_list MaterialDDL.selected
	)

	on DeleteDebugMarkers pressed do 
	(
		-- Delete the markers
		DeleteDebugMarkersFn ()
	)

	on AddPaintMod pressed do 
	(
		AddPainterModToVisible ()
	)

	on ShowHelpZones pressed do
	(
		-- Get nelsize
		cellSize = NeLLigoGetCellSize ()

		-- Select a zone
		if ($ == undefined) or ($ == $selection) then
		(
			messageBox "Please select a 3ds NeL patch mesh object" title:title_msgbox beep:true
		)
		else
		(
			-- Array of index
			zoneUsed = #()
			zoneFilled = #()

			-- Get the mask
			-- resMinX = #()
			-- resMinY = #()
			resWidth = #()
			resHeight = #()
			-- if (NeLLigoGetZoneMask $ zoneUsed resMinX resMinY resWidth resHeight true) == true then
			if (NeLLigoGetZoneMask $ zoneUsed resWidth resHeight true) == true then
			(
				-- Width and height
				-- minx = resMinX[1]
				-- miny = resMinY[1]
				width = resWidth[1]
				height = resHeight[1]

				-- Number of vertices
				zoneCount = width*height

				-- Fill it
				for i=1 to zoneCount do
				(
					append zoneFilled false
				)

				-- Node matrix
				nodeTM = $.transform

				-- The imported zone
				imported = undefined

				-- For each cells
				for y = 0 to (height-1) do
				for x = 0 to (width-1) do
				(
					-- Id of the zone
					zoneId = x+y*width+1

					-- Already filled ?
					if ((zoneFilled[zoneId] == false) and (zoneUsed[zoneId] == false)) then
					(
						-- Ok ?
						ok = false

						-- No, must be filled ?
						for neighbor = 1 to 4 do
						(
							-- Delta pos
							deltaPos = NeighborDeltaPos[neighbor]

							-- New x and y
							newX = x+deltaPos.x
							newY = y+deltaPos.y

							-- Clip
							if ((newX >= 0) and (newY >= 0) and (newX < width) and (newY < height)) then
							(
								-- Neighbor id
								nId = newX+newY*width+1

								-- Good ?
								if (zoneUsed[nId] == true) then
								(
									-- Ok
									ok = true
									exit
								)
							)
						)

						-- Exited ?
						if (ok == true) then
						(
							-- Filled
							zoneFilled[zoneId] = true

							-- Impoted ?
							if (imported == undefined) then
							(
								-- Get the material file name
								result = #()
								get_material_and_zone_name_in_zone_project_name maxFileName result

								-- Material filename
								filename = ligo_root_path+ligo_material_path+result[1]+nel_zone_ext

								-- Import it
								imported = NeLImportZone filename true

								-- Import success ?
								if imported != undefined then
								(
									-- Imported transform
									imported.transform = translate imported.transform ([x-1, y-1, 0]*cellSize)
									importedTransform = transMatrix ([-x+1, -y+1, 0]*cellSize)
									
									-- It is our reference
									importedRef = imported
								)
								else
									importedRef = undefined
							)
							else
							(
								importedRef = instance imported
								setappdata importedRef ligo_instance_app "1"
							)

							-- Ok ?
							if importedRef != undefined then
							(
								-- New position of the zone
								importedRef.transform = translate (importedRef.transform*importedTransform) ([x-1, y-1, 0]*cellSize)

								-- New name
								importedRef.name = "[NELLIGO] edge zone"

								-- Freeze it
								freeze importedRef
								SetRykolTileSteps importedRef -2
							)
						)
					)
				)
			)
		)
	)

	on material_zone_rollout open do
	(
		refresh_mat_list ()
		refresh_zone_list MaterialDDL.selected
	)

	on DelZone pressed do
	(
		if (ZoneProject.selected != undefined) then
		(
			if (MaterialDDL.selected == "-- ALL --") then
				zoneNameTmp = (substring ZoneProject.selected 5 100)
			else
				zoneNameTmp = MaterialDDL.selected + "-" + (substring ZoneProject.selected 5 100)
			fileName = ligo_root_path + ligo_zonematerial_prefix + zoneNameTmp + ".max"
			if queryBox ("Do you want to delete "+fileName+" ?") beep:no then
			(
				deleteFile filename
				print ("deleting "+filename)
				-- delete the exported file .tga
				fileName = ligo_export_path + "zoneBitmaps\\" + zoneNameTmp + ".tga"
				deleteFile filename
				print ("deleting "+filename)
				-- delete the exported file .zone
				fileName = ligo_export_path + "zones\\" + zoneNameTmp + ".zone"
				deleteFile filename
				print ("deleting "+filename)
				-- delete the exported file .ligozone
				fileName = ligo_export_path + "zoneLigos\\" + zoneNameTmp + ".ligozone"
				deleteFile filename
				print ("deleting "+filename)

				refresh_project_list ()
				refresh_zone_list MaterialDDL.selected
			)
		)
	)

	on AddZone pressed do
	(
		-- Valid name ?
		if (NewZoneName.text == "")  then
		(
			Messagebox "Choose a name for your new zone." title:title_msgbox beep:true
		)
		else
		(
			if (MaterialDDL.selected == "-- ALL --") then
			(
				Messagebox "Please select a valid material template." title:title_msgbox beep:true
			)
			else
			(
				existingFile = ligo_root_path + ligo_material_prefix + MaterialDDL.selected + ".max"
				newFile = ligo_root_path + ligo_zonematerial_prefix + MaterialDDL.selected + "-" + NewZoneName.text + ".max"
				if (copyFile existingFile newFile) == false then
				(
					Messagebox ("Cannot create file :"+newFile) title:title_msgbox beep:true
				)
				else
				(
					loadMaxFile newFile
					project_cur_file = newFile
					refresh_project_list ()
					refresh_zone_list MaterialDDL.selected
				)
			)
		)
	)

	on CopyZone pressed do
	(
		if (NewZoneName.text == "")  then
		(
			Messagebox "Choose a name for your new zone." title:title_msgbox beep:true
		)
		else
		(
			if (ZoneProject.selection == 0) then
			(
				Messagebox "Please select a valid material zone." title:title_msgbox beep:true
			)
			else
			(
				if (MaterialDDL.selected == "-- ALL --") then
				(
					tokenArray = filterString (substring ZoneProject.selected 5 100) "-"
					existingFile = ligo_root_path + ligo_zonematerial_prefix + tokenArray[1] + "-" + tokenArray[2] + ".max"
					newFile = ligo_root_path + ligo_zonematerial_prefix + tokenArray[1] + "-" + NewZoneName.text + ".max"
				)
				else
				(
					fileName = substring ZoneProject.selected 5 100
					existingFile = ligo_root_path + ligo_zonematerial_prefix + MaterialDDL.selected + "-" + fileName + ".max"
					newFile = ligo_root_path + ligo_zonematerial_prefix + MaterialDDL.selected + "-" + NewZoneName.text + ".max"
				)
				print ("Copying "+existingfile+" to "+newfile)
				if (copyFile existingFile newFile) == false then
				(
					Messagebox ("Cannot create file :"+newFile) title:title_msgbox beep:true
				)
				else
				(
					loadMaxFile newFile
					project_cur_file = newFile
					refresh_project_list ()
					refresh_zone_list MaterialDDL.selected
				)
			)
		)
	)

	on ZoneProject doubleClicked id do
	(
		-- Check for save
		if (checkForSave ()) == true then
		(
			-- Open the max project
			if (MaterialDDL.selected == "-- ALL --") then
				project_cur_file = substring ZoneProject.items[id] 5 100
			else
				project_cur_file = MaterialDDL.selected + "-" + (substring ZoneProject.items[id] 5 100)
			project_cur_file = ligo_root_path + ligo_zonematerial_prefix + project_cur_file + ".max"
			loadMaxFile project_cur_file
			refresh_zone_list MaterialDDL.selected
		)
	)

	on MaterialDDL selected NewMaterialSelected do
	(
		refresh_zone_list MaterialDDL.selected
	)
)

rollout transition_zone_rollout "Transition Zone"
(
group "Check, debug and export"
	(

	Button		ExportAllTransZone		"Export all transZone"			width:140 align:#center
	Button		ExportAllTransZoneNonOk	"Export all transZone non-ok"	width:140 align:#center
	Button		ExportZone				"Export zone"					width:140 align:#center
	Button		HideOtherZones			"Hide other zones"				width:140 align:#center
	Button		UnhideOtherZones		"Unhide other zones"			width:140 align:#center
	Button		ShowHelpZones			"Show help zones"				width:140 align:#center
	Button		DeleteDebugMarkers		"Delete debug markers"			width:140 align:#center
	Button		AddPaintMod				"Add paint mod to visible"		width:140 align:#center
	Checkbox	CheckOnly				"Check only"					checked:false enabled:true
	Checkbox	Debug					"Debug"							checked:false enabled:true

	)

group "Projects management"
	(

	Dropdownlist	TransitionDDL			"Transition"					height:8 align:#center
	Listbox			ZoneProject				"transZone projects"			height:8 align:#center
	Button			DelTransZone			"Delete selected transZone"		width:140 align:#center
	Label			ZoneName				"transZone name:	"			align:#left
	EditText		NewTransZoneName		""								width:140 align:#left
	Button			AddTransZone			"Add transZone"					width:140 align:#center
	Button			CopyTransZone			"Copy selected transZone"		width:140 align:#center

	)

	fn refresh_trans_list =
	(
		tmpArray = #()
		append tmpArray "-- ALL --"
		for proj in project_file_list do
		(
			-- Get token
			tokenArray = filterString (getFilenameFile proj) "-"
			if (isTransition proj) then
				append tmpArray (tokenArray[2] + "-" + tokenArray[3])
		)
		TransitionDDL.items = tmpArray
	)

	fn refresh_list trans =
	(
		tmpArray = #()

		for curFileName in project_file_list do
		(
			-- Get token
			tokenArray = filterString (getFilenameFile curFileName) "-"

			-- Valid name ?
			if (tokenArray.count == 4) and (tokenArray[1] == "zonetransition") and
				((trans == "-- ALL --") or (trans == (tokenArray[2]+"-"+tokenArray[3]))) then
			(
				curFileDate = getFileModDate curFileName

				-- exported file date
				isUpToDate = true
				for i = 0 to 8 do
				(
					expFileDate = ""
					expFileName = ligo_export_path + "zoneLigos\\" + tokenArray[2] + "-" + tokenArray[3]
					expFileName = expFileName + "-" + tokenArray[4] + "-" + (i as string) + ligo_zone_ext
					if (getFiles expFileName).count != 0 then
						expFileDate = getFileModDate expFileName
					if not ((expFileDate != "") and (date_older_or_equal_than curFileDate expFileDate)) then
						isUpToDate = false
				)

				if (isUpToDate == true) then
				(
					if (trans == "-- ALL --") then
						append tmpArray ("[ok]" + tokenArray[2] + "-" + tokenArray[3] + "-" + tokenArray[4])
					else
						append tmpArray ("[ok]" + tokenArray[4])
				)
				else
				(
					if (trans == "-- ALL --") then
						append tmpArray ("[..]" + tokenArray[2] + "-" + tokenArray[3] + "-" + tokenArray[4])
					else
						append tmpArray ("[..]" + tokenArray[4])
				)
			)
		)

		ZoneProject.items = tmpArray
	)

	on ExportAllTransZone pressed do
	(
		if (project_cur_file != "") then
			saveMaxFile project_cur_file

		refresh_list TransitionDDL.selected
		for curTrans in ZoneProject.items do
		(
			minimise ()
			curFileName = ligo_root_path + ligo_zonetransition_prefix
			if (TransitionDDL.selected != "-- ALL --") then
				curFileName = curFileName + TransitionDDL.selected + "-" 
			curFileName = curFileName + (substring curTrans 5 100) + ".max"
			loadMaxFile curFileName
			selectAllPatch ()
			export_and_check_and_debug_transition_zone false false false
			refresh_list TransitionDDL.selected
		)

		if (project_cur_file != "") then
			loadMaxFile project_cur_file
		Messagebox "Export All TransZone End" title:title_msgbox beep:true
	)

	on ExportAllTransZoneNonOk pressed do
	(
		if (project_cur_file != "") then
			saveMaxFile project_cur_file

		refresh_list TransitionDDL.selected
		for curTrans in ZoneProject.items do
		(
			minimise ()
			if (substring curTrans 1 4) == "[..]" then
			(
				curFileName = ligo_root_path + ligo_zonetransition_prefix
				if (TransitionDDL.selected != "-- ALL --") then
					curFileName = curFileName + TransitionDDL.selected + "-" 
				curFileName = curFileName + (substring curTrans 5 100) + ".max"
				loadMaxFile curFileName
				selectAllPatch ()
				export_and_check_and_debug_transition_zone false false false
				refresh_list TransitionDDL.selected
			)
		)

		if (project_cur_file != "") then
			loadMaxFile project_cur_file
		Messagebox "Export All TransZone Non-Ok End" title:title_msgbox beep:true
	)

	on ExportZone pressed do 
	(
		minimise ()
		if (project_cur_file != "") then
			saveMaxFile project_cur_file
		export_and_check_and_debug_transition_zone CheckOnly.checked Debug.checked true
		refresh_list TransitionDDL.selected
	)

	on DeleteDebugMarkers pressed do 
	(
		-- Delete the markers
		DeleteDebugMarkersFn ()
	)

	on AddPaintMod pressed do 
	(
		AddPainterModToVisible ()
	)

	on ShowHelpZones pressed do
	(
		-- Get nelsize
		cellSize = NeLLigoGetCellSize ()

		-- Select a file ?
		if $ == undefined then
		(
			messageBox "Please select a 3ds NeL patch mesh object" title:title_msgbox beep:true
		)
		else
		(
			if $ == $selection then
			(
				messageBox "Please select only one 3ds NeL patch mesh object" title:title_msgbox beep:true
			)
			else
			(
				-- Select the two materials
				materialName = #()
				ok = get_transition_and_zone_name_in_zone_project_name maxFileName materialName

				-- Make material file names with
				materialName[1] = ligo_root_path+ligo_material_path+materialName[1]+ligo_material_ext
				materialName[2] = ligo_root_path+ligo_material_path+materialName[2]+ligo_material_ext

				-- Selected ?
				if ok == true then
				(
					-- Get the zone coordinate
					x = #()
					y = #()
					if (getTransitionZoneCoordinates $ x y) == true then
					(
						-- Copy the pos
						posX = x[1]
						posY = y[1]

						-- Transition number
						id = TransitionIds[posY+1][posX+1]

						-- For the 4 edges
						for edge = 1 to 4 do
						(
							-- Our zone
							ref = undefined

							-- First transfor, to place good edge on the left
							firstTransfo = zone_transformation false 0

							-- Second transfor, to place good edge on the good destination edge
							secondTransfo = zone_transformation false 0

							-- Translation to do
							trans= point3 0 0 0

							-- What kind of edge
							if (EdgeType[id][edge]==1) or (EdgeType[id][edge]==-1) then
							(
								-- Load material one
								ref = NeLImportzone ((getFilenamePath materialName[1])+(getFilenameFile materialName[1])+nel_zone_ext) true
							)
							else if (EdgeType[id][edge]==2) or (EdgeType[id][edge]==-2) then
							(
								-- Load material two
								ref = NeLImportzone ((getFilenamePath materialName[2])+(getFilenameFile materialName[2])+nel_zone_ext) true
							)
							else if (EdgeType[id][edge]==3) or (EdgeType[id][edge]==-3) then
							(
								-- Reference transition 2
								ref = NeLImportzone (ligo_root_path+ligo_transition_path+(getFilenameFile materialName[1])+"-"+(getFilenameFile materialName[2])+"-1"+nel_zone_ext) true

								-- Rotation
								-- Symmetry
								firstTransfo.symmetry = true
								firstTransfo.rotation = 1

								-- Translation
								trans = [-cellSize,0,0]
							)
							else if (EdgeType[id][edge]==4) or (EdgeType[id][edge]==-4) then
							(
								-- Reference transition 1
								ref = NeLImportzone (ligo_root_path+ligo_transition_path+(getFilenameFile materialName[1])+"-"+(getFilenameFile materialName[2])+"-0"+nel_zone_ext) true

								-- Rotation
								-- Symmetry
								firstTransfo.symmetry = true
								firstTransfo.rotation = 0
							)

							if ref != undefined then
							(
								-- New name
								ref.name = NeighborName[edge]

								-- Show it
								unhide ref

								-- Freeze it
								freeze ref

								-- Set the tile set
								SetRykolTileSteps ref -2

								-- Get an identity matrix
								mt = matrix3 1

								-- Rotation
								mt = translate mt ([-cellSize/2,-cellSize/2,0] + trans)

								if EdgeType[id][edge] >= 3 then
								(
									-- Destintation edge in inverted ? Symmetry
									secondTransfo = compose (zone_transformation false 3) (zone_transformation true 1) 
								)

								-- Final transformation
								secondTransfo = compose secondTransfo (zone_transformation false (NeighborDeltaRot[edge] / 90))
								finalTransfo = compose firstTransfo secondTransfo

								-- Final symmetry
								if finalTransfo.symmetry == true then
								(
									mt = scale mt [-1,1,1] true
								)

								-- Final rotation
								mt = rotateZ mt (90*finalTransfo.rotation)

								-- Set appdata
								if finalTransfo.symmetry == true then
									symmetryValue = 1
								else
									symmetryValue = 0
								setappdata ref ligo_symmetry_app (symmetryValue as string)
								setappdata ref ligo_rotate_app (finalTransfo.rotation as string)

								-- Final translation
								pos = (NeighborDeltaPos[edge] + [posX,posY,0]) * cellSize
								mt = translate mt ([cellSize/2,cellSize/2,0] + pos)

								-- Final rotation and position
								ref.transform = ref.transform * mt
							)
						)
					)
				)
			)
		)
	)

	on transition_zone_rollout open do
	(
		refresh_trans_list ()
		refresh_list TransitionDDL.selected
	)

	on DelTransZone pressed do
	(
		if (ZoneProject.selected != undefined) then
		(
			if (TransitionDDL.selected == "-- ALL --") then
				transZoneTmp = (substring ZoneProject.selected 5 100)
			else
				transZoneTmp = TransitionDDL.selected + "-" + (substring ZoneProject.selected 5 100)
			fileName = ligo_root_path + ligo_zonetransition_prefix + transZoneTmp + ".max"
			if queryBox ("Do you want to delete "+fileName+" ?") beep:no then
			(
				deleteFile filename
				print ("deleting "+filename)
				-- delete the 9 .tga, .zone and .ligozone files
				for i = 0 to 8 do
				(
					-- delete exported file associated : .tga
					fileName = ligo_export_path + "zoneBitmaps\\" + transZoneTmp + "-" + (i as string) +  ".tga"
					deleteFile filename
					print ("deleting "+filename)

					fileName = ligo_export_path + "zones\\" + transZoneTmp + "-" + (i as string) + ".zone"
					deleteFile filename
					print ("deleting "+filename)

					fileName = ligo_export_path + "zoneLigos\\" + transZoneTmp + "-" + (i as string) + ".ligozone"
					deleteFile filename
					print ("deleting "+filename)
				)
				refresh_project_list ()
				refresh_list TransitionDDL.selected
			)
		)
	)

	on AddTransZone pressed do
	(
		-- Valid name ?
		if (NewTransZoneName.text == "")  then
		(
			Messagebox "Choose a name for your new transition zone." title:title_msgbox beep:true
		)
		else
		(
			if (TransitionDDL.selected == "-- ALL --") then
			(
				Messagebox "Please select a valid transition template." title:title_msgbox beep:true
			)
			else
			(
				existingFile = ligo_root_path + ligo_transition_prefix + TransitionDDL.selected + ".max"
				newFile = ligo_root_path + ligo_zonetransition_prefix + TransitionDDL.selected + "-" + NewTransZoneName.text + ".max"
				if (copyFile existingFile newFile) == false then
				(
					Messagebox ("Cannot create file :"+newFile) title:title_msgbox beep:true
				)
				else
				(
					loadMaxFile newFile
					project_cur_file = newFile
					refresh_project_list ()
					refresh_list TransitionDDL.selected
				)
			)
		)
	)

	on CopyTransZone pressed do
	(
		if (NewTransZoneName.text == "")  then
		(
			Messagebox "Choose a name for your new transZone." title:title_msgbox beep:true
		)
		else
		(
			if (ZoneProject.selection == 0) then
			(
				Messagebox "Please select a valid transition zone." title:title_msgbox beep:true
			)
			else
			(
				existingFile = ligo_root_path + ligo_zonetransition_prefix
				if (TransitionDDL.selected == "-- ALL --") then
				(
					tokenArray = filterString (substring ZoneProject.selected 5 100) "-"
					existingFile = existingFile + tokenArray[1] + "-" + tokenArray[2] + "-"
					newFile = existingFile
					existingFile = existingFile + tokenArray[3] + ".max"
					newFile = newFile + NewTransZoneName.text + ".max"
				)
				else
				(
					fileName = substring ZoneProject.selected 5 100
					existingFile = existingFile + TransitionDDL.selected + "-" 
					newFile  = existingFile
					existingFile = existingFile + fileName + ".max"
					newFile = newFile + NewTransZoneName.text + ".max"
				)
				print ("Copying "+existingfile+" to "+newfile)
				if (copyFile existingFile newFile) == false then
				(
					Messagebox ("Cannot create file :"+newFile) title:title_msgbox beep:true
				)
				else
				(
					loadMaxFile newFile
					project_cur_file = newFile
					refresh_project_list ()
					refresh_list TransitionDDL.selected
				)
			)
		)
	)

	on ZoneProject doubleClicked id do
	(
		-- Check for save
		if (checkForSave ()) == true then
		(
			-- Open the max project
			if (TransitionDDL.selected == "-- ALL --") then
				project_cur_file = substring ZoneProject.items[id] 5 100
			else
				project_cur_file = transitionDDL.selected + "-" + (substring ZoneProject.items[id] 5 100)
			project_cur_file = ligo_root_path + ligo_zonetransition_prefix + project_cur_file + ".max"
			loadMaxFile project_cur_file
		)
	)

	on TransitionDDL selected NewTransitionSelected do
	(
		refresh_list TransitionDDL.selected
	)

	on HideOtherZones pressed do
	(
		-- For each node
		for node in geometry do
		(
			-- Is a NeL patch mesh ?
			if (classof node) == RklPatch then
			(
				-- Is not a debug marker ?
				if (isDebugMarker node) == false then
				(
					-- It is node selected ?
					if node.isSelected == false then
					(
						-- Hide it
						hide node
					)
				)
			)
		)
	)

	on UnhideOtherZones pressed do
	(
		-- For each node
		for node in geometry do
		(
			-- Is a NeL patch mesh ?
			if (classof node) == RklPatch then
			(
				-- Is not a debug marker ?
				if (isDebugMarker node) == false then
				(
					-- It is node selected ?
					if node.isSelected == false then
					(
						-- Hide it
						unhide node
					)
				)
			)
		)
	)
)

rollout special_zone_rollout "Special Zone"
(
group "Check, debug and export"
	(

	Button		ExportAllZone			"Export all zone"				width:140 align:#center
	Button		ExportAllZoneNonOk		"Export all zone non-ok"		width:140 align:#center
	Button		ExportZone				"Export zone"					width:140 align:#center
	Checkbox	CheckOnly				"Check only"					checked:false enabled:true
	Checkbox	Debug					"Debug"							checked:false enabled:true

	)

group "Projects management"
	(

	Listbox			ZoneProject				"Zones"					height:8 align:#center
	Button			DelZone					"Delete selected zone"	width:140 align:#center
	Label			ZoneName				"Zone name:"			align:#left
	EditText		NewZoneName				""						width:140 align:#left
	Button			AddZone					"Add zone"				width:140 align:#center

	)

	fn refresh_zone_list =
	(
		tmpArray = #()

		-- Make an array of zonespecial project
		for curFileName in project_file_list do
		(
			-- Get token
			tokenArray = filterString (getFilenameFile curFileName) "-"

			-- Valid name ?
			if (tokenArray.count == 2) and (tokenArray[1] == "zonespecial")  then
			(
				curFileDate = getFileModDate curFileName

				-- exported file date
				expFileDate = ""
				expFileName = ligo_export_path + "zoneLigos\\" + tokenArray[2] + ligo_zone_ext
				if (getFiles expFileName).count != 0 then
					expFileDate = getFileModDate expFileName

				-- Get token
				tokenArray = filterString (getFilenameFile curFileName) "-"
				if (expFileDate != "") and (date_older_or_equal_than curFileDate expFileDate) then
				(
					append tmpArray ("[ok]"+tokenArray[2])
				)
				else
				(
					append tmpArray ("[..]"+tokenArray[2])
				)
			)
		)
		ZoneProject.items = tmpArray
	)

	on ExportAllZone pressed do
	(
		if (project_cur_file != "") then
			saveMaxFile project_cur_file

		refresh_project_list ()
		refresh_zone_list ()
		for curZone in ZoneProject.items do
		(
			minimise ()
			curFileName = ligo_root_path + ligo_zonespecial_prefix
			curFileName = curFileName + (substring curZone 5 100) + ".max"
			loadMaxFile curFileName
			selectAllPatch ()
			export_and_check_and_debug_special_zone false false false
			refresh_project_list ()
			refresh_zone_list ()
		)

		if (project_cur_file != "") then
			loadMaxFile project_cur_file
		Messagebox "Export All Zone End" title:title_msgbox beep:true
	)

	on ExportAllZoneNonOk pressed do
	(
		if (project_cur_file != "") then
			saveMaxFile project_cur_file

		refresh_project_list ()
		refresh_zone_list ()
		for curZone in ZoneProject.items do
		(
			minimise ()
			if (substring curZone 1 4) == "[..]" then
			(
				curFileName = ligo_root_path + ligo_zonespecial_prefix
				curFileName = curFileName + (substring curZone 5 100) + ".max"
				loadMaxFile curFileName
				selectAllPatch ()
				export_and_check_and_debug_special_zone false false false
				refresh_project_list ()
				refresh_zone_list ()
			)
		)

		if (project_cur_file != "") then
			loadMaxFile project_cur_file
		Messagebox "Export All Zone Non-Ok End" title:title_msgbox beep:true
	)

	on ExportZone pressed do 
	(
		minimise ()
		if (project_cur_file != "") then
			saveMaxFile project_cur_file
		export_and_check_and_debug_special_zone CheckOnly.checked Debug.checked true
		refresh_project_list ()
		refresh_zone_list ()
	)

	on special_zone_rollout open do
	(
		refresh_project_list ()
		refresh_zone_list ()
	)

	on DelZone pressed do
	(
		if (ZoneProject.selected != undefined) then
		(
			zoneNameTmp = (substring ZoneProject.selected 5 100)
			fileName = ligo_root_path + ligo_zonespecial_prefix + zoneNameTmp + ".max"
			if queryBox ("Do you want to delete "+fileName+" ?") beep:no then
			(
				deleteFile filename
				print ("deleting "+filename)
				-- delete the exported file .tga
				fileName = ligo_export_path + "zoneBitmaps\\" + zoneNameTmp + ".tga"
				deleteFile filename
				print ("deleting "+filename)
				-- delete the exported file .zone
				fileName = ligo_export_path + "zones\\" + zoneNameTmp + ".zone"
				deleteFile filename
				print ("deleting "+filename)
				-- delete the exported file .ligozone
				fileName = ligo_export_path + "zoneLigos\\" + zoneNameTmp + ".ligozone"
				deleteFile filename
				print ("deleting "+filename)

				refresh_project_list ()
				refresh_zone_list ()
			)
		)
	)

	on AddZone pressed do
	(
		-- Valid name ?
		if (NewZoneName.text == "")  then
		(
			Messagebox "Choose a name for your new zone." title:title_msgbox beep:true
		)
		else
		(
			newFile = ligo_root_path + ligo_zonespecial_prefix + NewZoneName.text + ".max"
			if (saveMaxFile newFile) == false then
			(
				Messagebox ("Cannot create file :"+newFile) title:title_msgbox beep:true
			)
			else
			(
				project_cur_file = newFile
				refresh_project_list ()
				refresh_zone_list ()
			)
		)
	)

	on ZoneProject doubleClicked id do
	(
		-- Check for save
		if (checkForSave ()) == true then
		(
			-- Open the max project
			project_cur_file = substring ZoneProject.items[id] 5 100
			project_cur_file = ligo_root_path + ligo_zonespecial_prefix + project_cur_file + ".max"
			loadMaxFile project_cur_file
			refresh_project_list ()
			refresh_zone_list ()
		)
	)
)

rollout properties_rollout "Properties"
(
group "Flags"
	(
		Checkbox		Passable				"Passable"				checked:false enabled:false
	)

	fn refreshProperties =
	(
		if (($ != undefined) and ($ != $selection)) then
		(
			-- Get the passable appdata
			Passable.enabled = true
			Passable.checked = ((getappdata $ ligo_passable_app) != undefined)
		)
		else
		(
			Passable.enabled = false
		)
	)

	on Passable changed state do
	(
		if (($ != undefined) and ($ != $selection)) then
		(
			-- Get the passable appdata
			if Passable.checked then
				setappdata $ ligo_passable_app "1"
			else
				deleteappdata $ ligo_passable_app 
		)
		else
		(
			Passable.enabled = false
		)
	)

	on properties_rollout open do
	(
		refreshProperties ()
	)
)

rollout options_rollout "Options"
(
group "Directories"
	(
		Label			LigoPathLabel			"Ligo Root Path"		align:#left
		EditText		LigoPath				""						width:140 align:#center
		Label			ExportPathLabel			"Ligo Export Path"		align:#left
		EditText		LigoExportPath			""						width:140 align:#center		
		Label			OldZonePathPathLabel	"Old Zone Path"			align:#left
		EditText		OldZonePath				""						width:140 align:#center		
		Button 			SetupDirectories		"Setup directories"		width:140 align:#center
		Button 			ExportOldZonesTga		"Export old zones TGA"	width:140 align:#center
	)

	on SetupDirectories pressed do
	(
		makedir ligo_root_path
		makedir (ligo_root_path+"\\materials")
		makedir (ligo_root_path+"\\transitions")
		makedir (ligo_export_path+"\\zones")
		makedir (ligo_export_path+"\\zonebitmaps")
		makedir (ligo_export_path+"\\zoneLigos")
	)

	fn findID node =
	(
		local

		-- Const
		alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZ" 
		NameTab = filterString node.name "_"
		if (NameTab != undefined) and (NameTab.count == 2) then 
		(
			Z_ID = -1
			alpha_letter1 = NameTab[2][1]
			alpha_letter2 = NameTab[2][2]
			alpha_letter1_value = findstring alphabet alpha_letter1 
			alpha_letter2_value = findstring alphabet alpha_letter2 

			-- Decompose theh name in an array array[1]=numeric string value  array[2]=alpha string value
			-- The index of the engine start at 0 but the script one at 1 so we sub 1 each time
			alpha_sub_id = (((alpha_letter1_value as integer - 1) * 26 + (alpha_letter2_value as integer)))-1
			num_sub_id = (NameTab[1] as integer)-1

				-- Array of 256 per 256
				---------------------------
				--	0	1	2	3	...	255
				--	256	257	258	259	...	511 
				--	512	513	514	515	...	767
				--	...

			Z_ID = num_sub_id*256 + alpha_sub_id

			return Z_ID
		)
		else
			return 0
	)

	fn idToCoord id coord =
	(
		coord[2] = - floor (id/256)
		coord[1] = id + (coord[2]*256)
		coord[2] = coord[2] - 1
	)

	on ExportOldZonesTga pressed do
	(
		makedir ligo_root_path
		makedir (ligo_root_path+"\\old_zones_tga")

		-- Get files in the zone_source_directory
		files = getFiles (old_zone_path+"*.max")

		-- Sort files
		sort files

		-- No file ?
		if files.count != 0 then
		(
			-- For each files
			for i = 1 to files.count do
			(
				-- Ok ?
				ok = false

				-- Output file
				output = ligo_root_path+"old_zones_tga\\"+(getFilenameFile files[i])+".tga"

				-- Compare file date
				if (NeLTestFileDate output files[i]) == true then
				(
					-- Open the max project
					if loadMaxFile files[i] == true then
					(
						-- Select none
						max select none

						-- For each object in the priject
						for i in geometry do
						(
							-- Look for a NeL patch mesh
							if (classof i)==RklPatch then
							(
								-- Get coord with the name
								coord = #()
								idToCoord (findID i) coord
								NeLLigoMakeSnapShot i output (coord[1] as integer) ((coord[1] as integer) + 1) (coord[2] as integer) ((coord[2] as integer) + 1) true
							)
						)
					)
				)
			)
		)
	)

	on LigoPath changed newPath do
	(
		ligo_root_path = newPath
		lastChar = ligo_root_path[ligo_root_path.count]
		if (lastChar != "\\") and (lastChar != "/") then
			ligo_root_path += "\\"
		setINISetting ((GetDir #plugcfg)+"\\nelligo.ini") "LigoConfig" "LigoPath" ligo_root_path
	)

	on LigoExportPath changed newPath do
	(
		ligo_export_path = newPath
		lastChar = ligo_export_path[ligo_export_path.count]
		if (lastChar != "\\") and (lastChar != "/") then
			ligo_export_path += "\\"
		setINISetting ((GetDir #plugcfg)+"\\nelligo.ini") "LigoConfig" "LigoExportPath" ligo_export_path
	)

	on OldZonePath changed newPath do
	(
		old_zone_path = newPath
		lastChar = old_zone_path[old_zone_path.count]
		if (lastChar != "\\") and (lastChar != "/") then
			old_zone_path += "\\"
		setINISetting ((GetDir #plugcfg)+"\\nelligo.ini") "LigoConfig" "LigoOldZonePath" old_zone_path
	)

	on options_rollout open do
	(
		LigoPath.text = ligo_root_path
		LigoExportPath.text = ligo_export_path
		OldZonePath.text = old_zone_path
	)

)

rollout exporter_rollout "Exporter"
(
	Button			ExpAll				"Export All"					width:140 align:#center
	Button			ExpAllMaterialTemp	"Export Material Template"		width:140 align:#center
	Button			ExpAllTransTemp		"Export Transition Template"	width:140 align:#center
	Button			ExpAllZone			"Export Zone"					width:140 align:#center
	Button			ExpAllTransZone		"Export TransZone"				width:140 align:#center

	fn expMatTemp =
	(
		for curFileName in project_file_list do
		(
			if (isMaterial curFileName) == true then
			(
				loadMaxFile curFileName
				selectAllPatch ()
				print ("Exporting "+ (getFilenameFile curFileName))
				export_and_check_and_debug_material false false false
			)
		)
	)

	fn expTransTemp =
	(
		for curFileName in project_file_list do
		(
			if (isTransition curFileName) == true then
			(
				loadMaxFile curFileName
				selectAllPatch ()
				print ("Exporting "+ (getFilenameFile curFileName))
				export_and_check_and_debug_transition_template false false false
			)
		)
	)

	fn expZone =
	(
		for curFileName in project_file_list do
		(
			tokenArray = filterString (getFilenameFile curFileName) "-"
			if (tokenArray.count == 3) and (tokenArray[1] == "zonematerial")  then
			(
				loadMaxFile curFileName
				selectAllPatch ()
				print ("Exporting "+ (getFilenameFile curFileName))
				export_and_check_and_debug_zone false false false
			)
		)
	)

	fn expTransZone =
	(
		for curFileName in project_file_list do
		(
			tokenArray = filterString (getFilenameFile curFileName) "-"
			if (tokenArray.count == 4) and (tokenArray[1] == "zonetransition") then
			(
				loadMaxFile curFileName
				selectAllPatch ()
				print ("Exporting "+ (getFilenameFile curFileName))
				export_and_check_and_debug_transition_zone false false false
			)
		)
	)

	on ExpAll pressed do
	(
		if (project_cur_file != "") then
			saveMaxFile project_cur_file

		expMatTemp ()
		expTransTemp ()
		expZone ()
		expTransZone ()

		if (project_cur_file != "") then
			loadMaxFile project_cur_file
		print "Export Finished"
		Messagebox "Export All End" title:title_msgbox beep:true
		refresh_project_list ()
	)

	on ExpAllMaterialTemp pressed do
	(
		if (project_cur_file != "") then
			saveMaxFile project_cur_file

		expMatTemp ()

		if (project_cur_file != "") then
			loadMaxFile project_cur_file
		print "Export Finished"
		Messagebox "Export All Material Template End" title:title_msgbox beep:true
		refresh_project_list ()
	)

	on ExpAllTransTemp pressed do
	(
		if (project_cur_file != "") then
			saveMaxFile project_cur_file

		expTransTemp ()

		if (project_cur_file != "") then
			loadMaxFile project_cur_file
		print "Export Finished"
		Messagebox "Export All Transition Template End" title:title_msgbox beep:true
		refresh_project_list ()
	)

	on ExpAllZone pressed do
	(
		if (project_cur_file != "") then
			saveMaxFile project_cur_file

		expZone ()

		if (project_cur_file != "") then
			loadMaxFile project_cur_file
		print "Export Finished"
		Messagebox "Export All Zone End" title:title_msgbox beep:true
		refresh_project_list ()
	)
	
	on ExpAllTransZone pressed do
	(
		if (project_cur_file != "") then
			saveMaxFile project_cur_file

		expTransZone ()

		if (project_cur_file != "") then
			loadMaxFile project_cur_file
		print "Export Finished"
		Messagebox "Export All TransZone End" title:title_msgbox beep:true
		refresh_project_list ()
	)
)

-- ========== --
-- Main entry --
-- ========== --

if ligoscape_tools_floater != undefined do
(
	closerolloutfloater ligoscape_tools_floater
)

-- Get the ligo path
ligopath = getINISetting ((GetDir #plugcfg)+"\\nelligo.ini") "LigoConfig" "LigoPath" 
if ligopath != "" then ligo_root_path = ligopath
ligopath = getINISetting ((GetDir #plugcfg)+"\\nelligo.ini") "LigoConfig" "LigoExportPath" 
if ligopath != "" then ligo_export_path = ligopath
ligopath = getINISetting ((GetDir #plugcfg)+"\\nelligo.ini") "LigoConfig" "LigoOldZonePath" 
if ligopath != "" then old_zone_path = ligopath

ligoscape_tools_floater = newRolloutFloater "NeL Ligoscape" 225 600

refresh_project_list ()

addrollout material_rollout ligoscape_tools_floater rolledUp:true
addrollout transition_rollout ligoscape_tools_floater rolledUp:true
addrollout material_zone_rollout ligoscape_tools_floater rolledUp:true
addrollout transition_zone_rollout ligoscape_tools_floater rolledUp:true
addrollout special_zone_rollout ligoscape_tools_floater rolledUp:true
addrollout properties_rollout ligoscape_tools_floater rolledUp:true
addrollout options_rollout ligoscape_tools_floater rolledUp:true
addrollout exporter_rollout ligoscape_tools_floater rolledUp:true


fn updataCallBack =
(
	properties_rollout.refreshProperties ()
)

callbacks.addScript #filePostOpen "updataCallBack ()" 
callbacks.addScript #selectionSetChanged  "updataCallBack ()" 
