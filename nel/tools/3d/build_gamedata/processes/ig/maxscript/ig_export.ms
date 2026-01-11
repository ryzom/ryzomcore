-- Some globals

NEL3D_APPDATA_ACCEL = 1423062561			-- type of accelerator :	"32" = is not an accelerator and IS clusterized 
											--							 "0" = is not an accelerator and IS NOT clusterized (always visible)
											--							 "1" = is an accelerator type PORTAL
											--							 "2" = is an accelerator type CLUSTER
											--							 "6" = is an accelerator type CLUSTER FATHER-VISIBLE
											--							"10" = is an accelerator type CLUSTER VISIBLE-FROM-FATHER
											--							"14" = is an accelerator type CLUSTER FATHER-VISIBLE and VISIBLE-FROM-FATHER
											--							"17" = is an accelerator type PORTAL DYNAMIC
																			 
NEL3D_APPDATA_DONOTEXPORT = 1423062565		-- do not export me : "undefined" = export me
											--							  "0" = export me
											--							  "1" = DONT export me

NEL3D_APPDATA_IGNAME = 1423062564			-- string : name of the Instance Group

NEL3D_APPDATA_LOD_NAME_COUNT_MAX	= 10
NEL3D_APPDATA_LOD					= 1423062537
NEL3D_APPDATA_LOD_NAME_COUNT		= NEL3D_APPDATA_LOD
NEL3D_APPDATA_LOD_NAME				= NEL3D_APPDATA_LOD_NAME_COUNT+1
NEL3D_APPDATA_LOD_BLEND_IN			= NEL3D_APPDATA_LOD_NAME+NEL3D_APPDATA_LOD_NAME_COUNT_MAX
NEL3D_APPDATA_LOD_BLEND_OUT			= NEL3D_APPDATA_LOD_BLEND_IN+1
NEL3D_APPDATA_LOD_COARSE_MESH		= NEL3D_APPDATA_LOD_BLEND_OUT+1

NEL_OBJECT_NAME_DATA				= 1970

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

-- This node is n accelerator ?
fn isAccelerator node =
(
	accel = getappdata node NEL3D_APPDATA_ACCEL
	if (accel != undefined) then
	(
		if (accel == "0") or (accel == "32") then
			return false
		else
			return true
	)
	return false
)

-- Get the ig name of this object
fn getIg node =
(
	return (getappdata node NEL3D_APPDATA_IGNAME)
)

try
(
	-- Get files in the ig_source_directory
	files = getFiles "ig_source_directory/*.max" 

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

						-- Ig array
						ig_array = #()

						-- Scan all the ig in this project
						for node in objects do
						(
							ig = getIg node
							if ( (ig != undefined) and (ig != "") ) then
							(
								-- Found ?
								found = false

								-- Already found ?
								for j = 1 to ig_array.count do
								(
									if (ig_array[j]==ig) then
									(								
										found = true
										exit
									)
								)

								-- Found ?
								if (found == false) then
								(
									append ig_array ig
								)
							)
						)
	
						-- Have some ig ?
						if (ig_array.count != 0) then
						(
							-- For each ig
							for ig = 1 to ig_array.count do
							(
								-- Output filename
								output = ("output_directory_ig/"+ig_array[ig]+".ig")

								-- Check date
								if (NeLTestFileDate output files[i]) == true then
								(
									-- Select none
									max select none

									-- Select all node in this ig
									for node in geometry do
									(
										-- Select it if in the ig
										if ( (getIg node) == ig_array[ig]) then
											selectmore node
									)
									-- Select all lights in this ig
									for node in lights do
									(
										-- Select it if in the ig
										if ( (getIg node) == ig_array[ig]) then
											selectmore node
									)
									-- Select all lights in this ig
									for node in helpers do
									(
										-- Select it if in the ig
										if ( (getIg node) == ig_array[ig]) then
											selectmore node
									)

									-- Check export
									try
									(
										-- Export the ig
										instancegroup2export = $selection as array
										if (NelExportInstanceGroup instancegroup2export output) == true then
										(
											nlerror ("OK "+output)
										)
										else
										(
											-- Error
											nlerror ("ERROR exporting ig "+ig_array[ig]+" in file "+files[i])
											tagThisFile = false
										)
									)
									catch
									(
										-- Error
										nlerror ("ERROR fatal error exporting ig "+ig_array[ig]+" in file "+files[i])
										tagThisFile = false
									)
								)
								else
								(
									nlerror ("SKIPPED "+output)
								)
							)
						)
						else
						(
							-- Error
							nlerror ("WARNING nothing exported from ig max file "+files[i])
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
						nlerror ("ERROR exporting ig: can't open the file "+files[i])
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
				nlerror ("ERROR error exporting ig in files " + files[i])
			)
		)
	)
	else
	(
		nlerror ("WARNING no max file in folder ig_source_directory")
	)
)
catch 
(
	-- Error
	nlerror ("ERROR fatal error exporting ig in folder ig_source_directory")
)

-- Bye

resetMAXFile #noprompt
quitMAX #noPrompt
quitMAX () #noPrompt

