
-- Script configuration

DefaultPostFixe = "_nel_shoot.tga"

rollout db_shooter_rollout "Database Erase Mesh"
(
	Label RootDataBaseLabel "Root path: " align:#left
	EditText RootDataBase text:"c:\database" align:#left
	Label MeshNameLabel "Mesh name: " align:#left
	EditText MeshName text:"toto" align:#left
	Button EraseMesh "Erase (warning !!!)" width:110 align:#left

	on EraseMesh pressed do
	(
		-- Path name
		path = RootDataBase.text

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
				-- Array to delete
				arrayToDelete = #()

				-- For each geometry node
				for i in geometry do
				(
					if (i.name == MeshName.text) then
					(
						append arrayToDelete i
					)
				)

				-- Modified
				modified = false

				-- Erase the objects
				for i in arrayToDelete do
				(
					delete i
					modified = true
				)

				-- Modified ?
				if (modified == true) then
				(
					-- Save the file
					max file save  
				)
			)
		)
	)
)

	
if dbase_cleaner_floater != undefined do
(
	closerolloutfloater dbase_cleaner_floater
)	

dbase_cleaner_Floater = newRolloutFloater "NeL DB Erase Mesh" 400 815 800 200
addrollout db_shooter_rollout dbase_cleaner_Floater

