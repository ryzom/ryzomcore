

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



NEL3D_APPDATA_COLLISION					= 1423062613
NEL3D_APPDATA_COLLISION_EXTERIOR		= 1423062614


-- Must export this node ?
fn isToBeExported node =
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

fn runNelMaxExport inputMaxFile = 
(
	outputNelDir = "%OutputDirectory%"
	
	-- Tag this file ?
	tagThisFile = true
	
	-- Unhide category
	unhidecategory()
	
	-- Select all collision mesh
	max select none
	clearSelection()
	for m in geometry do
	(
		if (isToBeExported m) == true then
			selectmore m
	)
	
	-- Export the collision
	if (NelExportCollision ($selection as array) outputNelDir) == false then
	(
		nlerror("ERROR exporting collision " + inputMaxFile)
		tagThisFile = false
	)
	else
	(
		nlerror("OK collision in folder " + outputNelDir)
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
							nlerror("ERROR exporting 'cmb': can't open the file " + inputMaxFile)
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
				--	nlerror("ERROR error exporting 'cmb' in file " + inputMaxFile)
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
	nlerror("ERROR Fatal error exporting 'cmb' in folder %MaxSourceDirectory%")
	nlerror("FAIL Fatal error occurred")
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
	nlerror("FAIL One or more issues occurred")
	NelForceQuitRightNow()
)

-- Bye
nlerror("BYE")
quitMAX #noPrompt
quitMAX() #noPrompt

