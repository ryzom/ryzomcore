-- *** Convert old zone into ligo zone

-- *** How to work with old zones:

-- Continue to edit old zone files in world position, not in new ligo material zone
-- Place villages near the origine, not in the world and save the metazone in the ligo folder
-- When modifing metazone or zone, launch this tool to create new ligo files
	-- It will create a meta materialzone for each village.
	-- It will create a materialzone for each old zone not in metazone.
	-- It will remove modified zone in ligo materialzone files and merge new zone modified in old file translated at origin and flagged properly
	-- It will update the .land file

-- Process overview
	-- For each meta zone
		-- If one old file in the metazone is more recent than dest metazone file or no old file exist
			-- Reset max
			-- Load the destination zone file
			-- For each old zone file in the meta zone
				-- Remove the patch mesh from max
				-- If the old file zone exist
					-- Merge the new zone file in the destination file
	-- For each max file in the old directory
		-- If the src file is more recent than dest zone file
			-- Reset max
			-- Load the destination zone file
			-- Delete the old zone in the destination file
			-- Merge the new zone in the destination file
	-- Generate the .land file

-- Structures
struct metazone (name, posMin, posMax)

-- Fyros : X: [150 - 169] Y: [DV - EW]     Generate : X: [99 - 128] Y: [-169 - -169]
-- Trykers : X: [194 - 215] Y: [DV - EW]     Generate : X: [96 - 115] Y: [-215 - -215+22]

-- Parameters, uppercase for cell position
old_zone_directory_fyros	=	"w:/database/landscape/zones/fyros"
ligo_directory_fyros		=	"w:/database/landscape/ligo/desert"
metazones_fyros				=	#(metazone "1_MZ_city" "152_EK" "155_EO", metazone "bridge_a" "158_EM" "158_EO", metazone "brandon" "164_ES" "165_ET", metazone "ecorce_A" "155_EI" "157_EJ")

old_zone_directory_tryker	=	"w:/database/landscape/zones/tryker"
ligo_directory_tryker		=	"r:/database/landscape/ligo/lacustre"
metazones_tryker			=	#(metazone "village_a" "205_EC" "208_EE", metazone "village_b" "199_EH" "200_EI", metazone "village_c" "206_EH" "208_EJ", metazone "village_d" "206_DS" "208_DU")

cell_size			=	160.0
weld_threshold		=	1.0

-- Constants
alphabet			=	"ABCDEFGHIJKLMNOPQRSTUVWXYZ" 
NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX	=	1342141818
NEL3D_APPDATA_DONTEXPORT			=	1423062565

-- Convert a name in coordinate
fn nameToCoord name coord =
(
	local

	-- Const
	NameTab = filterString name "_"
	alpha_letter1 = NameTab[2][1]
	alpha_letter2 = NameTab[2][2]
	alpha_letter1_value = findstring alphabet alpha_letter1 
	alpha_letter2_value = findstring alphabet alpha_letter2 

	-- Decompose theh name in an array array[1]=numeric string value  array[2]=alpha string value
	-- The index of the engine start at 0 but the script one at 1 so we sub 1 each time
	alpha_sub_id = (((alpha_letter1_value as integer - 1) * 26 + (alpha_letter2_value as integer)))-1
	num_sub_id = (NameTab[1] as integer)-1

	-- X
	coord[1] = alpha_sub_id

	-- Y
	coord[2] = num_sub_id
)

-- Convert a coordinate in a name
fn coordToName coord =
(
	up = coord[1] / 26 + 1
	down = coord[1] - ((up-1) * 26) + 1

	-- X
	coord[1] = alpha_sub_id

	-- Y
	return ((coord[2] + 1) as string) + "_" + alphabet[up] + alphabet[down]
)

-- Get the bounding of the metazone
fn getMetaZoneBound metazone minPos maxPos =
(
	nameToCoord metazone.posMin minPos
	nameToCoord metazone.posMax maxPos
	if (minPos[1] > maxPos[1]) then
	(
		tmp = minPos[1]
		minPos[1] = maxPos[1]
		maxPos[1] = tmp
	)
	if (minPos[2] > maxPos[2]) then
	(
		tmp = minPos[2]
		minPos[2] = maxPos[2]
		maxPos[2] = tmp
	)
)

-- File exist ?
fn fileExist filename = 
(
	file = openFile filename mode:"r"
	if file == undefined then
	(
		return false
	)
	else
	(
		close file
		return true
	)
)

-- Output error
fn outputError message =
(
	messagebox message
)

-- To uppercase
fn uppercase instring =                -- beginning of function definition 
(
	local upper, lower, outstring       -- declare variables as local 
	upper="ABCDEFGHIJKLMNOPQRSTUVWXYZ"  -- set variables to literals 
	lower="abcdefghijklmnopqrstuvwxyz" 
	outstring=instring
	for i=1 to outstring.count do 
	(  
		j=findString lower outstring[i] 
		if (j != undefined) do outstring[i]=upper[j] 
	) 
	return outstring     -- value of outstring will be returned as function result 
)

fn mergeNeighbor x y old_zone_directory trans =
(
	local name, node, src

	-- Get the file name
	name = coordToName #(x, y)

	-- Source filename
	src = old_zone_directory+"/"+name+".max"

	-- Delete the zone if exist
	node = getnodebyname name
	if (node != undefined) and ((classof node) == RklPatch) then
	(
		delete node
	)

	-- File exist ?
	if (fileExist (src)) then
	(
		-- Load the file
		name = uppercase (name)
		if mergeMAXFile src #(name) #mergeDups then
		(
			-- Get the node
			node = getnodebyname name
			if (node != undefined) then
			(
				node.transform = translate node.transform trans

				-- Collapse the node
				collapseStack node

				-- Set the appdata
				setAppData node NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX "1"
				setAppData node NEL3D_APPDATA_DONTEXPORT "1"
				node.isFrozen = true
			)
		)
	)
)

fn convertLandscape old_zone_directory ligo_directory metazones =
(
	-- For each meta zone
	for i in metazones do
	(
		-- Memory dump
		gc ()

		-- Get the coordinate of the metazone
		minPos = #()
		maxPos = #()
		getMetaZoneBound i minPos maxPos

		-- Get the translation for this metazone
		trans = [-cell_size*minPos[1], cell_size*(minPos[2] + (maxPos[2] - minPos[2] + 1)), 0.0]

		-- Metazone filename
		metazoneFilename = ligo_directory+"/max/zonematerial-converted-"+i.name+".max"

		----- If one old file in the metazone is more recent than dest metazone file or no old file exist
		
		exist = false
		export = false

		for y = minPos[2] to maxPos[2] do
		(
			x = 0
			for x = minPos[1] to maxPos[1] do
			(
				-- Get the file name
				name = coordToName #(x, y)

				-- Source filename
				src = old_zone_directory+"/"+name+".max"

				-- Does the file exist ?
				if (fileExist (src) == true) then
				(
					-- One file exist
					exist = true

					-- Need rebuild ?
					if (NeLTestFileDate metazoneFilename src) == true then
					(
						export = true
						exit
					)
				)
			)

			-- Breaked ?
			if (x > maxPos[1]) then
				exit
		)

		if ( (exist == false) or (export == true) ) then
		(
			-- Reset max
			resetMAXFile #noprompt
			
			-- For each old zone file in the meta zone
			for y = minPos[2] to maxPos[2] do
			(
				x = 0
				for x = minPos[1] to maxPos[1] do
				(
					-- Get the file name
					name = coordToName #(x, y)

					-- If the old file zone exist
					src = old_zone_directory+"/"+name+".max"
					
					-- Merge the new zone file in the destination file
					if (fileExist (src) == true) then
					(
						name = coordToName #(x, y)
						mergeMAXFile src #(name) #mergeDups

						-- Translate the node
						node = getnodebyname name
						if (node != undefined) then
						(
							node.transform = translate node.transform trans

							-- Collapse the node
							collapseStack node
							SetRykolTileSteps node -5

							-- Merge it
							if (getnodebyname "metazonetemp") == undefined then
							(
								node.name = "metazonetemp"
							)
							else
							(
								NeLAttachPatchMesh node $metazonetemp
							)
						)
					)
				)
			)

			-- Set the appdata
			setAppData $metazonetemp NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX "1"

			-- Weld it
			NeLWeldPatchMesh $metazonetemp weld_threshold

			-- Merge neighbor zones
			for x = minPos[1]-1 to maxPos[1]+1 do
			(
				if x >= 0 then
				(
					arrayY = #(minPos[2]-1, maxPos[2]+1)
					for y in arrayY do
					(
						if (y >= 0) then
						(
							-- Load neighbor								
							mergeNeighbor x y old_zone_directory trans
						)
					)
				)
			)
			for y = minPos[2] to maxPos[2] do
			(
				arrayX = #(minPos[1]-1, maxPos[1]+1)
				for x in arrayX do
				(
					if x >= 0 then
					(
						-- Load neighbor
						mergeNeighbor x y old_zone_directory trans
					)
				)
			)

			-- If file exist, load metazone project
			cont = true
			
			if (fileExist (metazoneFilename) == true) then
			(
				if (mergeMAXFile metazoneFilename #mergeDups) == false then
				(
					outputError ("Can't merge max file "+metazoneFilename)
					cont = false
				)
			)

			if cont == true then
			(
				-- Remove the patch mesh from max
				node = getnodebyname "metazone"
				if (node != undefined) and ((classof node) == RklPatch) then
				(
					delete node
				)

				-- Rename new zone
				$metazonetemp.name = "metazone"

				-- Save the meta zone
				saveMaxFile metazoneFilename
			)
		)
	)

	-- Get files in the zone_source_directory
	files = getFiles (old_zone_directory + "/*.max")
	for i in files do
	(
		-- Zone name
		name = uppercase (getFilenameFile i)

		-- Zone position
		pos = #()
		nameToCoord name pos

		-- Translation
		trans = [-cell_size*pos[1], cell_size*(pos[2] + 1), 0.0]

		-- Zone filename
		zoneFilename = ligo_directory+"/max/zonematerial-converted-"+name+".max"

		-- Need rebuild ?
		if (NeLTestFileDate zoneFilename i) == true then
		(
			-- Reset max
			resetMAXFile #noprompt

			-- Memory dump
			gc ()

			-- If file exist, load azone project
			cont = true
			if (fileExist (zoneFilename) == true) then
			(
				if loadMaxFile zoneFilename == false then
				(
					outputError ("Can't load max file "+zoneFilename)
					cont = false
				)
			)

			-- Continue ?
			if (cont == true) then
			(
				-- Delete the zone if exist
				node = getnodebyname name
				if (node != undefined) and ((classof node) == RklPatch) then
				(
					delete node
				)

				-- Load the file
				name = uppercase (getFilenameFile i)
				if mergeMAXFile i #(name) #mergeDups then
				(
					-- Get the node
					node = getnodebyname name
					if (node != undefined) then
					(
						node.transform = translate node.transform trans

						-- Set the appdata
						setAppData node NEL3D_APPDATA_LIGO_USE_BOUNDINGBOX "1"

						-- Merge neighbor zones
						for x = pos[1]-1 to pos[1]+1 do
						(
							if x >= 0 then
							(
								arrayY = #(pos[2]-1, pos[2]+1)
								for y in arrayY do
								(
									if (y >= 0) then
									(
										-- Load neighbor								
										mergeNeighbor x y old_zone_directory trans
									)
								)
							)
						)
						for y = pos[2] to pos[2] do
						(
							arrayX = #(pos[1]-1, pos[1]+1)
							for x in arrayX do
							(
								if x >= 0 then
								(
									-- Load neighbor
									mergeNeighbor x y old_zone_directory trans
								)
							)
						)

						-- Save the meta zone
						saveMaxFile zoneFilename
					)
				)
				else
				(
					outputError ("Can't load max file "+i)
				)
			)
		)
	)
)

-- Convert Fyros
convertLandscape old_zone_directory_fyros ligo_directory_fyros metazones_fyros

-- Convert Tryker
convertLandscape old_zone_directory_tryker ligo_directory_tryker metazones_tryker