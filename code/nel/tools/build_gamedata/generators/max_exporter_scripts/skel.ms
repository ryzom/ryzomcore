

fn runNelMaxExport inputMaxFile = 
(
	outputNelFile = ("%OutputDirectory%/" + (getFilenameFile inputMaxFile) + ".%PreGenFileExtension%")
	tagThisFile = false
	
	-- Unhide category
	unhidecategory()
	
	-- Select Bip01, not very smart
	if $Bip01 != undefined then
	(
		-- Select Bip01
		select $Bip01
		
		if ($ != undefined) then
		(
			-- Set figure mode on
			if ((classof $) == Biped_Object) then
			(
				$.controller.figureMode = true
			)
			
			-- Export the skeleton template
			if (NelExportSkeleton $ outputNelFile) == false then
			(
				nlerror("ERROR exporting skeleton " + inputMaxFile)
				tagThisFile = false
			)
			else
			(
				nlerror("OK " + outputNelFile)
				tagThisFile = true
			)
		)
		else
		(
			-- Error
			nlerror("ERROR exporting skeleton: no Bip01 node in file " + inputMaxFile)
			tagThisFile = false
		)
	)
	else
	(
		-- Error
		nlerror("ERROR exporting skeleton: no Bip01 node in file " + inputMaxFile)
		tagThisFile = false
	)
	
	return tagThisFile
)

