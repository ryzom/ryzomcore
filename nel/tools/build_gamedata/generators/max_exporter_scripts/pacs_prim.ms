

fn runNelMaxExport inputMaxFile = 
(
	outputNelFile = ("%OutputDirectory%/" + (getFilenameFile inputMaxFile) + ".%PreGenFileExtension%")
	tagThisFile = false
	
	-- Unhide category
	unhidecategory()
	
	-- Select none
	max select none
	clearSelection()
	
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
		if (NelExportPACSPrimitives arrayNode outputNelFile) == false then
		(
			nlerror("ERROR exporting PACS primitives in file " + inputMaxFile)
			tagThisFile = false
		)
		else
		(
			nlerror("OK PACS primitives in file " + inputMaxFile)
			tagThisFile = true
		)
	)
	else
	(
		nlerror("WARNING no PACS primitives in file " + inputMaxFile)
		tagThisFile = true
	)
	
	return tagThisFile
)

