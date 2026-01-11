-- Constants

NEL3D_APPDATA_IGNAME				=	1423062564
NEL3D_APPDATA_LOD_NAME_COUNT		=	1423062537
NEL3D_APPDATA_LOD_NAME				=   1423062538
NEL3D_APPDATA_LOD_NAME_COUNT_MAX	=	10
NEL3D_APPDATA_DONTEXPORT			=	1423062565
NEL_OBJET_NAME_DATA					=	1970
NEL3D_APPDATA_COLLISION				=	1423062613
NEL3D_APPDATA_COLLISION_EXTERIOR	=	1423062614

-- Globals

BuildingFilenamePath = ""
VillageFilenamePath = ""

-- Function

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

fn isInArray array elm =
(
	local ii
	for ii in array do
	(
		if (ii as string) == (elm as string) then
			return true
	)
	return false
)

-- Roll out

rollout xref_rollout "Properties"
(
group "Parameters"
	(
		Button 			InputFilename			"Building File Name"				width:650	align:#left
		Button 			DestinationFilename		"Village File Name"				width:650	align:#left
		Label			IgNameLabel				"Ig Name"									align:#left
		EditText		IgName					""								width:140	align:#left
		Label			ZoneNameLabel			"Zone Name"									align:#left
		EditText		ZoneName				""								width:140	align:#left
	)
group "Exterior Meshes"
	(
		listbox			ExteriorMesh											width:650
		Button 			RemoveFromExteriorMeshList	"Remove From Exterior Mesh List" align:#left
	)
group "Landscape Lights"
	(
		listbox			LandscapeLight											width:650
		Button 			RemoveFromLandscapeLightList	"Remove From Landscape Light List" align:#left
	)
group "Other Nodes"
	(
		listbox			ImportedNodes											width:650
		Button 			PutToExteriorMeshList	"Put To Exterior Mesh List"					align:#left across:2
		Button 			PutToLandscapeLightList	"Put To Landscape Light List"				align:#left
	)
	Label				ProgressText											width:650	align:#left
	ProgressBar			Progress												width:650	align:#left
	Button 				XRef					"XRef!"						width:650	align:#left

	on InputFilename pressed do
	(
		-- Choose the max project
		BuildingFilenamePath = getOpenFileName caption:"Select the building project to xref" filename:"*.max" types:"3D Studio MAX (*.max)|*.max|All Files (*.*)|*.*||"

		-- Ok ?
		if (BuildingFilenamePath != undefined) then
		(
			InputFilename.text = BuildingFilenamePath

			-- Get the node names
			nodesName = getMAXFileObjectNames BuildingFilenamePath
			if (nodesName != undefined) then
			(
				-- Build the 3 lists
				_LandscapeLight = #()
				_ExteriorMesh = #()
				_ImportedNodes = #()

				-- For each node in the max file
				for i in nodesName do
				(
					addItExterior = false
					addItLandscape = false
					names = filterString (lowercase (i as string)) "_"
					for j in names do
					(
						if j == "ext" then
						(
							addItExterior = true
							exit
						)
						else if j == "land" then
						(
							addItLandscape = true
							exit
						)
					)

					if addItLandscape == true then
						append _LandscapeLight i
					else if addItExterior == true then
						append _ExteriorMesh i
					else
						append _ImportedNodes i
				)
				sort _LandscapeLight
				sort _ExteriorMesh
				sort _ImportedNodes
				LandscapeLight.items = _LandscapeLight
				ExteriorMesh.items = _ExteriorMesh
				ImportedNodes.items = _ImportedNodes
			)
		)
	)

	on DestinationFilename pressed do
	(
		-- Choose the max project
		VillageFilenamePath = getOpenFileName caption:"Select the village project to xref into" filename:"*.max" types:"3D Studio MAX (*.max)|*.max|All Files (*.*)|*.*||"

		-- Ok ?
		if (VillageFilenamePath != undefined) then
		(
			DestinationFilename.text = VillageFilenamePath

			-- Get the zone name
			array_name = filterString (getFilenameFile VillageFilenamePath) "-"
			ZoneName.text = ""
			for i = 2 to array_name.count do
			(
				if ZoneName.text != "" then
					ZoneName.text = ZoneName.text + "-"
				ZoneName.text = ZoneName.text + lowercase (array_name[i])
			)
		)
	)

	on PutToExteriorMeshList pressed do
	(
		if (ImportedNodes.selection != 0) then
		(
			_exteriorMeshes = ExteriorMesh.items
			_importedNodes = ImportedNodes.items
			append _exteriorMeshes _importedNodes[ImportedNodes.selection]
			deleteItem _importedNodes ImportedNodes.selection
			sort _exteriorMeshes
			ExteriorMesh.items = _exteriorMeshes
			ImportedNodes.items = _importedNodes
		)
	)

	on PutToLandscapeLightList pressed do
	(
		if (ImportedNodes.selection != 0) then
		(
			_landscapeLight = LandscapeLight.items
			_importedNodes = ImportedNodes.items
			append _landscapeLight _importedNodes[ImportedNodes.selection]
			deleteItem _importedNodes ImportedNodes.selection
			sort _landscapeLight
			LandscapeLight.items = _landscapeLight
			ImportedNodes.items = _importedNodes
		)
	)

	on RemoveFromExteriorMeshList pressed do
	(
		if (ExteriorMesh.selection != 0) then
		(
			_exteriorMeshes = ExteriorMesh.items
			_importedNodes = ImportedNodes.items
			append _importedNodes _exteriorMeshes[ExteriorMesh.selection]
			deleteItem _exteriorMeshes ExteriorMesh.selection
			sort _importedNodes
			ExteriorMesh.items = _exteriorMeshes
			ImportedNodes.items = _importedNodes
		)
	)

	on RemoveFromLandscapeLightList pressed do
	(
		if (LandscapeLight.selection != 0) then
		(
			_landscapeLight = LandscapeLight.items
			_importedNodes = ImportedNodes.items
			append _importedNodes _landscapeLight[LandscapeLight.selection]
			deleteItem _landscapeLight LandscapeLight.selection
			sort _importedNodes
			LandscapeLight.items = _landscapeLight
			ImportedNodes.items = _importedNodes
		)
	)

	on XRef pressed do 
	(
		-- Selected ?
		if (IgName.text != "") and (BuildingFilenamePath != "") and (VillageFilenamePath != "") then
		(
			-- Reset max file
			resetMAXFile #noprompt

			-- Load the building

			-- Look for an unique postfix
			buildingName = IgName.text
			postfix = "_" + buildingName

			-- XRef the projects
			nodesName = getMAXFileObjectNames BuildingFilenamePath
			if nodesName != undefined then
			(
				-- The lod name array
				lodNameArray = #()

				-- For each node visible
				for iCount = 1 to nodesName.count do
				(
					-- Garbage collector
					gc ()

					-- Progress bar
					ProgressText.text  = "XRef building " + (getFilenameFile BuildingFilenamePath)
					Progress.value = iCount*100/nodesName.count

					-- Node ref
					nodeName = nodesName[iCount]

					-- Make an xref
					node = xrefs.addNewXRefObject BuildingFilenamePath (nodeName as string)
					if node != undefined then
					(
						-- Rename its lod
						countString = getAppData node NEL3D_APPDATA_LOD_NAME_COUNT
						if (countString != undefined) then
						(
							count = countString as integer
							for lod = 1 to count do
							(
								lodName = getAppData node (NEL3D_APPDATA_LOD_NAME+lod-1)
								if lodName != undefined then
								(
									setAppData node (NEL3D_APPDATA_LOD_NAME+lod-1) (lodName + postfix)
									append lodNameArray (lodName + postfix)
								)
							)
						)

						-- Remove some node properties
						deleteAppData node NEL3D_APPDATA_IGNAME
						deleteAppData node NEL3D_APPDATA_DONTEXPORT
						deleteAppData node NEL_OBJET_NAME_DATA

						-- Is it a light ?
						if (superclassof node == light) then
						(
							-- Is it in the landscape ?
							arrayToTest = LandscapeLight.items
							if (isInArray arrayToTest node.name) == true then
							(
								if (ZoneName.text != "") then
									setAppData node NEL3D_APPDATA_IGNAME ZoneName.text
							)
							else
							(
								if (IgName.text != "") then
									setAppData node NEL3D_APPDATA_IGNAME IgName.text
							)
						)
						else
						(
							-- Add ig name
							if (IgName.text != "") then
								setAppData node NEL3D_APPDATA_IGNAME IgName.text

							-- Is a collision shape ?
							col = getAppData node NEL3D_APPDATA_COLLISION
							if col == undefined then
								col = getAppData node NEL3D_APPDATA_COLLISION_EXTERIOR

							-- not a collision
							if (col == undefined) or (col == "0") then
							(
								-- Is it exterior ?
								arrayToTest = ExteriorMesh.items
								if (isInArray arrayToTest node.name) == false then
								(
									-- Don't export this shape
									setAppData node NEL3D_APPDATA_DONTEXPORT "1"

									-- This shape is a reference
									setAppData node NEL_OBJET_NAME_DATA node.name
								)
							)
						)

						-- Rename the node
						node.name = node.name + postfix
					)
				)

				-- Erase lod properties
				for lod in lodNameArray do
				(
					-- Select the node by name
					node = getnodebyname lod
					if (node != undefined) then
					(
						deleteAppData node NEL3D_APPDATA_IGNAME
						deleteAppData node NEL3D_APPDATA_DONTEXPORT
						deleteAppData node NEL_OBJET_NAME_DATA
					)
				)

				-- Save a temp file
				tempFile = getdir #scene + "\\nel_xref_building_temp.max"
				ProgressText.text  = "Save temp file"
				Progress.value = 50

				-- Unhide all
				max unfreeze all
				max unhide all

				if saveMaxFile tempFile == true then
				(
					-- Load the destination project
					ProgressText.text  = "Load village " + (getFilenameFile VillageFilenamePath)
					Progress.value = 50

					-- Load village file
					if (loadMaxFile VillageFilenamePath) == true then
					(
						-- Merge the temp file
						ProgressText.text  = "Merge building"
						Progress.value = 50

						-- Hide all
						max select all
						hide selection

						-- Merge the temp file
						if (mergeMAXFile tempFile #mergeDups) == true then
						(
							ProgressText.text  = "Finished - OK"
							Progress.value = 100

							-- Make a selection group
							max select all
							selectionSets[buildingName] = selection

							-- Unhide all
							max unhide all
						)
						else
						(
							ProgressText.text  = "Error : can't merge the temp file nel_xref_building_temp.max"
							Progress.value = 100
						)
					)
					else
					(
						ProgressText.text  = ("Error : can't load the village file " + VillageFilenamePath)
						Progress.value = 100
					)
				)
				else
				(
					ProgressText.text  = "Error saving temp scene nel_xref_building_temp.max"
					Progress.value = 100
				)

				-- Delete the temp file
				deleteFile tempFile
			)
		)
	)
)

-- Go

gc ()

if xref_building_floater != undefined do
(
	closerolloutfloater xref_building_floater
)

xref_building_floater = newRolloutFloater "NeL XRef Building" 700 874
addrollout xref_rollout xref_building_floater rolledUp:false
