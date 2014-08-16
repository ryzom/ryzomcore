

-- #################################################################
-- ## WARNING : this is a generated file, don't change it !
-- #################################################################


-- Allocate 20 Me for the script
heapSize += 15000000

-- In case of error just abort the app and don't show nel report window
NelForceQuitOnMsgDisplayer()

nlErrorFilename = "%OutputLogfile%"
nlErrorStream = openFile nlErrorFilename mode:"a"
if nlErrorStream == undefined then
	nlErrorStream = createFile nlErrorFilename

-- Unhide category
fn unhidecategory =
(
	if (geometry.count > 0) then
	(
		unhide geometry[1]
		if (geometry[1].ishidden == true) then
			max hide object toggle
	)
	if (shapes.count > 0) then
	(
		unhide shapes[1]
		if (shapes[1].ishidden == true) then
			max hide shape toggle
	)
	if (lights.count > 0) then
	(
		unhide lights[1]
		if (lights[1].ishidden == true) then
			max hide light toggle
	)
	if (cameras.count > 0) then
	(
		unhide cameras[1]
		if (cameras[1].ishidden == true) then
			max hide camera toggle
	)
	if (helpers.count > 0) then
	(
		unhide helpers[1]
		if (helpers[1].ishidden == true) then
			max hide helper toggle
	)
)

-- Log a message
fn nlerror message =
(
	if nlErrorStream != undefined then
	(
		format "%\n" message to:nlErrorStream
		flush nlErrorStream
	)

	-- To the console
	print message
)



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

NEL3D_APPDATA_LOD_NAME_COUNT_MAX	= 10
NEL3D_APPDATA_LOD					= 1423062537
NEL3D_APPDATA_LOD_NAME_COUNT		= NEL3D_APPDATA_LOD
NEL3D_APPDATA_LOD_NAME				= NEL3D_APPDATA_LOD_NAME_COUNT+1
NEL3D_APPDATA_LOD_BLEND_IN			= NEL3D_APPDATA_LOD_NAME+NEL3D_APPDATA_LOD_NAME_COUNT_MAX
NEL3D_APPDATA_LOD_BLEND_OUT			= NEL3D_APPDATA_LOD_BLEND_IN+1
NEL3D_APPDATA_LOD_COARSE_MESH		= NEL3D_APPDATA_LOD_BLEND_OUT+1
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

fn runNelMaxExportSub inputMaxFile retryCount = 
(
	tagThisFile = false
	
	-- Unhide category
	unhidecategory()
	
	-- Unhide
	max unhide all
	
	-- unselect
	max select none
	clearSelection()
	
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
						nlerror("Error in Execute $'"+lod+"' from node "+node.name)
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
						
						-- Compare file date
						if (NeLTestFileDate output inputMaxFile) == true then
						(
							try
							(
								-- Export the shape
								if (NelExportShapeEx node output %ShapeExportOptShadow% %ShapeExportOptExportLighting% "%OutputDirectoryLightmap%" %ShapeExportOptLightingLimit% %ShapeExportOptLumelSize% %ShapeExportOptOversampling% true false %ShapeExportOptLightmapLog%) == true then
								(
									nlerror("OK "+output)
									exported = exported +1 
								)
								else
								(
									-- Error
									nlerror("ERROR exporting shape " + node.name + " in file " + inputMaxFile)
									tagThisFile = false
								)
							)
							catch 
							(
								-- Error
								nlerror("ERROR fatal error exporting shape " + node.name + " in file " + inputMaxFile)
								tagThisFile = false
							)
						)
						else
						(
							nlerror("SKIPPED " + output)
							exported = exported + 1
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
				nlerror ("ERROR exporting animation " + output)
				tagThisFile = false
			)
			else
			(
				nlerror("OK " + output)
			)
		)
	)
	
	-- Something exported
	if exported == 0 then
	(
		-- Error
		nlerror("WARNING no shape exported from the file " + inputMaxFile)
		if tagThisFile then
		(
			if retryCount < 2 then
			(
				nlerror("INFO retry this file")
				
				-- Free memory and file handles
				gc()
				heapfree
				
				-- Reset 3dsmax
				resetMAXFile #noprompt
				
				if (loadMaxFile inputMaxFile quiet:true) == true then
				(
					tagThisFile = runNelMaxExportSub inputMaxFile (retryCount + 1)
				)
				else
				(
					-- Error
					nlerror("ERROR exporting 'shape': can't open the file " + inputMaxFile)
					nlerror("FAIL Mysterious error occured")
					NelForceQuitRightNow()
				)
			)
		)
	)
	
	return tagThisFile
)

fn runNelMaxExport inputMaxFile = 
(
	return runNelMaxExportSub inputMaxFile 0
)



removeRunningTag = true

try
(
	undo off
	(
		-- Get files in the %MaxSourceDirectory% directory
		files = getFiles "%MaxSourceDirectory%/*.max" 
		gc()
		
		-- Sort files
		sort files
		gc()
		
		-- No file ?
		if files.count != 0 then
		(
			-- For each files
			for i = 1 to files.count do
			(
				inputMaxFile = files[i]
				outputTagFile = ("%TagDirectory%/" + (getFilenameFile inputMaxFile) + (getFilenameType inputMaxFile) + ".tag")
				
				--try
				--(
					-- Compare file date
					if (NeLTestFileDate outputTagFile inputMaxFile) == true then
					(
						-- Free memory and file handles
						gc()
						heapfree
						
						-- Reset 3dsmax
						resetMAXFile #noprompt
						
						-- Open the max project
						nlerror("Scanning file " + inputMaxFile + " ...")
						if (loadMaxFile inputMaxFile quiet:true) == true then
						(
							tagThisFile = runNelMaxExport(inputMaxFile)
							
							-- Write a tag file
							if tagThisFile == true then
							(
								tagFile = createFile outputTagFile
								if tagFile == undefined then
								(
									nlerror("WARNING can't create tag file " + outputTagFile)
									removeRunningTag = false
								)
								else
								(
									print "mukyu" to: tagFile
									close tagFile
								)
							)
							else
							(
								removeRunningTag = false
							)
						)
						else
						(
							-- Error
							nlerror("ERROR exporting 'shape': can't open the file " + inputMaxFile)
							removeRunningTag = false
						)
					)
					else
					(
						nlerror("SKIPPED BY TAG " + inputMaxFile)
					)
				--)
				--catch 
				--(
				--	-- Error
				--	nlerror("ERROR error exporting 'shape' in file " + inputMaxFile)
				--	removeRunningTag = false
				--)
			)
		)
		else
		(
			nlerror("WARNING no *.max file in folder %MaxSourceDirectory%")
		)
	)
)
catch 
(
	-- Error
	nlerror("ERROR Fatal error exporting 'shape' in folder %MaxSourceDirectory%")
	nlerror("FAIL Fatal error occured")
	NelForceQuitRightNow()
	removeRunningTag = false
)

try
(
	if (removeRunningTag) then
	(
		resetMAXFile #noPrompt
	)
)
catch 
(
	nlerror("FAIL Last reset fails")
	removeRunningTag = false
)

if (removeRunningTag) then
(
	nlerror("SUCCESS All .max files have been successfully exported")
	deleteFile("%TagDirectory%/max_running.tag")
)
else
(
	nlerror("FAIL One or more issues occured")
	NelForceQuitRightNow()
)

-- Bye
nlerror("BYE")
quitMAX #noPrompt
quitMAX() #noPrompt

