---------------------------------------------------------

-- Some utility functions

---------------------------------------------------------

-- SUMMARY:

-- lowercase
-- uppercase
-- fileExist
-- adjustPathStringForScript

---------------------------------------------------------

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

---------------------------------------------------------

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

---------------------------------------------------------

-- Adjust path string to use "/" and not finish with a /
-- "c:\temp" -> "c:/temp" 
-- "c:\temp\" -> "c:/temp" 
-- "c:/temp" -> "c:/temp" 
-- "c:/temp/" -> "c:/temp" 
fn adjustPathStringForScript pathString =
(
	-- Change '\' in '/'

	local arrayString, output
	arrayString = filterString pathString "\\"
	output = ""
	for fragment in arrayString do
	(
		output = output + fragment + "/"
	)

	-- Remove final "/"
	while (output.count) != 0 and (output[output.count] == "/") do
	(
		output = substring output 1 (output.count - 1)
	)

	-- Return result
	return output
)

---------------------------------------------------------

fn fileExist filename = 
(
	local file
	file = openFile filename mode:"r"
	if file == undefined then
	(
		return false
	)
	else
	(
		close file
		return true
	)
)

---------------------------------------------------------
