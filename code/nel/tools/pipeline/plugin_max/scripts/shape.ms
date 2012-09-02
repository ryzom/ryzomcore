

-- Some globals

NEL3D_APPDATA_ACCEL = 1423062561			-- type of accelerator :	"32" = is not an accelerator and IS clusterized 
											--							 "0" = is not an accelerator and IS NOT clusterized (always visible)
											--							 "1" = is an accelerator type PORTAL
											--							 "2" = is an accelerator type CLUSTER
											--							 "6" = is an accelerator type CLUSTER FATHER-VISIBLE
											--							"10" = is an accelerator type CLUSTER VISIBLE-FROM-FATHER
											--							"14" = is an accelerator type CLUSTER FATHER-VISIBLE and VISIBLE-FROM-FATHER
											--							"17" = is an accelerator type PORTAL DYNAMIC
																			 
NEL3D_APPDATA_DONOTEXPORT = 1423062565		-- do not export me : "undefined" = export me
											--							  "0" = export me
											--							  "1" = DONT export me

NEL3D_APPDATA_COLLISION				= 1423062613
NEL3D_APPDATA_COLLISION_EXTERIOR	= 1423062614
NEL3D_APPDATA_AUTOMATIC_ANIMATION	= 1423062617

-- This node is n accelerator ?
fn isAccelerator node =
(
	accel = getappdata node NEL3D_APPDATA_ACCEL
	if (accel != undefined) then
	(
		if (accel == "0") or (accel == "32") then
			return false
		else
			return true
	)
	return false
)

-- Must export this node ?
fn isToBeExported node =
(
	if (isAccelerator node) == true then
		return false
	
	if ((classof node) == RklPatch) then
		return false
	
	if ((classof node) == nel_ps) then
		return false
	
	if ((classof node) == nel_pacs_cylinder) then
		return false
	
	if ((classof node) == nel_pacs_box) then
		return false
	
	doNotExport = getappdata node NEL3D_APPDATA_DONOTEXPORT
	if (doNotExport != undefined) then
	(
		if (doNotExport == "1") then
			return false
	)
	
	doNotExport = getappdata node NEL3D_APPDATA_COLLISION
	if (doNotExport != undefined) then
	(
		if (doNotExport == "1") then
			return false
	)
	
	doNotExport = getappdata node NEL3D_APPDATA_COLLISION_EXTERIOR
	if (doNotExport != undefined) then
	(
		if (doNotExport == "1") then
			return false
	)
	
	return true
)

-- Must export this node ?
fn isAnimToBeExported node =
(
	automaticAnimation = getappdata node NEL3D_APPDATA_AUTOMATIC_ANIMATION
	if (automaticAnimation == undefined) then
		return false
	if (automaticAnimation == "0") then
		return false
	
	if (isAccelerator node) == true then
		return false
	
	if ((classof node) == nel_pacs_cylinder) then
		return false
	
	if ((classof node) == nel_pacs_box) then
		return false
	
	doNotExport = getappdata node NEL3D_APPDATA_DONOTEXPORT
	if (doNotExport != undefined) then
	(
		if (doNotExport == "1") then
			return false
	)

	doNotExport = getappdata node NEL3D_APPDATA_COLLISION
	if (doNotExport != undefined) then
	(
		if (doNotExport == "1") then
			return false
	)

	doNotExport = getappdata node NEL3D_APPDATA_COLLISION_EXTERIOR
	if (doNotExport != undefined) then
	(
		if (doNotExport == "1") then
			return false
	)

	return true
)

-- Cast shadow ?
fn isCastShadow node = 
(
	if (classof node == nel_ps) then
		return false

	if (isAccelerator node) == true then
	(
		return false
	)
	else
	(
		return true
	)
)

-- List the lod
lod_array = #()

-- is a lod ?
fn isLod node = 
(
	for i = 1 to lod_array.count do
	(
		if (lod_array[i] == node) then
			return true
	)
	return false
)

-- have a coarse mesh ?
fn haveCoarseMesh node = 
(
	-- Get lod count
	nodeCount = getappdata node NEL3D_APPDATA_LOD_NAME_COUNT
	if (nodeCount != undefined) then
	(
		-- For each lod
		nodeCountNum = nodeCount as Integer
		for lod = 1 to nodeCountNum do
		(
			-- Get the lod
			lod = getappdata node (NEL3D_APPDATA_LOD_NAME+lod-1)
			
			-- Exist ?
			if (lod != undefined) then
			(
				-- Select a node
				nd = execute ("$'"+lod+"'")
				
				-- Node exist ?
				if (nd != undefined) then
				(
					-- Is a coarse mesh ?
					if (getappdata nd NEL3D_APPDATA_LOD_COARSE_MESH == "1") then
						return true
				)
			)
		)
	)
	return false
)

fn runNelMaxExportSub = 
(	
	sceneName = maxFilePath + maxFileName
	
	-- Exported object count
	exported = 0
	
	-- Add the lod
	for node in geometry do
	(
		-- Get lod count
		nodeCount = getappdata node NEL3D_APPDATA_LOD_NAME_COUNT
		if (nodeCount != undefined) then
		(
			-- For each lod
			nodeCountNum = nodeCount as Integer
			for lod = 1 to nodeCountNum do
			(
				-- Get the lod
				lod = getappdata node (NEL3D_APPDATA_LOD_NAME+lod-1)
				
				-- Exist ?
				if (lod != undefined) then
				(
					-- Select a node
					try
					(
						nd = execute("$'"+lod+"'")
					)
					catch
					(
						NelToolLoggerWriteError "ERROR" "*" ("Error in Execute $'"+lod+"' from node "+node.name)
						nd = undefined
					)
					
					-- Node exist ?
					if (nd != undefined) then 
					(
						append lod_array nd
					)
				)
			)
		)
	)
	
	-- Select objects for shadows
	for node in geometry do
	(
		if (node.parent == undefined) then
		(
			-- Cast shadow ?
			if (isCastShadow node == true) then
			(
				-- Select this node
				selectmore node
			)
		)
	)
	
	-- Tag this file ?
	tagThisFile = true
	
	-- Array of node to export
	array_node = #()
	
	-- Add geometry
	for node in geometry do
		append array_node node
	
	-- Add shapes
	for node in shapes do
		append array_node node
	
	-- For each node
	for node in array_node do
	(
		-- It is root ?
		if (node.parent == undefined) then
		(
			-- Is not a skeleton ?
			if (node.name != "Bip01") then
			(
				-- Can be exported ?
				if (isToBeExported node == true) then
				(
					-- Not a lod ?
					if ((isLod node) == false) then
					(
						-- Output directory
						if (haveCoarseMesh node) == true then
							output = ("%OutputDirectoryWithCoarseMesh%/" + (node.name) + ".shape")
						else
							output = ("%OutputDirectoryWithoutCoarseMesh%/" + (node.name) + ".shape")
						
						try
						(
							-- Export the shape
							if (NelExportShapeEx node output %ShapeExportOptShadow% %ShapeExportOptExportLighting% "%OutputDirectoryLightmap%" %ShapeExportOptLightingLimit% %ShapeExportOptLumelSize% %ShapeExportOptOversampling% true false %ShapeExportOptLightmapLog%) == true then
							(
								exported = exported + 1 
							)
							else
							(
								NelToolLoggerWriteError "ERROR" "*" "Failed exporting shape " + node.name + ".shape"
							)
						)
						catch 
						(
							NelToolLoggerWriteError "ERROR" "*" "Fatal error exporting shape " + node.name + ".shape"
						)
					)
				)
			)
		)
	)
	
	-- Export default animations	
	for node in objects do
	(
		-- Can export it ?
		if (isAnimToBeExported node) == true then
		(
			-- Anim output directory
			output = ("%OutputDirectoryAnim%/" + (node.name) + ".anim")
			
			-- Export the animation
			if (NelExportAnimation #(node) output false) == false then
			(
				NelToolLoggerWriteError "ERROR" "*" "Exporting animation " + node.name + ".anim"
			)
		)
	)
	
	-- Nothing exported
	if exported == 0 then
	(
		-- Warning
		NelToolLoggerWriteError "WARNING" "*" "No shape exported"
	)
)
