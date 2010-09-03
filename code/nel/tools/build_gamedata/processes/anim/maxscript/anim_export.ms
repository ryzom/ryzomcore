

-- #################################################################
-- ## WARNING : this is a generated file, don't change it !
-- #################################################################


-- Allocate 20 Me for the script
heapSize += 15000000

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



NEL3D_APPDATA_EXPORT_NODE_ANIMATION = 1423062800

fn runNelMaxExport inputMaxFile = 
(
	outputNelFile = ("%OutputDirectory%/" + (getFilenameFile inputMaxFile) + ".anim")
	tagThisFile = false
	
	-- Unhide category
	unhidecategory()
	
	-- Select Bip01, not very smart
	if $Bip01 != undefined then
	(
		select $Bip01
		
		-- Always uncheck triangle pelvis
		if (classof $Bip01) == Biped_Object then
		(
			$Bip01.controller.figureMode = true
			$Bip01.controller.trianglepelvis = false
			$Bip01.controller.figureMode = false
		)
	)
	
	-- For each node
	for node in objects do
	(
		exportNodeAnmation = getappdata node NEL3D_APPDATA_EXPORT_NODE_ANIMATION
		if (exportNodeAnmation != undefined) then
		(
			if (exportNodeAnmation == "1") then
			(
				selectmore node
				
				-- Is it a biped ?
				if (classof node.controller) == Vertical_Horizontal_Turn then
				(
					-- Always uncheck triangle pelvis
					node.controller.trianglepelvis = false
				)
			)
		)
	)
	
	if ((selection as array).count != 0) then
	(
		-- Export the animation
		if (NelExportAnimation (selection as array) outputNelFile false) == false then
		(
			nlerror("ERROR exporting animation " + inputMaxFile)
			tagThisFile = false
		)
		else
		(
			nlerror("OK " + outputNelFile)
			tagThisFile = true
		)
	)
	else
	(
		-- Error
		nlerror("ERROR exporting animation: no node animated to export in file " + inputMaxFile)
	)
	
	return tagThisFile
)



try
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
			outputNelFile = ("%OutputDirectory%/" + (getFilenameFile inputMaxFile) + ".anim")
			
			try
			(
				-- Compare file date
				if (NeLTestFileDate outputNelFile inputMaxFile) == true then
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
						runNelMaxExport(inputMaxFile)
					)
					else
					(
						-- Error
						nlerror("ERROR exporting 'anim': can't open the file " + inputMaxFile)
					)
				)
				else
				(
					nlerror("SKIPPED " + inputMaxFile)
				)
			)
			catch 
			(
				-- Error
				nlerror("ERROR error exporting 'anim' in files " + inputMaxFile)
			)
		)
	)
	else
	(
		nlerror("WARNING no *.max file in folder %MaxSourceDirectory%")
	)
)
catch 
(
	-- Error
	nlerror("ERROR fatal error exporting 'anim' in folder %MaxSourceDirectory%")
)

-- Bye

resetMAXFile #noprompt
quitMAX #noPrompt
quitMAX() #noPrompt

