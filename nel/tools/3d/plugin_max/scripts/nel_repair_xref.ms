-- This script is use to correct a 3dsmax bug in xref. When the material script is modified, the max projects
-- with xref objects on other max projects that use the matrial script with an older version lead to an error.
--
-- This script open all the .max files in the directory list all the max project used in XREF objects.
-- Then it opens each of those projects (with the merge option, not the load option) and saves the project.

include "nel_utility.ms"

fn fullreset =
(
	-- Free memory and file handles
	gc ()
	heapfree

	-- Reset 3dsmax
	resetMAXFile #noprompt
)

global xrefobj

fn addString str =
(
	lwrstr = lowercase str
	addit = true
	for i in xrefobj do
	(
		if (lwrstr == i) then
		(
			addit = false
			exit
		)
	)
	if addit == true then
		append xrefobj lwrstr
)

fn appendList l0 l1 =
(
	local out = #()
	for i in l0 do
	(
		append out i
	)
	for i in l1 do
	(
		append out i
	)
	return out
)

fn getDirList baseDir =
(
	local numChar = baseDir.count
	local newDir = ""
	if numChar != 0 then
	(
		if baseDir[numChar] == "\\" then
		(
		    baseDir = substring baseDir 1 (numChar - 1)
		)
	)
	retPathList = #(baseDir)
	subDirectories = getDirectories(baseDir+"\\*")
    for currDir in subDirectories do
	(		
		subDirectoriesTree = getDirList(currDir)
		retPathList = appendList retPathList subDirectoriesTree		
	)
	return retPathList
)

fn splitPathList str =
(
  if str.count == 0 then return #()
  local out = #()
  local curr = ""
  for index = 1 to str.count do
  (
	if str[index] == ";" then
	(
		append out curr
		curr = ""
	)
	else
	(
		curr = curr + str[index]
	)
  )
  if curr.count != 0 then
  (
  	append out curr
  )
  return out
)

fn getAllPathToFix pathListAsString recurse =
(
	local out = #()
	local currSubPaths
	local basePaths = splitPathList pathListAsString
	if recurse then
	(
		for basePath in basePaths do
		(
			currSubPaths = getDirList basePath
			out = appendList out currSubPaths
		)	
	)
	else
	(
		out = basePaths
	)
	return out
)

rollout repair_xref_rollout "Properties"
(
	label			PathLabel				"Path for max projects using XREF (separated by ;):"	align:#left
	edittext		Path														align:#left
   checkbox    RecurseSubPaths      "Recurse sub paths"
	button			List					"Get XRefed files"					align:#left across:2
	button			GetPathList				"Get path list"					    align:#left across:2
	button			Repair					"Repair all"						enabled:false align:#left
	listbox			XRefProjects			height:20							align:#left
	listbox			PathListBox				height:14							align:#left
	listbox			Errors					height:14							align:#left

	fn message str =
	(
		tmp = Errors.items
		append tmp str
		Errors.items = tmp
		Errors.selection = 0
	)

	on GetPathList pressed do
	(		
		PathListBox.items = getAllPathToFix Path.text RecurseSubPaths.checked
	)

	on List pressed do
	(
		xrefobj = #()
		Errors.items = #()
		local pathList = getAllPathToFix Path.text RecurseSubPaths.checked
		for currPath in pathList do
        (
		    files = getFiles (currPath+"\\*.max")
  		    for i in files do
    		(
			    -- Reset the max project
			    fullreset ()

			    -- Merge the project
			    message ("Merging "+i)
			    if (mergeMAXFile i) == true then
			    (
				    -- Unhide all
				    max unhide all

				    -- For each xref
				    xrefFound = false
					for node in objects do
					(
						if (classof node) == XRefObject then
						(
							addString node.filename
							xrefFound = true
						)
					)

					-- XRef found
					if xrefFound == true then
					(
						addString i;
					)
				)
				else
				(
					message ("Error merging "+i)
				)
			)
		)
		XRefProjects.items  = xrefobj
		Repair.enabled = true
	)

	on XRefProjects doubleClicked id do
	(
		if (id != 0) then
		(
			-- Reset the max project
			fullreset ()

			-- Load the project
			loadMAXFile XRefProjects.items[id]
		)
	)

	on Repair pressed do
	(
		for i in xrefobj do
		(
			-- Reset the max project
			fullreset ()

			-- Merge the project
			message ("Merging "+i)
			oldDate = NeLGetFileModificationDate i
			if (mergeMAXFile i) == true then
			(
				-- Save it
				message ("Saving "+i)
				if (saveMaxFile i) == false then
					message ("Error saving "+i)
				NeLSetFileModificationDate i oldDate
			)
			else
			(
				message ("Error opening "+i)
			)
		)
	)
)

--

gc ()

if repair_xref_floater != undefined do
(
	closerolloutfloater repair_xref_floater
)

repair_xref_floater = newRolloutFloater "NeL repair XREF" 600 600
addrollout repair_xref_rollout repair_xref_floater rolledUp:false
