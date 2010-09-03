

NEL3D_APPDATA_COLLISION					= 1423062613
NEL3D_APPDATA_COLLISION_EXTERIOR		= 1423062614


-- Must export this node ?
fn isToBeExported node =
(
	doNotExport = getappdata node NEL3D_APPDATA_COLLISION
	if (doNotExport != undefined) then
	(
		if (doNotExport == "1") then
			return true
	)

	doNotExport = getappdata node NEL3D_APPDATA_COLLISION_EXTERIOR
	if (doNotExport != undefined) then
	(
		if (doNotExport == "1") then
			return true
	)

	return false
)

fn runNelMaxExport inputMaxFile = 
(
	outputNelDir = "%OutputDirectory%"
	
	-- Tag this file ?
	tagThisFile = true
	
	-- Unhide category
	unhidecategory()
	
	-- Select all collision mesh
	max select none
	for m in geometry do
	(
		if (isToBeExported m) == true then
			selectmore m
	)
	
	-- Export the collision
	if (NelExportCollision ($selection as array) outputNelDir) == false then
	(
		nlerror("ERROR exporting collision " + inputMaxFile)
		tagThisFile = false
	)
	else
	(
		nlerror("OK collision in folder " + outputNelDir)
	)
	
	return tagThisFile
)


