
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

fn getAbsoluteSource t =
(
	return (mapPaths.getFullFilePath ((getFilenameFile (filenameFromPath t)) + ".png"))
)

fn getAbsoluteDestination t =
(
	if ((findString t "\\\\Amiga") != undefined) then
	(
		return ("W:\\" + (substring (getFilenamePath t) 11 -1) + "\\" + (getFilenameFile (filenameFromPath t)) + ".png")
	)
	else if ((findString t "\\\\amiga") != undefined) then
	(
		return ("W:\\" + (substring (getFilenamePath t) 11 -1) + "\\" + (getFilenameFile (filenameFromPath t)) + ".png")
	)
	else if (((findString t "\\\\Database") != undefined) or ((findString t "\\\\database") != undefined)) and ((findString t "W:\\") == undefined) then
	(
		return ("W:\\" + (getFilenamePath t) + "\\" + (getFilenameFile (filenameFromPath t)) + ".png")
	)
	else if ((findString t "W:\\") != undefined) then
	(
		return ((getFilenamePath t) + "\\" + (getFilenameFile (filenameFromPath t)) + ".png")
	)
	else if ((findString t "E:\\Ryzom\\tryker") != undefined) then
	(
		return "W:\\database\\Stuff\\Tryker\\Decors\\_textures\\Batiments\\" + (getFilenameFile (filenameFromPath t)) + ".png"
	)
	else if ((findString t "\\tronc.") != undefined) then
	(
		return "W:\\database\\database_proto\\stuff\\fyros\\objects\\tronc.png"
	)
	else if ((findString t "\\trame.") != undefined) then
	(
		return "W:\\database\\stuff\\lod_actors\\texture_lod\\trame.png"
	)
	else if ((findString t "\\PR_MO_phytopsy_tete01_Boss.") != undefined) then
	(
		return "W:\\database\\Stuff\\Tryker\\Agents\\_textures\\monster\\PR_MO_phytopsy_tete01_Boss.png"
	)
	else
	(
		return t
	)
)

fn getMaxDestination mf =
(
	if ((findString mf "\\\\Amiga") != undefined) then
	(
		return ("W:\\" + (substring mf 11 -1))
	)
	else if ((findString mf "\\\\amiga") != undefined) then
	(
		return ("W:\\" + (substring mf 11 -1))
	)
	else if (((findString mf "\\\\Database") != undefined) or ((findString mf "\\\\database") != undefined)) and ((findString mf "W:\\") == undefined) then
	(
		return ("W:\\" + mf)
	)
	else
	(
		return mf
	)
)

fn getFixedTexturePath t =
(
	absoluteDestination = (getAbsoluteDestination t)
	if (not (doesFileExist absoluteDestination)) and (((findString t "\\Stuff\\Fyros\\Decors\\_textures\\Batiments") != undefined) or ((findString t "\\Stuff\\Fyros\\Decors\\_textures\\Accessories\\Vv2\\") != undefined)) then
	(
		testDestination = ("W:\\database\\stuff\\Fyros\\Agents\\_textures\\Accessories\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	if (not (doesFileExist absoluteDestination)) and (((findString t "\\Stuff\\Fyros\\Decors\\_textures\\Batiments") != undefined) or ((findString t "\\Stuff\\Fyros\\Decors\\_textures\\Accessories\\") != undefined)) then
	(
		testDestination = ("W:\\database\\stuff\\Fyros\\Agents\\_textures\\Accessories\\Vv2\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	if (not (doesFileExist absoluteDestination)) and ((findString t "\\Stuff\\Fyros\\Decors\\_textures\\Accessories") != undefined) then
	(
		testDestination = ("W:\\database\\stuff\\Fyros\\Agents\\_textures\\Batiments\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	if (not (doesFileExist absoluteDestination)) and ((findString t "\\Stuff\\Tryker\\Decors\\_textures\\batiments") != undefined) then
	(
		testDestination = ("W:\\database\\Stuff\\Tryker\\Decors\\_textures\\batiments\\noutilise\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	if (not (doesFileExist absoluteDestination)) and ((findString t "\\Stuff\\Tryker\\Decors\\_textures\\batiments") != undefined) then
	(
		testDestination = ("W:\\database\\Stuff\\Tryker\\Decors\\_textures\\batiments\\vv2\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	if (not (doesFileExist absoluteDestination)) and ((findString t "\\Stuff\\Tryker\\Decors\\_textures\\batiments") != undefined) then
	(
		testDestination = ("W:\\database\\Stuff\\Tryker\\Decors\\_textures\\batiments\\tempautel\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	if (not (doesFileExist absoluteDestination)) and ((findString t "\\Stuff\\Tryker\\Decors\\_textures\\batiments") != undefined) then
	(
		testDestination = ("W:\\database\\Stuff\\Tryker\\Decors\\_textures\\batiments\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	if (not (doesFileExist absoluteDestination)) and ((findString t "\\Stuff\\Fyros\\Decors\\_textures") != undefined) then
	(
		testDestination = ("W:\\database\\Stuff\\Fyros\\Decors\\_textures\\Batiments\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	if (not (doesFileExist absoluteDestination)) and ((findString t "\\Stuff\\Fyros\\Decors\\_textures") != undefined) then
	(
		testDestination = ("W:\\database\\Stuff\\Tryker\\Decors\\_textures\\Batiments\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	if (not (doesFileExist absoluteDestination)) and ((findString t "\\sky_V2\\textures\\textures") != undefined) then
	(
		testDestination = ("W:\\database\\sky_V2\\textures\\textures\\nodds\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then (return testDestination)
		testDestination = ("W:\\database\\sky_V2\\textures\\textures\\desert\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then (return testDestination)
		testDestination = ("W:\\database\\sky_V2\\textures\\textures\\forest\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then (return testDestination)
		testDestination = ("W:\\database\\sky_V2\\textures\\textures\\jungle\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then (return testDestination)
		testDestination = ("W:\\database\\sky_V2\\textures\\textures\\lacustre\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then (return testDestination)
	)
	return absoluteDestination
)

fn renameTexture t =
(
	try
	(
		if (t != undefined) then
		(
			if (classof t == NelBitmapTexture) then
			(
				if (t.bitmap1FileName != "") then (t.bitmap1FileName = getFixedTexturePath t.bitmap1FileName)
				if (t.bitmap2FileName != "") then (t.bitmap2FileName = getFixedTexturePath t.bitmap2FileName)
				if (t.bitmap3FileName != "") then (t.bitmap3FileName = getFixedTexturePath t.bitmap3FileName)
				if (t.bitmap4FileName != "") then (t.bitmap4FileName = getFixedTexturePath t.bitmap4FileName)
				if (t.bitmap5FileName != "") then (t.bitmap5FileName = getFixedTexturePath t.bitmap5FileName)
				if (t.bitmap6FileName != "") then (t.bitmap6FileName = getFixedTexturePath t.bitmap6FileName)
				if (t.bitmap7FileName != "") then (t.bitmap7FileName = getFixedTexturePath t.bitmap7FileName)
				if (t.bitmap8FileName != "") then (t.bitmap8FileName = getFixedTexturePath t.bitmap8FileName)
				renameTexture t.bitmap
				if (t.bitmap.fileName == undefined) then
				(
					if (doesFileExist (mapPaths.getFullFilePath t.bitmap1FileName)) then (t.bitmap.fileName = t.bitmap1FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap2FileName)) then (t.bitmap.fileName = t.bitmap2FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap3FileName)) then (t.bitmap.fileName = t.bitmap3FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap4FileName)) then (t.bitmap.fileName = t.bitmap4FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap5FileName)) then (t.bitmap.fileName = t.bitmap5FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap6FileName)) then (t.bitmap.fileName = t.bitmap6FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap7FileName)) then (t.bitmap.fileName = t.bitmap7FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap8FileName)) then (t.bitmap.fileName = t.bitmap8FileName)
				)
				else if not (doesFileExist (mapPaths.getFullFilePath t.bitmap.fileName)) then
				(
					if (doesFileExist (mapPaths.getFullFilePath t.bitmap1FileName)) then (t.bitmap.fileName = t.bitmap1FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap2FileName)) then (t.bitmap.fileName = t.bitmap2FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap3FileName)) then (t.bitmap.fileName = t.bitmap3FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap4FileName)) then (t.bitmap.fileName = t.bitmap4FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap5FileName)) then (t.bitmap.fileName = t.bitmap5FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap6FileName)) then (t.bitmap.fileName = t.bitmap6FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap7FileName)) then (t.bitmap.fileName = t.bitmap7FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap8FileName)) then (t.bitmap.fileName = t.bitmap8FileName)
				)
				t.delegate.RGBOutput = 0
				t.delegate.monoOutput = 1
				t.delegate.alphasource = 2
			)
			else if (classof t == Reflect_Refract) then
			(
				if (t.bitmapName[1] != undefined) then (t.bitmapName[1] = getFixedTexturePath t.bitmapName[1])
				if (t.bitmapName[2] != undefined) then (t.bitmapName[2] = getFixedTexturePath t.bitmapName[2])
				if (t.bitmapName[3] != undefined) then (t.bitmapName[3] = getFixedTexturePath t.bitmapName[3])
				if (t.bitmapName[4] != undefined) then (t.bitmapName[4] = getFixedTexturePath t.bitmapName[4])
				if (t.bitmapName[5] != undefined) then (t.bitmapName[5] = getFixedTexturePath t.bitmapName[5])
				if (t.bitmapName[6] != undefined) then (t.bitmapName[6] = getFixedTexturePath t.bitmapName[6])
				if (t.outputname != undefined) then (t.outputname = getFixedTexturePath t.outputname)
			)
			else
			(
				if (t.fileName != undefined) then (t.fileName = getFixedTexturePath t.fileName)
				if (classof t == BitmapTexture) then
				(
					t.RGBOutput = 0
					t.monoOutput = 1
					t.alphasource = 2
				)
			)
		)
	)
	catch
	(
	
	)
)


rollout assets_png_rollout "Properties"
(
	fn do_it =
	(
		for m in getClassInstances BitmapTexture do
		(
			renameTexture m
		)
		
		for m in getClassInstances NelBitmapTexture do
		(
			renameTexture m
		)
		
		for m in getClassInstances NelMaterial do
		(
			renameTexture m.tTexture_1
			renameTexture m.tTexture_2
			renameTexture m.tTexture_3
			renameTexture m.tTexture_4
			renameTexture m.tTexture_5
			renameTexture m.tTexture_6
			renameTexture m.tTexture_7
			renameTexture m.tTexture_8
			m.delegate.DiffuseMapEnable = m.bEnableSlot_1
			m.delegate.DiffuseMap = m.tTexture_1
			m.delegate.AmbientMapEnable = m.bEnableSlot_1
			m.delegate.AmbientMap = m.tTexture_1
			m.delegate.SpecularMapEnable = m.bEnableSlot_2
			m.delegate.SpecularMap = m.tTexture_2
			m.delegate.SelfIllumMap = undefined
			m.delegate.OpacityMap = undefined
			m.delegate.FilterMap = undefined
			m.delegate.BumpMap = undefined
			m.delegate.DisplacementMap = undefined
			m.delegate.ReflectionMap = undefined
			m.delegate.RefractionMap = undefined
		)
		
		max select none
		
		for node in geometry do
		(
			itfMaxFile = getAppData node NEL3D_APPDATA_INTERFACE_FILE
			if (itfMaxFile != undefined) then
			(
				if (itfMaxFile != "") then
				(
					itfMaxFile = getMaxDestination itfMaxFile
					setAppData node NEL3D_APPDATA_INTERFACE_FILE itfMaxFile
				)
			)
		)
		
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
	--	The function should retun -1 if an arror occurred, else the count of modification done
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

assets_png_floater = newRolloutFloater "NeL Assets PNG Database" 550 874
addrollout assets_png_rollout assets_png_floater rolledUp:false

