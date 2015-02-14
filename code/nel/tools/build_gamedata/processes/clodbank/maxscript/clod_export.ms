

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

NEL3D_APPDATA_DONOTEXPORT = 1423062565		-- do not export me : "undefined" = export me
											--							  "0" = export me
											--							  "1" = DONT export me
NEL3D_APPDATA_CHARACTER_LOD = 1423062618	-- "1": I am a character lod if "1". "0" or undefined: I am not.


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

fn runNelMaxExport inputMaxFile = 
(
	tagThisFile = true
	
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
					outputNelFile = ("%OutputDirectory%/" + (node.name) + ".clod")
					
					-- Compare file date
					if (NeLTestFileDate outputNelFile inputMaxFile) == true then
					(
						try
						(
							-- Export the shape
							if (NelExportLodCharacter node outputNelFile false) == true then
							(
								nlerror("OK " + outputNelFile)
								exported = exported+1
							)
							else
							(
								-- Error
								nlerror("ERROR exporting .clod " + node.name + " in file " + inputMaxFile)
								tagThisFile = false
							)
						)
						catch 
						(
							-- Error
							nlerror("ERROR fatal error exporting .clod " + node.name + " in file " + inputMaxFile)
							tagThisFile = false
						)
					)
					else
					(
						-- Error
						nlerror("SKIPPED " + outputNelFile)
						exported = exported + 1
					)
				)
			)
		)
	)
	
	-- Something exported
	if (exported == 0) then
	(
		-- Error
		nlerror ("WARNING no .clod exported from the file " + inputMaxFile)
	)
	
	return tagThisFile
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
							nlerror("ERROR exporting 'clod': can't open the file " + inputMaxFile)
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
				--	nlerror("ERROR error exporting 'clod' in file " + inputMaxFile)
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
	nlerror("ERROR Fatal error exporting 'clod' in folder %MaxSourceDirectory%")
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

