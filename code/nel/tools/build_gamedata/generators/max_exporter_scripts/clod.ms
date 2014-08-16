

-- Some globals

NEL3D_APPDATA_DONOTEXPORT = 1423062565		-- do not export me : "undefined" = export me
											--							  "0" = export me
											--							  "1" = DONT export me
NEL3D_APPDATA_CHARACTER_LOD = 1423062618	-- "1": I am a character lod if "1". "0" or undefined: I am not.


-- Must export this node ?
fn isToBeExported node = 
(
	if ((classof node) == RklPatch) then
		return false
	
	if ((classof node) == nel_ps) then
		return false
	
	if ((classof node) == nel_pacs_cylinder) then
		return false
	
	if ((classof node) == nel_pacs_box) then
		return false
	
	doNotExport = getappdata node NEL3D_APPDATA_DONOTEXPORT
	if (doNotExport != undefined) then
	(
		if (doNotExport == "1") then
			return false
	)
	return true
)


-- is this node flagged as a LodCharacter ??
fn isLodCharacter node = 
(
	isCLod = getappdata node NEL3D_APPDATA_CHARACTER_LOD
	if (isCLod == undefined) then
		return false
	if (isCLod == "1") then
		return true
	return false
)

fn runNelMaxExport inputMaxFile = 
(
	tagThisFile = true
	
	-- Unhide category
	unhidecategory()
	
	-- Unhide
	max unhide all
	
	-- unselect
	max select none
	clearSelection()
	
	-- Exported object count
	exported = 0
	
	-- For each node
	for node in geometry do
	(
		-- It is root ?
		if (node.parent == undefined) then
		(
			-- Can be exported ?
			if (isToBeExported node == true) then
			(
				-- Is a Lod character?
				if ((isLodCharacter node) == true) then
				(
					-- Output directory
					outputNelFile = ("%OutputDirectory%/" + (node.name) + ".clod")
					
					-- Compare file date
					if (NeLTestFileDate outputNelFile inputMaxFile) == true then
					(
						try
						(
							-- Export the shape
							if (NelExportLodCharacter node outputNelFile false) == true then
							(
								nlerror("OK " + outputNelFile)
								exported = exported+1
							)
							else
							(
								-- Error
								nlerror("ERROR exporting .clod " + node.name + " in file " + inputMaxFile)
								tagThisFile = false
							)
						)
						catch 
						(
							-- Error
							nlerror("ERROR fatal error exporting .clod " + node.name + " in file " + inputMaxFile)
							tagThisFile = false
						)
					)
					else
					(
						-- Error
						nlerror("SKIPPED " + outputNelFile)
						exported = exported + 1
					)
				)
			)
		)
	)
	
	-- Something exported
	if (exported == 0) then
	(
		-- Error
		nlerror ("WARNING no .clod exported from the file " + inputMaxFile)
	)
	
	return tagThisFile
)

