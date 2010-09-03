

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



fn runNelMaxExport inputMaxFile = 
(
	outputNelFile = ("%OutputDirectory%/" + (getFilenameFile inputMaxFile) + ".pacs_prim")
	tagThisFile = false
	
	-- Unhide category
	unhidecategory()
	
	-- Select none
	max select none
	
	-- Select all PACS primitives
	for i in geometry do
	(
		if ((classof i) == nel_pacs_cylinder) or ((classof i) == nel_pacs_box) then
			selectmore i
	)
	
	-- Array of node
	arrayNode = selection as array
	
	-- Something to export ?
	if (arrayNode.count != 0) then
	(
		-- Export the collision
		if (NelExportPACSPrimitives arrayNode outputNelFile) == false then
		(
			nlerror("ERROR exporting PACS primitives in file " + inputMaxFile)
			tagThisFile = false
		)
		else
		(
			nlerror("OK PACS primitives in file " + inputMaxFile)
			tagThisFile = true
		)
	)
	else
	(
		nlerror("WARNING no PACS primitives in file " + inputMaxFile)
		tagThisFile = true
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
			outputNelFile = ("%OutputDirectory%/" + (getFilenameFile inputMaxFile) + ".pacs_prim")
			
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
						nlerror("ERROR exporting 'pacs_prim': can't open the file " + inputMaxFile)
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
				nlerror("ERROR error exporting 'pacs_prim' in files " + inputMaxFile)
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
	nlerror("ERROR fatal error exporting 'pacs_prim' in folder %MaxSourceDirectory%")
)

-- Bye

resetMAXFile #noprompt
quitMAX #noPrompt
quitMAX() #noPrompt

