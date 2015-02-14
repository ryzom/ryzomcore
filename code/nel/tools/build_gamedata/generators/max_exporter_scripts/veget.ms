

-- Some globals

NEL3D_APPDATA_DONOTEXPORT = 1423062565		-- do not export me : "undefined" = export me
											--							  "0" = export me
											--							  "1" = DONT export me

NEL3D_APPDATA_VEGETABLE = 1423062580		-- "undefined" = not vegetable
											-- "0" = not vegetable
											-- "1" = vegetable


-- Must export this node ?
fn isToBeExported node =
(
	if (classof node == nel_ps) then
		return false

	doNotExport = getappdata node NEL3D_APPDATA_DONOTEXPORT
	if (doNotExport != undefined) then
	(
		if (doNotExport == "1") then
			return false
	)

	vegetable = getappdata node NEL3D_APPDATA_VEGETABLE
	if (vegetable != undefined) then
		if (vegetable == "1") then
			return true

	-- Do not export
	return false
)


fn runNelMaxExport inputMaxFile = 
(
	tagThisFile = true
	
	-- Unhide category
	unhidelayers()
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
			-- Is not a skeleton ?
			if (node.name != "Bip01") then
			(
				-- Can be exported ?
				if (isToBeExported node == true) then
				(
					-- Output directory
					outputNelFile = ("%OutputDirectory%/" + (node.name) + ".veget")
					
					-- Compare file date
					if (NeLTestFileDate outputNelFile inputMaxFile) == true then
					(
						try
						(
							-- Export the veget
							if (NelExportVegetable node outputNelFile false) == true then
							(
								nlerror("OK "+outputNelFile)
								exported = exported+1
							)
							else
							(
								-- Error
								nlerror("ERROR exporting veget " + node.name + " in file " + inputMaxFile)
								tagThisFile = false
							)
						)
						catch 
						(
							-- Error
							nlerror("ERROR fata error exporting veget " + node.name + " in file " + inputMaxFile)
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
	if exported == 0 then
	(
		-- Error
		nlerror("WARNING no veget exported from the file " + inputMaxFile)
	)
	
	return tagThisFile
)

