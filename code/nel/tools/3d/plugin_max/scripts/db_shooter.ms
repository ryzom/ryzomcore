
-- Script configuration

DefaultPostFixe = "_nel_shoot.tga"

rollout db_shooter_rollout "Database Picture Shooter"
(
	Label RootDataBaseLabel "Root path: " align:#left
	EditText RootDataBase text:"c:\database" align:#left
	Spinner Width "Width:" range:[1,10000,320] type:#integer align:#left
	Spinner Height "Height:" range:[1,10000,200] type:#integer align:#left
	CheckBox Recurse "Recurse sub directories" checked:true align:#left
	CheckBox ReportErrors "Report errors" checked:true align:#left
	Button ShootTheDatabase "Shoot" width:110 align:#left
	Button ClearShootTheDatabase "Clear Shoot" width:110 align:#left

	fn shoot_directory path =
	(
		-- Make sure the path name is formatted
		if path.count > 0 then
		(
			-- Get last character
			lastChar = path[path.count]

			if ( lastChar != '\\' ) and ( lastChar != '/' ) then
			(
				path += "\\"
			)
		)

		-- List the file in this folder
		files = getFiles (path+"*.max")

		-- For each filename
		for i in files do
		(
			-- Open the project
			if (loadMaxFile i) == true then
			(
				-- Front view
				max vpt front

				-- Zoom
				max tool zoomextents all
				max tool zoomextents all
				max tool zoomextents all

				-- Hide all not geometry
				for i in objects do
				(
					if ((superClassOf i) != GeometryClass) or (matchPattern i.name pattern:"Bip01*") then
						hide i
					else
						unhide i
				)

				-- Output filename
				outputFileName = (getFilenamePath i) + (getFilenameFile i) + "_nel_shoot.tga"

				-- Render
				render outputwidth:(Width.value as integer) outputheight:(Height.value as integer) outputfile:outputFileName shadows:false autoReflect:false vfb:false
			)
		)

		-- Recursse ?
		if Recurse.checked == true then
		(
			-- For each others directory
			directories = getDirectories (path+"*")

			-- For each directories
			for i in directories do
			(
				-- Recall the function
				shoot_directory i
			)
		)
	)

	fn clear_shoot_directory path =
	(
		-- Make sure the path name is formatted
		if path.count > 0 then
		(
			-- Get last character
			lastChar = path[path.count]

			if ( lastChar != '\\' ) and ( lastChar != '/' ) then
			(
				path += "\\"
			)
		)

		-- List the file in this folder
		files = getFiles (path+"*.tga")

		-- For each filename
		for i in files do
		(
			-- Remove this shoot ?
			if ((findString (filenameFromPath i) DefaultPostFixe) != undefined) then
			(
				-- Erase the file
				deleteFile i
			)
		)

		-- Recursse ?
		if Recurse.checked == true then
		(
			-- For each others directory
			directories = getDirectories (path+"*")

			-- For each directories
			for i in directories do
			(
				-- Recall the function
				clear_shoot_directory i
			)
		)
	)

	on ShootTheDatabase pressed do
	(
		shoot_directory RootDataBase.text
	)

	on ClearShootTheDatabase pressed do
	(
		clear_shoot_directory RootDataBase.text
	)
)

	
if dbase_cleaner_floater != undefined do
(
	closerolloutfloater dbase_cleaner_floater
)	

dbase_cleaner_Floater = newRolloutFloater "NeL DB Shooter" 400 815 800 200
addrollout db_shooter_rollout dbase_cleaner_Floater

