

-- Some globals

NEL3D_APPDATA_ACCEL = 1423062561			-- type of accelerator :	"32" = is not an accelerator and IS clusterized 
											--							 "0" = is not an accelerator and IS NOT clusterized (always visible)
											--							 "1" = is an accelerator type PORTAL
											--							 "2" = is an accelerator type CLUSTER
											--							 "6" = is an accelerator type CLUSTER FATHER-VISIBLE
											--							"10" = is an accelerator type CLUSTER VISIBLE-FROM-FATHER
											--							"14" = is an accelerator type CLUSTER FATHER-VISIBLE and VISIBLE-FROM-FATHER
											--							"17" = is an accelerator type PORTAL DYNAMIC
																			 
NEL3D_APPDATA_DONOTEXPORT = 1423062565		-- do not export me : "undefined" = export me
											--							  "0" = export me
											--							  "1" = DONT export me

NEL3D_APPDATA_IGNAME = 1423062564			-- string : name of the Instance Group

NEL3D_APPDATA_LOD_NAME_COUNT_MAX	= 10
NEL3D_APPDATA_LOD					= 1423062537
NEL3D_APPDATA_LOD_NAME_COUNT		= NEL3D_APPDATA_LOD
NEL3D_APPDATA_LOD_NAME				= NEL3D_APPDATA_LOD_NAME_COUNT+1
NEL3D_APPDATA_LOD_BLEND_IN			= NEL3D_APPDATA_LOD_NAME+NEL3D_APPDATA_LOD_NAME_COUNT_MAX
NEL3D_APPDATA_LOD_BLEND_OUT			= NEL3D_APPDATA_LOD_BLEND_IN+1
NEL3D_APPDATA_LOD_COARSE_MESH		= NEL3D_APPDATA_LOD_BLEND_OUT+1

NEL_OBJECT_NAME_DATA				= 1970


-- This node is n accelerator ?
fn isAccelerator node =
(
	accel = getappdata node NEL3D_APPDATA_ACCEL
	if (accel != undefined) then
	(
		if (accel == "0") or (accel == "32") then
			return false
		else
			return true
	)
	return false
)

-- Get the ig name of this object
fn getIg node =
(
	return (getappdata node NEL3D_APPDATA_IGNAME)
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
	
	-- Ig array
	ig_array = #()
	
	-- Scan all the ig in this project
	for node in objects do
	(
		ig = getIg node
		if ( (ig != undefined) and (ig != "") ) then
		(
			-- Found ?
			found = false
			
			-- Already found ?
			for j = 1 to ig_array.count do
			(
				if (ig_array[j]==ig) then
				(
					found = true
					exit
				)
			)
			
			-- Found ?
			if (found == false) then
			(
				append ig_array ig
			)
		)
	)
	
	-- Have some ig ?
	if (ig_array.count != 0) then
	(
		-- For each ig
		for ig = 1 to ig_array.count do
		(
			-- Output filename
			outputNelFile = ("%OutputDirectory%/" + ig_array[ig] + ".ig")
			
			-- Check date
			if (NeLTestFileDate outputNelFile inputMaxFile) == true then
			(
				-- Select none
				max select none
				clearSelection()
				
				-- Select all node in this ig
				for node in geometry do
				(
					-- Select it if in the ig
					if ((getIg node) == ig_array[ig]) then
						selectmore node
				)
				-- Select all lights in this ig
				for node in lights do
				(
					-- Select it if in the ig
					if ((getIg node) == ig_array[ig]) then
						selectmore node
				)
				-- Select all lights in this ig
				for node in helpers do
				(
					-- Select it if in the ig
					if ((getIg node) == ig_array[ig]) then
						selectmore node
				)
				
				-- Check export
				try
				(
					-- Export the ig
					instancegroup2export = $selection as array
					if (NelExportInstanceGroup instancegroup2export outputNelFile) == true then
					(
						nlerror("OK " + outputNelFile)
					)
					else
					(
						-- Error
						nlerror("ERROR exporting ig " + ig_array[ig] + " in file " + inputMaxFile)
						tagThisFile = false
					)
				)
				catch
				(
					-- Error
					nlerror("ERROR fatal error exporting ig " + ig_array[ig] + " in file " + inputMaxFile)
					tagThisFile = false
				)
			)
			else
			(
				nlerror("SKIPPED " + outputNelFile)
			)
		)
	)
	else
	(
		-- Error
		nlerror("WARNING nothing exported from ig max file " + inputMaxFile)
	)
	
	return tagThisFile
)

