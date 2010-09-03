

-- Find id
Fn findID node =
(
	local

	-- Const
	alphabet="ABCDEFGHIJKLMNOPQRSTUVWXYZ" 
	NameTab = filterString node.name "_"
	Z_ID = -1
	alpha_letter1 = NameTab[2][1]
	alpha_letter2 = NameTab[2][2]
	alpha_letter1_value = findstring alphabet alpha_letter1 
	alpha_letter2_value = findstring alphabet alpha_letter2 

	-- Decompose theh name in an array array[1]=numeric string value  array[2]=alpha string value
	-- The index of the engine start at 0 but the script one at 1 so we sub 1 each time
	alpha_sub_id = (((alpha_letter1_value as integer - 1) * 26 + (alpha_letter2_value as integer)))-1
	num_sub_id = (NameTab[1] as integer)-1

		-- Array of 256 per 256
		---------------------------
		--	0	1	2	3	...	255
		--	256	257	258	259	...	511 
		--	512	513	514	515	...	767
		--	...

	Z_ID = num_sub_id*256 + alpha_sub_id
	return Z_ID
)

fn runNelMaxExport inputMaxFile = 
(
	outputNelFile = ("%OutputDirectory%/" + (getFilenameFile inputMaxFile) + ".%PreGenFileExtension%")
	tagThisFile = false
	
	-- Unhide category
	unhidecategory()
	
	-- Select none
	max select none
	
	-- Found it ?
	find = false
	
	-- For each object in the priject
	for i in geometry do
	(
		-- Look for a NeL patch mesh
		if (classof i) == RklPatch then
		(
			-- Error catching
			try
			(
				if (ExportRykolZone i outputNelFile (findID i)) == false then
				(
					nlerror("ERROR exporting zone " + i.name + " in file " + inputMaxFile)
				)
				else
				(
					nlerror("OK " + outputNelFile)
					tagThisFile = true
					find = true
					exit
				)
			)
			catch
			(
				-- Error
				nlerror("ERROR fatal error exporting zone " + i.name + " in file " + inputMaxFile)
				tagThisFile = false
			)
		)
	)
	
	-- Not found ?
	if (find == false) then
	(
		-- Error
		nlerror("WARNING no zone found in project " + inputMaxFile)
		tagThisFile = true
	)
	
	return tagThisFile
)
