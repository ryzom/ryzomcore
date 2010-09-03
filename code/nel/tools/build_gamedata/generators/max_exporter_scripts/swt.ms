

fn runNelMaxExport inputMaxFile = 
(
	outputNelFile = ("%OutputDirectory%/" + (getFilenameFile inputMaxFile) + ".%PreGenFileExtension%")
	tagThisFile = false
	
	-- Unhide category
	unhidecategory()
	
	-- Select all the nodes
	max select all
	
	-- Export the skeleton template
	if NelExportSkeletonWeight ($selection as array) outputNelFile == false then
	(
		nlerror("ERROR exporting skeleton weight " + inputMaxFile)
		tagThisFile = false
	)
	else
	(
		nlerror("OK " + outputNelFile)
		tagThisFile = true
	)
	
	return tagThisFile
)

