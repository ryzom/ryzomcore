

NEL3D_APPDATA_EXPORT_NODE_ANIMATION = 1423062800

fn runNelMaxExport inputMaxFile = 
(
	outputNelFile = ("%OutputDirectory%/" + (getFilenameFile inputMaxFile) + ".%PreGenFileExtension%")
	tagThisFile = true
	
	-- Unhide category
	unhidecategory()
	
	-- Select Bip01, not very smart
	if $Bip01 != undefined then
	(
		select $Bip01
		
		-- Always uncheck triangle pelvis
		if (classof $Bip01) == Biped_Object then
		(
			$Bip01.controller.figureMode = true
			$Bip01.controller.trianglepelvis = false
			$Bip01.controller.figureMode = false
		)
	)
	
	-- For each node
	for node in objects do
	(
		exportNodeAnmation = getappdata node NEL3D_APPDATA_EXPORT_NODE_ANIMATION
		if (exportNodeAnmation != undefined) then
		(
			if (exportNodeAnmation == "1") then
			(
				selectmore node
				
				-- Is it a biped ?
				if (classof node.controller) == Vertical_Horizontal_Turn then
				(
					-- Always uncheck triangle pelvis
					node.controller.trianglepelvis = false
				)
			)
		)
	)
	
	if ((selection as array).count != 0) then
	(
		-- Export the animation
		if (NelExportAnimation (selection as array) outputNelFile false) == false then
		(
			nlerror("ERROR exporting animation " + inputMaxFile)
			tagThisFile = false
		)
		else
		(
			nlerror("OK " + outputNelFile)
		)
	)
	else
	(
		-- Error
		nlerror("WARNING exporting animation: no node animated to export in file " + inputMaxFile)
	)
	
	return tagThisFile
)

