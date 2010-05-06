-- Allocate 20 Me for the script
heapSize += 15000000

nlErrorFilename = "output_logfile"
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

try
(
	-- Get files in the skel_source_directory
	files = getFiles "skel_source_directory/*.max" 

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
				-- Output file
				output = ("output_directory/"+(getFilenameFile files[i])+".skel")

				-- Compare file date
				if (NeLTestFileDate output files[i]) == true then
				(
					-- Free memory and file handles
					gc ()

					-- Reset 3dsmax
					resetMAXFile #noprompt

					-- Open the max project
					nlerror ("Scanning file "+files[i]+" ...")
					if loadMaxFile files[i] == true then
					(
						-- Unhide category
						unhidecategory()

						-- Select Bip01, not very smart
						if $Bip01 != undefined then
						(
							-- Select Bip01
							select $Bip01

							if ($ != undefined) then
							(
								-- Set figure mode on
								if ((classof $) == Biped_Object) then
								(
									$.controller.figureMode = true
								)

								-- Export the skeleton template
								if (NelExportSkeleton $ output) == false then
								(
									nlerror ("ERROR exporting skeleton "+files[i])
								)
								else
								(
									nlerror ("OK "+output)
								)
							)
							else
							(
								-- Error
								nlerror ("ERROR exporting skeleton: no Bip01 node in file "+files[i])
							)
						)
						else
						(
							-- Error
							nlerror ("ERROR exporting skeleton: no Bip01 node in file "+files[i])
						)
					)
					else
					(
						-- Error
						nlerror ("ERROR exporting skeleton: can't open the file "+files[i])
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
				nlerror ("ERROR error exporting skeleton in files " + files[i])
			)
		)
	)
	else
	(
		nlerror ("WARNING no skeleton file in folder skel_source_directory")
	)
)
catch 
(
	-- Error
	nlerror ("ERROR fatal error exporting skeleton in folder skel_source_directory")
)

-- Bye

resetMAXFile #noprompt
quitMAX #noPrompt
quitMAX () #noPrompt

