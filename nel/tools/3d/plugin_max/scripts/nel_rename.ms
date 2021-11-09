-- This script rename texture files in material

-- Include some NeL stuff

include "nel_utility.ms"

NelRenameSampleFilename = "c:/dir/name.ext"
NelRenameSample = "Bip01 toto"

NelRenamePropertiesMaterial = 1
NelRenamePropertiesNode = 2

NelRenameSelectionAlways = 1
NelRenameSelectionSearch = 2

NelRenameActionReplace = 1
NelRenameActionPrefix = 2
NelRenameActionPostfix = 3

NelRenameFilterDir = 1
NelRenameFilterDirName = 2
NelRenameFilterDirNameExt = 3
NelRenameFilterName = 4
NelRenameFilterNameExt = 5
NelRenameFilterExt = 6

rollout rename_rollout "Properties"
(
	group "Global settings"
	(
		RadioButtons	RenameWhat				"Properties to rename"			labels:#("Material", "Node") align:#left
		RadioButtons	SelectionMode			"String selection"				labels:#("Apply to all the string", "Search pattern in the string") align:#left
		RadioButtons	Action					"Action to perform"				labels:#("Replace", "Prefix", "Postfix") align:#left
		Label			RemplaceWithLabel		"Remplace / Prefix / Postfix with:"			align:#left
		EditText		RemplaceWith			""								width:500	align:#left
		RadioButtons	PathNameFilter			"Path name filter"				labels:#("c:/dir", "c:/dir/name", "c:/dir/name.ext", "name", "name.ext", ".ext") align:#left
		Label			Sample					""								align:#left
	)

	group "Replace"
	(
		Label			FindWhatLabel			"Find what:"								align:#left
		EditText		FindWhat				""								width:500	align:#left
		CheckBox		MatchWholeWordOnly		"Match whole word only"			checked:true
		CheckBox		MatchCase				"Match case"					checked:false
	)

	fn replaceString str pattern_to_search =
	(
		local result, position, match, temp_str
		
		-- Default, doesn't match
		match = false
		if SelectionMode.state == NelRenameSelectionSearch then
		(
			-- Match all the word ?
			if MatchWholeWordOnly.checked == true then
			(
				if str == pattern_to_search then
					match = true
			)
			else
			(
				-- Find pattern
				result = ""
				temp_str = str
				while (temp_str != "") do
				(
					-- Find the pattern in the temp_str ?
					position = findString temp_str pattern_to_search

					if position != undefined then
					(
						-- Contact the fist part
						result = result + (substring temp_str 1 (position-1)) + RemplaceWith.text
				
						-- Remove the pattern
						temp_str = substring temp_str (position+pattern_to_search.count) -1

						-- Match
						match = true
					)
					else
					(
						-- Final
						result = result + temp_str
						temp_str = ""
					)
				)

				-- If replace, return the result
				if Action.state == NelRenameActionReplace then
					return result
			)
		)
		else if SelectionMode.state == NelRenameSelectionAlways then
		(
			-- Always match
			match = true
		)

		-- Match ?
		if match == true then
		(
			if Action.state == NelRenameActionReplace then
				return RemplaceWith.text
			else if Action.state == NelRenameActionPrefix then
				return RemplaceWith.text + str
			else if Action.state == NelRenameActionPostfix then
				return str + RemplaceWith.text
		)
		else
		(
			return str
		)
	)

	fn isFilename =
	(
		if RenameWhat.state == NelRenamePropertiesMaterial then
			return true
		else
			return false
	)

	fn modifyString filename =
	(
		local dir, name, ext, find_what, prefix, name_to_modify, postfix

		-- Find what
		find_what = FindWhat.text

		-- Get the bitmap file
		if MatchCase.checked == false then
		(
			filename = lowercase filename
			find_what = lowercase find_what
		)

		-- Filename 
		prefix = ""
		name_to_modify = ""
		postfix = ""
		if isFilename() then
		(
			-- Extract different part of the path string
			dir = adjustPathStringForScript ((getFilenamePath filename)) + "/"
			name = getFilenameFile filename
			ext = getFilenameType filename

			-- Build the final filename
			if PathNameFilter.state == NelRenameFilterDir then
			(
				name_to_modify = dir
				postfix = name + ext
			)
			else if PathNameFilter.state == NelRenameFilterDirName then
			(
				name_to_modify = dir + name
				postfix = ext
			)
			else if PathNameFilter.state == NelRenameFilterDirNameExt then
			(
				name_to_modify = dir + name + ext
			)
			else if PathNameFilter.state == NelRenameFilterName then
			(
				prefix = dir
				name_to_modify = name
				postfix = ext
			)
			else if PathNameFilter.state == NelRenameFilterNameExt then
			(
				prefix = dir
				name_to_modify = name + ext
			)
			else -- if PathNameFilter.state == NelRenameFilterExt then
			(
				prefix = dir + name
				name_to_modify = ext
			)
		)
		else
		(
			name_to_modify = filename
		)

		-- Replace
		return (prefix + (replaceString name_to_modify find_what) + postfix)
	)

	fn updateSample =
	(
		if isFilename() then
			Sample.text = "Sample: \"" + NelRenameSampleFilename + "\" -> \"" + (modifyString NelRenameSample) + "\""
		else
			Sample.text = "Sample: \"" + NelRenameSample + "\" -> \"" + (modifyString NelRenameSample) + "\""
	)

	fn updateSearchMode =
	(
		if SelectionMode.state == NelRenameSelectionSearch then
		(
			MatchWholeWordOnly.enabled = true
			FindWhat.enabled = true
			MatchCase.enabled = true
		)
		else if SelectionMode.state == NelRenameSelectionAlways then
		(
			MatchWholeWordOnly.enabled = false
			FindWhat.enabled = false
			MatchCase.enabled = false
		)
	)

	fn updatePathNameFilter =
	(
		PathNameFilter.enabled = isFilename()
	)

	on FindWhat changed text do
	(
		updateSample ()
	)

	on RemplaceWith changed text do
	(
		updateSample ()
	)

	on SelectionMode changed state do
	(
		updateSearchMode ()
		updateSample ()
	)

	on Action changed state do
	(
		updateSample ()
	)

	on PathNameFilter changed state do
	(
		updateSample ()
	)

	on MatchWholeWordOnly changed state do
	(
		updateSample ()
	)

	on MatchCase changed state do
	(
		updateSample ()
	)

	on RenameWhat changed state do
	(
		updatePathNameFilter ()
		updateSample ()
	)

	local countRenamed

	fn renameMaterial material =
	(
		local find_what_lower, map, filename, position, mat

		-- Text to find in lower case
		find_what_lower = lowercase FindWhat.text

		-- Std material
		if ((classof material) == Standardmaterial) then
		(
			-- For each maps
			for map in material.maps do
			(
				-- Map defined ?
				if (map != undefined) then
				(
					-- Map is a bitmap file ?
					if (classof map) == Bitmaptexture then
					(
						-- Get the bitmap file
						filename = map.filename

						-- Modify it
						filename = modifyString filename

						-- Change something ?
						if (filename != map.filename) then
						(
							-- Remplace !
							map.filename = filename

							-- Count replaces
							countRenamed = countRenamed + 1
						)
					)
				)
			)
		)
		if ((classof material) == Multimaterial) then
		(
			for mat in material.materialList do
			(
				renameMaterial mat
			)
		)
	)

	fn do_it =
	(
		-- Reset count
		countRenamed = 0

		-- For each node in the project
		if (RenameWhat.state == NelRenamePropertiesMaterial) then
		(
			local mat
			for mat in sceneMaterials do
			(
				-- Rename its material
				renameMaterial mat
			)
		)
		else
		(
			local node
			for node in objects do
			(
				-- Rename its material
				previousname = node.name
				node.name = filename = modifyString node.name
				if (node.name != previousname) then
					countRenamed = countRenamed + 1
			)
		)

		-- Add to total sum
		return countRenamed
	)

	include "nel_batched_script.ms"	

	on rename_rollout open do
	(
		updateSearchMode ()
		updateSample ()
	)
)

--

gc ()

if rename_floater != undefined do
(
	closerolloutfloater rename_floater
)

rename_floater = newRolloutFloater "NeL rename bitmap" 550 874
addrollout rename_rollout rename_floater rolledUp:false
