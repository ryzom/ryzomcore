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

try
(
	-- Get files in the zone_source_directory
	files = getFiles "zone_source_directory/*.max" 

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

				-- Output file
				output = ("output_directory/"+(getFilenameFile files[i])+".zone")

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

						-- Select none
						max select none

						-- Found it ?
						find = false

						-- For each object in the priject
						for i in geometry do
						(
							-- Look for a NeL patch mesh
							if (classof i)==RklPatch then
							(
								-- Error catching
								try
								(
									if (ExportRykolZone i output (findID i)) == false then
									(
										nlerror ("ERROR exporting zone "+i.name+" in file "+files[i])
									)
									else
									(
										nlerror ("OK "+output)
										find = true
										exit
									)
								)
								catch
								(
									-- Error
									nlerror ("ERROR fata error exporting zone "+i.name+" in file "+files[i])
								)
							)
						)

						-- Not found ?
						if (find == false) then
						(
							-- Error
							nlerror ("WARNING no zone found in project "+files[i])
						)
					)
					else
					(
						-- Error
						nlerror ("ERROR exporting zone: can't open the file "+files[i])
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
				nlerror ("ERROR error exporting zone in files " + files[i])
			)
		)
	)
	else
	(
		nlerror ("WARNING no zone file in folder zone_source_directory")
	)
)
catch 
(
	-- Error
	nlerror ("ERROR fatal error exporting zone in folder zone_source_directory")
)

-- Bye

resetMAXFile #noprompt
quitMAX #noPrompt
quitMAX () #noPrompt

