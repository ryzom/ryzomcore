
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

try
(
	-- Get files in the "pacs_prim_source_directory" directory
	files = getFiles "pacs_prim_source_directory/*.max" 
	gc ()

	-- Sort files
	sort files
	gc ()

	-- No file ?
	if files.count != 0 then
	(
		-- For each files
		for i = 1 to files.count do
		(
			try
			(
				-- Output file
				output = "output_directory/" + (getFilenameFile files[i]) + ".pacs_prim"

				-- Compare file date
				if (NeLTestFileDate output files[i]) == true then
				(
					-- Free memory and file handles
					gc ()
					heapfree

					-- Reset 3dsmax
					resetMAXFile #noprompt

					-- Open the max project
					nlerror ("Scanning file "+files[i]+" ...")
					if loadMaxFile files[i] == true then
					(
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
							if (NelExportPACSPrimitives arrayNode output) == false then
							(
								nlerror ("ERROR exporting PACS primitives in file "+files[i])
							)
							else
							(
								nlerror ("OK PACS primitives in file "+files[i])
							)
						)
						else
						(
							nlerror ("WARNING no PACS primitives in file "+files[i])
						)
					)
					else
					(
						-- Error
						nlerror ("ERROR exporting collision: can't open the file "+files[i])
					)
				)
				else
				(
					nlerror ("SKIPPED "+files[i])
				)
			)
			catch 
			(
				-- Error
				nlerror ("ERROR error exporting collision in files " + files[i])
			)
		)
	)
	else
	(
		nlerror ("WARNING no collision file in folder pacs_prim_source_directory")
	)
)
catch 
(
	-- Error
	nlerror ("ERROR fatal error exporting collision in folder pacs_prim_source_directory")
)

-- Bye

resetMAXFile #noprompt
quitMAX #noPrompt
quitMAX () #noPrompt

