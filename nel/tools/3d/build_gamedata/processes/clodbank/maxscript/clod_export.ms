-- Some globals

NEL3D_APPDATA_DONOTEXPORT = 1423062565		-- do not export me : "undefined" = export me
											--							  "0" = export me
											--							  "1" = DONT export me
NEL3D_APPDATA_CHARACTER_LOD = 1423062618	-- "1": I am a character lod if "1". "0" or undefined: I am not.


-- Allocate 20 Me for the script
heapSize += 15000000

nlErrorFilename = "output_logfile"
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

-- Must export this node ?
fn isToBeExported node =
(
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
	return true
)

-- is this node flagged as a LodCharacter ??
fn isLodCharacter node =
(
	isCLod = getappdata node NEL3D_APPDATA_CHARACTER_LOD
	if (isCLod == undefined) then
		return false
	if (isCLod == "1") then
		return true
	return false
)

try
(
	-- Get files in the shape_source_directory
	files = getFiles "shape_source_directory/*.max" 

	-- Sort files
	sort files

	-- No file ?
	if files.count != 0 then
	(
		-- For each files
		for i = 1 to files.count do
		(
			try
			(
				-- Ok ?
				ok = false

				-- Free memory and file handles
				gc ()

				-- Reset 3dsmax
				resetMAXFile #noprompt

				-- Get the tag file name
				tag = ("output_directory_tag/"+(getFilenameFile files[i])+(getFilenameType files[i])+".tag")

				-- Compare date with the tag file
				if (NeLTestFileDate tag files[i]) == true then
				(
					-- Open the max project
					nlerror ("Scanning file "+files[i]+" ...")
					if loadMaxFile files[i] == true then
					(
						-- Unhide category
						unhidecategory()

						tagThisFile = true

						-- Unhide
						max unhide all

						-- unselect
						max select none

						-- Exported object count
						exported = 0

						-- For each node
						for node in geometry do
						(
							-- It is root ?
							if (node.parent == undefined) then
							(
								-- Can be exported ?
								if (isToBeExported node == true) then
								(
									-- Is a Lod character?
									if ((isLodCharacter node) == true) then
									(
										-- Output directory
										output = ("output_directory_clod/"+(node.name)+".clod")

										-- Compare file date
										if (NeLTestFileDate output files[i]) == true then
										(
											try
											(
												-- Export the shape
												if (NelExportLodCharacter node output false) == true then
												(
													nlerror ("OK "+output)
													exported = exported+1
												)
												else
												(
													-- Error
													nlerror ("ERROR exporting .clod "+node.name+" in file "+files[i])
													tagThisFile = false
												)
											)
											catch 
											(
												-- Error
												nlerror ("ERROR fatal error exporting .clod "+node.name+" in file "+files[i])
												tagThisFile = false
											)
										)
										else
										(
											-- Error
											nlerror ("SKIPPED "+output)
											exported = exported+1
										)
									)
								)
							)
						)

						-- Something exported
						if (exported == 0) then
						(
							-- Error
							nlerror ("WARNING no .clod exported from the file "+files[i])
						)

						-- Write a tag file
						if tagThisFile == true then
						(
							tagFile = createFile tag
							if tagFile == undefined then
							(
								nlerror ("WARNING can't create tag file "+tag)
							)
							else
							(
								print "toto" to: tagFile
								close tagFile
							)
						)
					)
					else
					(
						-- Error
						nlerror ("ERROR exporting .clod: can't open the file "+files[i])
					)
				)
				else
				(
					-- Error
					nlerror ("SKIPPED BY TAG "+files[i])
				)
			)
			catch 
			(
				-- Error
				nlerror ("ERROR error exporting .clod in files " + files[i])
			)
		)
	)
	else
	(
		nlerror ("WARNING no max file in folder shape_source_directory")
	)
)
catch 
(
	-- Error
	nlerror ("ERROR fatal error exporting .clod in folder shape_source_directory")
)

-- Bye
gc ()
resetMAXFile #noprompt

quitMAX #noPrompt
quitMAX () #noPrompt

