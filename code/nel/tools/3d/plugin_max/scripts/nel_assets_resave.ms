
NEL3D_APPDATA_INTERFACE_FILE = 1423062700

-- Allocate 20 Me for the script
heapSize += 15000000

nlErrorFilename = "W:/database/conversion.log"
nlErrorStream = openFile nlErrorFilename mode:"w"
if nlErrorStream == undefined then
	nlErrorStream = createFile nlErrorFilename

-- Log a message
fn nllog message =
(
	if nlErrorStream != undefined then
	(
		format "%\n" message to:nlErrorStream
		flush nlErrorStream
	)

	-- To the console
	print message
)

include "nel_utility.ms"

rollout assets_resave_rollout "Properties"
(
	fn do_it =
	(
		max select none
		
		actionMan.executeAction 0 "40021"  -- Selection: Select All
		actionMan.executeAction 0 "311"  -- Tools: Zoom Extents All Selected
		actionMan.executeAction 0 "40807"  -- Views: Activate All Maps
		actionMan.executeAction 0 "63508"  -- Views: Standard Display with Maps
		actionMan.executeAction 0 "40043"  -- Selection: Select None
		
		max views redraw
		
		return 1
	)
	
	-- This script is a base script to include to add multiple functionality to your script

	-- To use this script
	--	Include it in your script into the rollout at the beginning.
	--	Implement a do_it function to do the job in your rollout.
	--	The function should retun -1 if an arror occured, else the count of modification done
	--	It the function returns <1, the project will not be overwritten

	Group "Running properties"
	(
		RadioButtons	SourceFiles				"Source projects"				labels:#("Current project", "All Projects in a folder") align:#left

		Label			DirectoryLabel			"Source directory"							align:#left
		EditText		Directory				""								width:500	align:#left enabled:false
		Button			BrowseDirectory			"Browse..."						align:#left enabled:false

		CheckBox		Recurse					"Look in subfolders"			checked:true enabled:false
		CheckBox		Test					"Test only, do not save"		checked:false enabled:false
		CheckBox		BackupFiles				"Backup files"					checked:false enabled:false
		CheckBox		StopOnError				"Stop on error"					checked:false enabled:false
		CheckBox		UseTag					"Use tag"						checked:false enabled:false

		Label			ProgressText											width:500	align:#left
		ProgressBar		Progress												width:500	align:#left

		Button 			GoButton				"Go"							width:500	align:#left
	)
	
	local countModifications
	local countErrors
	local fileModified
	local fileParsed

	fn UpdateData =
	(
		if SourceFiles.state == 2 then
			isSourceDir = true
		else
			isSourceDir = false
		if Test.checked == true then
			isTest = true
		else
			isTest = false

		Directory.enabled = isSourceDir
		BrowseDirectory.enabled = isSourceDir
		Recurse.enabled = isSourceDir
		Test.enabled = isSourceDir
		BackupFiles.enabled = isSourceDir and (isTest == false)
		StopOnError.enabled = isSourceDir
		UseTag.enabled = isSourceDir
	)

	on SourceFiles changed state do
	(
		UpdateData ()
	)

	on Test changed state do
	(
		UpdateData ()
	)

	fn call_do_it =
	(
		local result

		-- One more project
		fileParsed = fileParsed + 1

		-- Call it
		result = do_it ()

		-- Error ?
		if result < 0 then
			countErrors = countErrors + 1
		else 
			countModifications = countModifications + result

		-- Return result
		return result
	)

	fn BackupFile file =
	(
		local i
		local newFilename

		i = 0
		while true do
		(
			-- New file name
			newFilename = file + ".backup_" + (i as string)

			-- File exist ?
			if (fileExist newFilename) == false then
			(
				if (copyFile file newFilename) == false then 
					return false
				else 
					return true
			)
			i = i + 1
		)
	)

	fn RecurseFolder currentDirectory =
	(
		resetMAXFile #noprompt
		
		local result
		local file
		local files
		local origAnimStart
		local origAnimEnd
		local origFrameRate
		
		-- Parse files
		files = getFiles (currentDirectory+"/*.max")
		
		-- For each files
		for i = 1 to files.count do
		(
			-- File name
			file = files[i]
			
			-- Progress bar
			ProgressText.text  = "In directory "+currentDirectory+", compute file \"" + (getFilenameFile file) + "\""
			Progress.value = i*100/files.count
			
			if (UseTag.checked == false) or ((NeLTestFileDate file "W:/database/conversion.tag") == true) then
			(
				resetMAXFile #noprompt
				
				nllog("CONVERT " + file)
				
				-- Open the max project
				if loadMaxFile file quiet:true == true then
				(
					origAnimStart = animationRange.start
					origAnimEnd = animationRange.end
					origFrameRate = frameRate
					
					resetMAXFile #noprompt
					
					animationRange = interval origAnimStart origAnimEnd
					frameRate = origFrameRate
					
					-- Merge the max project
					if mergeMaxFile file quiet:true == true then
					(
						result = call_do_it () 
						
						-- Error ?
						if result < 0 then
						(
							if StopOnError.checked == true then
								Messagebox ("Error in file " + file)
						)
						else
						(
							-- Save the max project ?
							if (Test.checked == false) and (result != 0) then
							(
								-- Backup the max project ?
								local ok
								ok = true
								if BackupFiles.checked == true then
								(
									-- Backup the file
									if (BackupFile file) == false then
									(
										-- Don't save the file because backup has failed
										ok = false

										if StopOnError.checked == true then
											Messagebox ("Can't backup file " + file)

										-- One more error
										countErrors = countErrors + 1
									)
								)

								-- Save the max project ?
								if ok == true then
								(
									if (saveMaxFile file) == true then
									(
										fileModified = fileModified + 1
									)
									else
									(
										if StopOnError.checked == true then
											Messagebox ("Can't write file " + file)

										-- One more error
										countErrors = countErrors + 1
									)
								)
							)
						)
					)
					else
					(
						if StopOnError.checked == true then
							Messagebox ("Can't load file " + file)

						-- One more error
						countErrors = countErrors + 1
					)
				)
			)
			else
			(
				nllog("SKIP " + file + " by tag")
			)
		)
		
		-- Parse sub directory ?
		if (Recurse.checked == true) then
		(
			local directories

			-- Get the directories
			directories = getDirectories (currentDirectory+"/*")

			-- For each directories
			for dir in directories do
			(
				RecurseFolder dir
			)
		)
	)
	
	on BrowseDirectory pressed do
	(
		local dir
		try
		(
			dir = getSavePath () -- caption:"Select the projects directory"
			if dir != undefined then
				Directory.text = dir
		)
		catch 
		(
		)
	)
	
	on GoButton pressed do
	(
		-- Reset count
		countModifications = 0
		countErrors = 0
		fileModified = 0
		fileParsed = 0
		
		-- Get files in the shape_source_directory
		if SourceFiles.state == 2 then
		(
			-- Should warning user ?
			if (SourceFiles.state == 2) and (Test.checked == false) then
			(
				-- Warning !
				if ((queryBox "Warning, all the files in the specified folders will be overwrited.\nYou should backup your files before executing this script.\nDo you want to continue executing this script ?" beep:true) == true) then
					RecurseFolder (adjustPathStringForScript Directory.text)
			)
			else
			(
				RecurseFolder (adjustPathStringForScript Directory.text)
			)
		)
		else
		(
			-- Just compute the current project
			call_do_it ()
		)
		
		-- Show errors
		ProgressText.text  = (fileParsed as string) + " project(s) opened, " + (countModifications as string) + " project modification(s), " + (fileModified as string) + " project(s) saved, " + (countErrors as string) + " error(s)."
		Progress.value = 100
	)
)

assets_resave_floater = newRolloutFloater "NeL Assets Resave Database" 550 874
addrollout assets_resave_rollout assets_resave_floater rolledUp:false

