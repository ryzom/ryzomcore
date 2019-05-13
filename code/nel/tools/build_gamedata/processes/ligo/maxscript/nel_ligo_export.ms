-- Export zones and transzones

ligo					= 1304892483
ligo_passable_app		= ligo
ligo_rotate_app			= 1266703978
ligo_scale_app			= 1266703979
ligo_instance_app		= 1266703980

TransitionScale			= #( false,		false,		false,			false,			true,		false,		false,			false,			false)
TransitionRot			= #( 2,			1,			3,				0,				1,			3,			0,				0,				0)
TransitionPos			= #( [0, 0, 0], [-1, 0, 0], [-1, -1, 0],	[-1, -2, 0],	[0, -2, 0], [0, -3, 0], [-1, -3, 0],	[-2, -3, 0],	[-3, -3, 0])
TransitionIds			= #( #( 1, 2 ), #( undefinded, 3 ), #( 5, 4 ), #( 6, 7, 8, 9 ) )
TransitionType			= #( "CornerA", "CornerA",	"Flat",			"CornerA",		"CornerB",	"CornerB",	"Flat",			"Flat",			"CornerB")
TransitionNumBis		= #( 5,			4,			2,				3,				7,			6,			0,				1,				8)

NEL3D_APPDATA_IGNAME = 1423062564			-- string : name of the Instance Group

tagThisFile = true
removeRunningTag = true

-- Unhide layers
fn unhidelayers = 
(
	for i = 0 to (LayerManager.count - 1) do
	(
		layer = (LayerManager.getLayer i)
		layer.ishidden = false
	)
)

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

-- Lower case
fn lowercase instring =
(  
	local upper, lower, outstring
	upper="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	lower="abcdefghijklmnopqrstuvwxyz" 

	outstring = copy instring 

	for iii = 1 to outstring.count do 
	(  
		jjj = findString upper outstring[iii]
		if (jjj != undefined) then
			outstring[iii] = lower[jjj] 
		else
			outstring[iii] = instring[iii]
	)
	return outstring     -- value of outstring will be returned as function result 
)

-- Allocate 20 Me for the script
heapSize += 30000000

-- In case of error just abort the app and don't show nel report window
NelForceQuitOnMsgDisplayer()

nlErrorFilename = "%OutputLogfile%"
nlErrorStream = openFile nlErrorFilename mode:"a"
if nlErrorStream == undefined then
	nlErrorStream = createFile nlErrorFilename

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

-- Add properties
fn addProperties node propsArray =
(
	-- The passable properties
	if (getappdata node ligo_passable_app) != undefined then
		append propsArray #( "passable", "yes")
	else
		append propsArray #( "passable", "no")
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
		if (nodeArray != undefined) then
			messageBox ("The zone "+nodeArray[zone].name+" is not well positionned. Check the transition scheme.") title:title_msgbox beep:true
		return false
	)
	else
	(
		-- Valid X coordinate ?
		if (x[1] < 0) or (x[1] >= TransitionIds[y[1]+1].count) then
		(
			if (nodeArray != undefined) then
				messageBox ("The zone "+nodeArray[zone].name+" is not well positionned. Check the transition scheme.") title:title_msgbox beep:true
			return false
		)
	)

	return true
)

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


-- Get the ig name of this object
fn getIg node =
(
	igName = (getappdata node NEL3D_APPDATA_IGNAME)
	if igName == "" then
	(
		return undefined
	)
	if (igName != undefined) then
		return (lowercase (igName))
	else
		return undefined
)


NEL3D_APPDATA_COLLISION					= 1423062613
NEL3D_APPDATA_COLLISION_EXTERIOR		= 1423062614

-- Must export this node ?
fn isToBeExportedCollision node = 
(
	doNotExport = getappdata node NEL3D_APPDATA_COLLISION
	if (doNotExport != undefined) then
	(
		if (doNotExport == "1") then
			return true
	)

	doNotExport = getappdata node NEL3D_APPDATA_COLLISION_EXTERIOR
	if (doNotExport != undefined) then
	(
		if (doNotExport == "1") then
			return true
	)

	return false
)

fn selectCollisionsForExport =
(
	-- Select all collision mesh
	max select none
	clearSelection()
	anySelected = false;
	for m in geometry do
	(
		if (isToBeExportedCollision m) == true then
		(
			selectmore m
			anySelected = true
		)
	)
	for node in objects where classOf node == XRefObject do
	(
		sourceObject = node.GetSourceObject false
		if (superclassOf sourceObject == GeometryClass) then
		(
			if (isToBeExportedCollision node) == true then
			(
				selectmore node
				anySelected = true
			)
		)
	)
	return anySelected
)

-- Export collisions from the current loaded zone
fn exportCollisionsFromZone outputNelDir filename =
(
	-- Export the collision
	if (NelExportCollision ($selection as array) outputNelDir) == false then
	(
		nlerror("ERROR exporting collision " + filename + " in folder " + outputNelDir)
	)
	else
	(
		nlerror("OK collision in folder " + outputNelDir)
	)
)

fn buildTransitionMatrix mt transitionZone cellSize =
(
	copyMt = mt
	backupPos = copyMt.pos
	copyMt.pos = [0,0,0]

	-- Scale ?
	if TransitionScale[transitionZone] == true then
	(
		copyMt = scale copyMt [-1,1,1]
	)

	-- Rotate
	if TransitionRot[transitionZone] != 0 then
	(
		copyMt = rotateZ copyMt (90*TransitionRot[transitionZone])
	)

	-- Translation
	return translate copyMt ( (TransitionPos[transitionZone] * cellSize) + backupPos)
)

fn buildTransitionMatrixObj mt transitionZone cellSize =
(
	-- Place at the origine
	copyMt = transMatrix (TransitionPos[transitionZone] * cellSize)

	-- Center the zone
	copyMt = translate copyMt [-cellSize/2, -cellSize/2, 0]

	-- Scale ?
	if TransitionScale[transitionZone] == true then
	(
		copyMt = scale copyMt [-1,1,1] true
	)

	-- Rotate
	if TransitionRot[transitionZone] != 0 then
	(
		copyMt = rotateZ copyMt (90*TransitionRot[transitionZone])
	)

	-- Translation
	copyMt = translate copyMt [cellSize/2,cellSize/2,0]

	-- Compose
	return (mt * copyMt)
)

-- Export instance groups from the current loaded zone
fn exportInstanceGroupFromZone inputFile outputPath igName transitionZone cellSize = 
(
	-- Unhide category
	unhidelayers()
	unhidecategory()

	-- Unhide
	max unhide all

	-- unselect
	max select none
	clearSelection()

	-- Exported object count
	exported = 0

	-- Ig array
	ig_array = #()

	-- Scan all the ig in this project
	nlerror("Scan all the ig in this project")
	for node in objects do
	(
		ig = getIg node
		nlerror("object node")
		if ( (ig != undefined) and (ig != "") and ( (igName == "") or (ig == igName) ) ) then
		(
			nlerror("Found something with an IG name")
			
			-- Transition ?
			if ( ig == IgName) then
			(
				-- Transform the object
				node.transform = buildTransitionMatrixObj node.transform transitionZone cellSize
			)

			-- Found ?
			found = false

			-- Already found ?
			for j = 1 to ig_array.count do
			(
				if (ig_array[j]==ig) then
				(								
					found = true
				)
			)

			-- Found ?
			if (found == false) then
			(
				append ig_array ig
				nlerror("New IG")
			)
		)
	)

	-- Have some ig ?
	if (ig_array.count != 0) then
	(
		-- For each ig
		for ig = 1 to ig_array.count do
		(
			if ( (igName == "") or (ig_array[ig] == IgName) ) then
			(
				-- Output filename
				output = (outputPath + ig_array[ig] + ".ig")

				-- Check date
				-- if (NeLTestFileDate output inputFile) == true then
				-- (
					-- Select none
					max select none
					clearSelection()

					for node in objects where classOf node == XRefObject do
					(
						if ((getIg node) == ig_array[ig]) then
						(
							sourceObject = node.GetSourceObject false
							if (classOf sourceObject == XRefObject) then
							(
								nlerror("FAIL XREF STILL XREF " + node.name)
							)
							else if (superclassOf sourceObject == GeometryClass) then
							(
								selectmore node
							)
							else if (superclassOf sourceObject == Helper) then
							(
								selectmore node
							)
							else if (superclassOf sourceObject == Light) then
							(
								selectmore node
							)
						)
					)
					-- Select all node in this ig
					for node in geometry do
					(
						-- Select it if in the ig
						if ( (getIg node) == ig_array[ig]) then
							selectmore node
					)
					-- Select all lights in this ig
					for node in lights do
					(
						-- Select it if in the ig
						if ( (getIg node) == ig_array[ig]) then
							selectmore node
					)
					-- Select all helpers in this ig
					for node in helpers do
					(
						-- Select it if in the ig
						if ( (getIg node) == ig_array[ig]) then
							selectmore node
					)
					-- Check export
					try
					(
						-- Export the ig
						instancegroup2export = $selection as array
						if (NelExportInstanceGroup instancegroup2export output) == true then
						(
							nlerror ("OK "+output)
						)
						else
						(
							-- Error
							nlerror ("ERROR exporting ig "+ig_array[ig]+" in file "+inputFile)
							tagThisFile = false
						)
					)
					catch
					(
						-- Error
						nlerror ("ERROR fatal error exporting ig "+ig_array[ig]+" in file "+inputFile)
						tagThisFile = false
					)
				-- )
				-- else
				-- (
				-- 	nlerror ("SKIPPED ligo ig "+output)
				-- 	tagThisFile = false
				-- )
			)
		)
	)
)

------------------------------------------------------------------------------------------------------------------
-- ENTRY POINT
-- ***********

ligopath = getINISetting ((GetDir #plugcfg)+"\\nelligo.ini") "LigoConfig" "LigoPath" 
if ligopath != "" then ligo_root_path = ligopath
ligopath = getINISetting ((GetDir #plugcfg)+"\\nelligo.ini") "LigoConfig" "LigoExportPath" 
if ligopath != "" then ligo_export_path = ligopath

MaxFilesList = getFiles (ligo_root_path + "*.max")


-- EXPORT ZONES

try
(
	-- Set the bank pathname
	bank_filename = "%SmallBankFilename%"
	NelSetTileBank bank_filename

	cellSize = NeLLigoGetCellSize ()

	for curFileName in MaxFilesList do
	(	
		-- Free memory and file handles
		-- gc ()
		-- resetMAXFile #noprompt
		

		tokenArray = filterString (getFilenameFile curFileName) "-"
		if (tokenArray.count == 3) and (tokenArray[1] == "zonematerial")  then
		(			
			-- Get the tag file name
			tag = ("%TagDirectory%/"+(getFilenameFile curFileName)+(getFilenameType curFileName)+".tag")

			-- Compare date with the tag file
			if (NeLTestFileDate tag curFileName) == true then
			(
				-- Try to tag this file
				tagThisFile = true
				
				resetMAXFile #noprompt
				
				nlerror ("Scanning file "+curFileName+" ...")
				loadMaxFile curFileName quiet:true
				try
				(
					nlerror("Update XRef records...")
					objXRefMgr.UpdateAllRecords()
				)
				catch
				(
					nlerror("ERROR Failed to update XRef! (DON'T TAG)...")
					tagThisFile = false
				)

				-- Unhide category
				unhidelayers()
				unhidecategory()

				DeleteDebugMarkersFn ()
				selectAllPatch ()
				-- nlerror ("EXPORT "+ (getFilenameFile curFileName))

				props = #( #("zone", (tokenArray[2] + "-" + tokenArray[3])), #("material", tokenArray[2]) ) 
				
				nlerror("NeLLigoExportZone " + curFileName)
				if (selection as array).count > 1 then
				(
					nlerror ("ERROR In file "+(getFilenameFile curFileName)+", multiple NelPatchMesh. Can't export.");
					tagThisFile = false
				)
				else 
				(
					if (selection as array).count == 0 then
					(
						nlerror ("WARNING In file "+(getFilenameFile curFileName)+", no NelPatchMesh to export.");
					)
					else
					(
						addProperties $ props

						dstFileName = ligo_export_path + tokenArray[2] + "-" + tokenArray[3] + ".ligozone"

						-- Error catching
						try
						(							
							-- export ligo zone
							if (NeLLigoExportZone $ dstFileName props false false) then
							(
								nlerror ("OK "+ (getFilenameFile curFileName))
							)
							else
							(
								nlerror ("ERROR Error exporting ligo zone from file "+ (getFilenameFile curFileName))
								tagThisFile = false
							)							
							
						)
						catch
						(
							-- Error
							nlerror ("ERROR fatal error exporting ligo zone "+curFileName)
							tagThisFile = false
						)

					)
				)

				-- export matching igs
				try
				(
					nlerror("exportInstanceGroupFromZone " + curFileName)
					exportInstanceGroupFromZone curFileName (ligo_export_path + "igs\\") "" 0 cellSize
				)
				catch
				(
					nlerror("couldn't export ig for " + curFileName)
					tagThisFile = false
				)

				-- export collisions
				if selectCollisionsForExport() then
				(
					try
					(
						nlerror("exportCollisionsFromZone " + curFileName)
						exportCollisionsFromZone (ligo_export_path + "cmb\\") curFileName
						nlerror("past exportCollisionsFromZone")
					)
					catch
					(
						nlerror("couldn't export collision for " + curFileName)
						tagThisFile = false
					)
				)

				-- Write a tag file
				nlerror("check to write tag")
				if tagThisFile == true then
				(
					nlerror("TAG " + curFileName)
					nlerror("TAGFILE " + tag)
					tagFile = createFile tag
					if tagFile == undefined then
					(
						nlerror ("WARNING can't create tag file "+tag)
						removeRunningTag = false
					)
					else
					(
						print "toto" to: tagFile
						close tagFile
					)
				)
				else
				(
					nlerror("NOT TAGGING " + curFileName)
					removeRunningTag = false
				)
				
				resetMAXFile #noprompt
				gc ()
				resetMAXFile #noprompt
			)
			else
			(
				nlerror ("SKIPPED BY TAG ligozone "+ (getFilenameFile curFileName))
			)
		)
	)

	-- EXPORT TRANSZONES

	for curFileName in MaxFilesList do
	(
		-- Free memory and file handles
		-- gc ()

		-- Reset 3dsmax
		-- resetMAXFile #noprompt

		tokenArray = filterString (getFilenameFile curFileName) "-"
		if (tokenArray.count == 4) and (tokenArray[1] == "zonetransition") then
		(
			HaveToExport = #()
			for zone = 1 to 9 do
			(
				filename = tokenArray[2] + "-" + tokenArray[3] + "-" + tokenArray[4] + "-" + ((zone-1) as string)
				filename = ligo_export_path + "zoneLigos\\" + filename + ".ligozone"

				if (nelTestFileDate filename curFileName) == true then
				(
					append HaveToExport true
				)
				else
				(
					append HaveToExport false
				)
			)

			-- Get the tag file name
			tag = ("%TagDirectory%/"+(getFilenameFile curFileName)+(getFilenameType curFileName)+".tag")

			-- Compare date with the tag file
			if (NeLTestFileDate tag curFileName) == true then
			(
				-- Try to tag this file
				tagThisFile = true
				
				resetMAXFile #noprompt
				
				nlerror ("Scanning file "+curFileName+" ...")
				loadMaxFile curFileName quiet:true
				try
				(
					nlerror("Update XRef records...")
					objXRefMgr.UpdateAllRecords()
				)
				catch
				(
					nlerror("ERROR Failed to update XRef! (DON'T TAG)...")
					tagThisFile = false
				)
				
				-- Unhide category
				unhidelayers()
				unhidecategory()
				
				DeleteDebugMarkersFn ()
				selectAllPatch ()
				-- nlerror ("EXPORT "+ (getFilenameFile curFileName))

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
							if (id == undefined) then
							(
								nlerror ("ERROR A zone is not at the right position in "+ (getFilenameFile curFileName))
								tagThisFile = false
								ok = false
							)
							else
							(
								transitionZone[id] = nodeArray[zone]
							)
						)
					)

					-- Continue ?
					if (ok == true) and (findOne == true) then
					(
						-- For each zone selected
						for zone = 1 to 9 do
						(
							-- Zone name
							zoneBaseName = tokenArray[2] + "-" + tokenArray[3] + "-" + tokenArray[4] + "-" + ((zone-1) as string)

							-- Get the destination filename for export
							filename = ligo_export_path + zoneBaseName + ".ligozone"

							if (HaveToExport[zone] == true) then
							(
								-- Defined ?
								if (transitionZone[zone] != undefined) then
								(
									-- Make an array of properties
									props = #( #("zone", zoneBaseName), #("transname", (tokenArray[2]+"-"+tokenArray[3]) ), #("transtype", TransitionType[zone] ), #("transtype", TransitionType[zone] ), #("transnum", (TransitionNumBis[zone] as string) ) ) 

									-- Add the properties
									addProperties transitionZone[zone] props

									-- Transform the zone
									c = instance transitionZone[zone]

									-- Change the name
									c.name = "[NELLIGO] tmp export"

									-- The matrix
									mt = buildTransitionMatrix transitionZone[zone].transform zone cellSize

									-- Scale ?
									if TransitionScale[zone] == true then
									(
										setappdata c ligo_scale_app "1"
									)

									-- Rotate
									if TransitionRot[zone] != 0 then
									(
										setappdata c ligo_rotate_app (TransitionRot[zone] as string)
									)

									-- Transform the node
									c.transform = mt

									-- Export the zone

									-- Error catching
									try
									(
										-- nlerror ("EXPORT "+ (getFilenameFile filename))
										if (NeLLigoExportZone c filename props false false) then
										(
											nlerror ("OK "+ (getFilenameFile filename))
										)
										else
										(
											nlerror ("ERROR Error exporting ligo zone from file "+ (getFilenameFile filename))
											tagThisFile = false
										)
									)
									catch
									(
										-- Error
										nlerror ("ERROR fatal error exporting ligo zone "+curFileName)
										tagThisFile = false
									)

									-- Delete
									delete c
								)
							)
							else
							(
								nlerror ("SKIPPED ligozone transition "+ (getFilenameFile filename))
							)

							-- export igs
							try
							(
								nlerror("exportInstanceGroupFromZone " + curFileName)
								exportInstanceGroupFromZone curFileName (ligo_export_path + "igs\\") (lowercase (zoneBaseName)) zone cellSize
							)
							catch
							(
								nlerror("couldn't export ig for " + curFileName)
								tagThisFile = false
							)

							-- export collisions
							if selectCollisionsForExport() then
							(
								try
								(
									nlerror("exportCollisionsFromZone " + curFileName)
									exportCollisionsFromZone (ligo_export_path + "cmb\\") curFileName
								)
								catch
								(
									nlerror("couldn't export collision for " + curFileName)
									tagThisFile = false
								)
							)
						)
					)

					-- Write a tag file
					if tagThisFile == true then
					(
						nlerror("TAG " + curFileName)
						nlerror("TAGFILE " + tag)
						tagFile = createFile tag
						if tagFile == undefined then
						(
							nlerror ("WARNING can't create tag file "+tag)
							removeRunningTag = false
						)
						else
						(
							print "toto" to: tagFile
							close tagFile
						)
					)
					else
					(
						nlerror("NOT TAGGING " + curFileName)
						removeRunningTag = false
					)
				)
				
				resetMAXFile #noprompt
				gc ()
				resetMAXFile #noprompt
			)
			else
			(
				nlerror ("SKIPPED BY TAG transition "+ (getFilenameFile curFileName))
			)
		)
	)

	-- EXPORT SPECIAL ZONES

	for curFileName in MaxFilesList do
	(
		-- Free memory and file handles
		-- gc ()
		-- resetMAXFile #noprompt

		tokenArray = filterString (getFilenameFile curFileName) "-"
		if (tokenArray.count == 2) and (tokenArray[1] == "zonespecial")  then
		(
			-- Get the tag file name
			tag = ("%TagDirectory%/"+(getFilenameFile curFileName)+(getFilenameType curFileName)+".tag")

			-- Compare date with the tag file
			if (NeLTestFileDate tag curFileName) == true then
			(
				-- Try to tag this file
				tagThisFile = true
				
				resetMAXFile #noprompt
				
				nlerror ("Scanning file "+curFileName+" ...")
				loadMaxFile curFileName quiet:true
				try
				(
					nlerror("Update XRef records...")
					objXRefMgr.UpdateAllRecords()
				)
				catch
				(
					nlerror("ERROR Failed to update XRef! (DON'T TAG)...")
					tagThisFile = false
				)

				-- Unhide category
				unhidelayers()
				unhidecategory()

				DeleteDebugMarkersFn ()
				selectAllPatch ()
				-- nlerror ("EXPORT "+ (getFilenameFile curFileName))

				props = #( #("zone", tokenArray[2]), #("material", "special") )

				if (selection as array).count > 1 then
				(
					nlerror ("ERROR In file "+(getFilenameFile curFileName)+", multiple NelPatchMesh. Can't export.");
					tagThisFile = false
				)
				else 
				(
					if (selection as array).count == 0 then
					(
						nlerror ("WARNING In file "+(getFilenameFile curFileName)+", no NelPatchMesh to export.");
					)
					else
					(
						addProperties $ props

						dstFileName = ligo_export_path + tokenArray[2] + ".ligozone"

						-- Error catching
						try
						(
							if (NeLLigoExportZone $ dstFileName props false false) then
							(
								nlerror ("OK "+ (getFilenameFile curFileName))
							)
							else
							(
								nlerror ("ERROR Error exporting ligo zone from file "+ (getFilenameFile curFileName))
								tagThisFile = false
							)
						)
						catch
						(
							-- Error
							nlerror ("ERROR fatal error exporting ligo zone "+curFileName)
							tagThisFile = false
						)
					)
				)

				-- export matching igs
				try
				(
					nlerror("exportInstanceGroupFromZone " + curFileName)
					exportInstanceGroupFromZone curFileName (ligo_export_path + "igs\\") "" 0 cellSize
				)
				catch
				(
					nlerror("couldn't export ig for " + curFileName)
					tagThisFile = false
				)

				-- export collisions
				if selectCollisionsForExport() then
				(
					try
					(
						nlerror("exportCollisionsFromZone " + curFileName)
						exportCollisionsFromZone (ligo_export_path + "cmb\\") curFileName
					)
					catch
					(
						nlerror("couldn't export collision for " + curFileName)
						tagThisFile = false
					)
				)

				-- Write a tag file
				if tagThisFile == true then
				(
					nlerror("TAG " + curFileName)
					nlerror("TAGFILE " + tag)
					tagFile = createFile tag
					if tagFile == undefined then
					(
						nlerror ("WARNING can't create tag file "+tag)
						removeRunningTag = false
					)
					else
					(
						print "toto" to: tagFile
						close tagFile
					)
				)
				else
				(
					nlerror("NOT TAGGING " + curFileName)
					removeRunningTag = false
				)
				
				resetMAXFile #noprompt
				gc ()
				resetMAXFile #noprompt
			)
			else
			(
				nlerror ("SKIPPED BY TAG ligozone special "+ (getFilenameFile curFileName))
			)
		)
	)
)
catch 
(
	-- Error
	nlerror("ERROR fatal error exporting ligo zone in folder"+ligo_root_path)
	nlerror("FAIL Fatal error occurred")
	NelForceQuitRightNow()
	removeRunningTag = false
	tagThisFile = false
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
	nlerror("FAIL One or more issues occurred")
	NelForceQuitRightNow()
)

nlerror("BYE")
quitMAX #noPrompt
quitMAX () #noPrompt

