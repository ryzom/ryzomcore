

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



-- Find id
Fn findID node =
(
	local

	-- Const
	alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZ" 
	NameTab = filterString node.name "_"
	Z_ID = -1
	alpha_letter1 = NameTab[2][1]
	alpha_letter2 = NameTab[2][2]
	alpha_letter1_value = findstring alphabet alpha_letter1 
	alpha_letter2_value = findstring alphabet alpha_letter2 

	-- Decompose theh name in an array array[1]=numeric string value  array[2]=alpha string value
	-- The index of the engine start at 0 but the script one at 1 so we sub 1 each time
	alpha_sub_id = (((alpha_letter1_value as integer - 1) * 26 + (alpha_letter2_value as integer)))-1
	num_sub_id = (NameTab[1] as integer)-1

		-- Array of 256 per 256
		---------------------------
		--	0	1	2	3	...	255
		--	256	257	258	259	...	511 
		--	512	513	514	515	...	767
		--	...

	Z_ID = num_sub_id*256 + alpha_sub_id
	return Z_ID
)

fn runNelMaxExport inputMaxFile = 
(
	outputNelFile = ("%OutputDirectory%/" + (getFilenameFile inputMaxFile) + ".zone")
	tagThisFile = false
	
	-- Unhide category
	unhidecategory()
	
	-- Select none
	max select none
	
	-- Found it ?
	find = false
	
	-- For each object in the priject
	for i in geometry do
	(
		-- Look for a NeL patch mesh
		if (classof i) == RklPatch then
		(
			-- Error catching
			try
			(
				if (ExportRykolZone i outputNelFile (findID i)) == false then
				(
					nlerror("ERROR exporting zone " + i.name + " in file " + inputMaxFile)
				)
				else
				(
					nlerror("OK " + outputNelFile)
					tagThisFile = true
					find = true
					exit
				)
			)
			catch
			(
				-- Error
				nlerror("ERROR fatal error exporting zone " + i.name + " in file " + inputMaxFile)
				tagThisFile = false
			)
		)
	)
	
	-- Not found ?
	if (find == false) then
	(
		-- Error
		nlerror("WARNING no zone found in project " + inputMaxFile)
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
			outputNelFile = ("%OutputDirectory%/" + (getFilenameFile inputMaxFile) + ".zone")
			
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
						nlerror("ERROR exporting 'zone': can't open the file " + inputMaxFile)
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
				nlerror("ERROR error exporting 'zone' in files " + inputMaxFile)
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
	nlerror("ERROR fatal error exporting 'zone' in folder %MaxSourceDirectory%")
)

-- Bye

resetMAXFile #noprompt
quitMAX #noPrompt
quitMAX() #noPrompt

