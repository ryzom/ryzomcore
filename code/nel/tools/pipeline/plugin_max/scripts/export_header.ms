

-- #################################################################
-- ## %PreGenWarning%
-- #################################################################


-- Allocate 20 Me for the script
heapSize += 15000000

-- In case of error just abort the app and don't show nel report window
NelForceQuitOnMsgDisplayer()

-- Global definitions
NEL3D_APPDATA_LOD_NAME_COUNT_MAX	= 10
NEL3D_APPDATA_LOD					= 1423062537
NEL3D_APPDATA_LOD_NAME_COUNT		= NEL3D_APPDATA_LOD
NEL3D_APPDATA_LOD_NAME				= NEL3D_APPDATA_LOD_NAME_COUNT+1
NEL3D_APPDATA_LOD_BLEND_IN			= NEL3D_APPDATA_LOD_NAME+NEL3D_APPDATA_LOD_NAME_COUNT_MAX
NEL3D_APPDATA_LOD_BLEND_OUT			= NEL3D_APPDATA_LOD_BLEND_IN+1
NEL3D_APPDATA_LOD_COARSE_MESH		= NEL3D_APPDATA_LOD_BLEND_OUT+1

fn lowercase instring =
(  
	local upper, lower, outstring
	upper="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	lower="abcdefghijklmnopqrstuvwxyz" 

	outstring = copy instring 

	for iii = 1 to outstring.count do 
	(  
		jjj = findString upper outstring[iii]
		if (jjj != undefined) then
			outstring[iii] = lower[jjj] 
		else
			outstring[iii] = instring[iii]
	)
	return outstring     -- value of outstring will be returned as function result 
)

fn uppercase instring =
(  
	local upper, lower, outstring
	upper="ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	lower="abcdefghijklmnopqrstuvwxyz" 

	outstring = copy instring 

	for iii = 1 to outstring.count do 
	(  
		jjj = findString upper outstring[iii]
		if (jjj != undefined) then
			outstring[iii] = upper[jjj] 
		else
			outstring[iii] = instring[iii]
	)
	return outstring     -- value of outstring will be returned as function result 
)

-- Add xrefs as dependencies
fn dependxref root =
(
	sceneName = maxFilePath + maxFileName
	
	for i = 1 to xrefs.getXRefFileCount(root) do
	(
		curxref = xrefs.getXRefFile i root
		NelToolLoggerWriteDepend "BUILD" "*" curxref.filename -- not sure if full filepath...
		dependxref curxref
	)
)

-- Lowercase node names etc, temporary until another full conversion run over the max files is done
fn lowercasenodes = 
(
	allNodes = $*
	
	for node in allNodes do
	(
		-- Lowercase the node name
		node.name = lowercase(node.name)
		
		-- Lowercase all LOD node name references
		lodCount = getAppData node NEL3D_APPDATA_LOD_NAME_COUNT
		if (lodCount != undefined) then
		(
			lodCountNum = nodeCount as Integer
			for lod = 1 to lodCountNum do
			(
				lod = getAppData node (NEL3D_APPDATA_LOD_NAME + lod - 1)
				if (lod != undefined) then
				(
					setAppData node (NEL3D_APPDATA_LOD_NAME + lod - 1) lowercase(lod)
				)
			)
		)
	)
)
	
-- Unhide category
fn unhidecategory =
(
	if (geometry.count > 0) then
	(
		unhide geometry[1]
		if (geometry[1].ishidden == true) then
			max hide object toggle
	)
	if (shapes.count > 0) then
	(
		unhide shapes[1]
		if (shapes[1].ishidden == true) then
			max hide shape toggle
	)
	if (lights.count > 0) then
	(
		unhide lights[1]
		if (lights[1].ishidden == true) then
			max hide light toggle
	)
	if (cameras.count > 0) then
	(
		unhide cameras[1]
		if (cameras[1].ishidden == true) then
			max hide camera toggle
	)
	if (helpers.count > 0) then
	(
		unhide helpers[1]
		if (helpers[1].ishidden == true) then
			max hide helper toggle
	)
)
