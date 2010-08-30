

include "nel_utility.ms"

fn getAbsoluteSource t =
(
	return (mapPaths.getFullFilePath ((getFilenameFile (filenameFromPath t)) + ".png"))
)

fn getAbsoluteDestination t =
(
	if ((findString t "\\\\Amiga") != undefined) then
	(
		return ("W:\\" + (substring (getFilenamePath t) 11 -1) + "\\" + (getFilenameFile (filenameFromPath t)) + ".png")
	)
	else if ((findString t "\\\\amiga") != undefined) then
	(
		return ("W:\\" + (substring (getFilenamePath t) 11 -1) + "\\" + (getFilenameFile (filenameFromPath t)) + ".png")
	)
	else if ((findString t "W:\\") != undefined) then
	(
		return ((getFilenamePath t) + "\\" + (getFilenameFile (filenameFromPath t)) + ".png")
	)
	else if ((findString t "E:\\Ryzom\\tryker") != undefined) then
	(
		return "W:\\database\\Stuff\\Tryker\\Decors\\_textures\\Batiments\\" + (getFilenameFile (filenameFromPath t)) + ".png"
	)
	else if ((findString t "\\tronc.") != undefined) then
	(
		return "W:\\database\\database_proto\\stuff\\fyros\\objects\\tronc.png"
	)
	else if ((findString t "\\trame.") != undefined) then
	(
		return "W:\\database\\stuff\\lod_actors\\texture_lod\\trame.png"
	)
	else if ((findString t "\\PR_MO_phytopsy_tete01_Boss.") != undefined) then
	(
		return "W:\\database\\Stuff\\Tryker\\Agents\\_textures\\monster\\PR_MO_phytopsy_tete01_Boss.png"
	)
	else
	(
		return t
	)
)

fn getFixedTexturePath t =
(
	absoluteDestination = (getAbsoluteDestination t)
	if (not (doesFileExist absoluteDestination)) and (((findString t "\\Stuff\\Fyros\\Decors\\_textures\\Batiments") != undefined) or ((findString t "\\Stuff\\Fyros\\Decors\\_textures\\Accessories\\Vv2\\") != undefined)) then
	(
		testDestination = ("W:\\database\\stuff\\Fyros\\Agents\\_textures\\Accessories\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	if (not (doesFileExist absoluteDestination)) and (((findString t "\\Stuff\\Fyros\\Decors\\_textures\\Batiments") != undefined) or ((findString t "\\Stuff\\Fyros\\Decors\\_textures\\Accessories\\") != undefined)) then
	(
		testDestination = ("W:\\database\\stuff\\Fyros\\Agents\\_textures\\Accessories\\Vv2\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	if (not (doesFileExist absoluteDestination)) and ((findString t "\\Stuff\\Fyros\\Decors\\_textures\\Accessories") != undefined) then
	(
		testDestination = ("W:\\database\\stuff\\Fyros\\Agents\\_textures\\Batiments\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	if (not (doesFileExist absoluteDestination)) and ((findString t "\\Stuff\\Tryker\\Decors\\_textures\\batiments") != undefined) then
	(
		testDestination = ("W:\\database\\Stuff\\Tryker\\Decors\\_textures\\batiments\\noutilise\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	if (not (doesFileExist absoluteDestination)) and ((findString t "\\Stuff\\Tryker\\Decors\\_textures\\batiments") != undefined) then
	(
		testDestination = ("W:\\database\\Stuff\\Tryker\\Decors\\_textures\\batiments\\vv2\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	if (not (doesFileExist absoluteDestination)) and ((findString t "\\Stuff\\Tryker\\Decors\\_textures\\batiments") != undefined) then
	(
		testDestination = ("W:\\database\\Stuff\\Tryker\\Decors\\_textures\\batiments\\tempautel\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	if (not (doesFileExist absoluteDestination)) and ((findString t "\\Stuff\\Tryker\\Decors\\_textures\\batiments") != undefined) then
	(
		testDestination = ("W:\\database\\Stuff\\Tryker\\Decors\\_textures\\batiments\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	if (not (doesFileExist absoluteDestination)) and ((findString t "\\Stuff\\Fyros\\Decors\\_textures") != undefined) then
	(
		testDestination = ("W:\\database\\Stuff\\Fyros\\Decors\\_textures\\Batiments\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	if (not (doesFileExist absoluteDestination)) and ((findString t "\\Stuff\\Fyros\\Decors\\_textures") != undefined) then
	(
		testDestination = ("W:\\database\\Stuff\\Tryker\\Decors\\_textures\\Batiments\\" + (getFilenameFile (filenameFromPath t)) + ".png")
		if (doesFileExist testDestination) then
		(
			return testDestination
		)
	)
	return absoluteDestination
)

fn renameTexture t =
(
	try
	(
		if (t != undefined) then
		(
			if (classof t == NelBitmapTexture) then
			(
				if (t.bitmap1FileName != "") then (t.bitmap1FileName = getFixedTexturePath t.bitmap1FileName)
				if (t.bitmap2FileName != "") then (t.bitmap2FileName = getFixedTexturePath t.bitmap2FileName)
				if (t.bitmap3FileName != "") then (t.bitmap3FileName = getFixedTexturePath t.bitmap3FileName)
				if (t.bitmap4FileName != "") then (t.bitmap4FileName = getFixedTexturePath t.bitmap4FileName)
				if (t.bitmap5FileName != "") then (t.bitmap5FileName = getFixedTexturePath t.bitmap5FileName)
				if (t.bitmap6FileName != "") then (t.bitmap6FileName = getFixedTexturePath t.bitmap6FileName)
				if (t.bitmap7FileName != "") then (t.bitmap7FileName = getFixedTexturePath t.bitmap7FileName)
				if (t.bitmap8FileName != "") then (t.bitmap8FileName = getFixedTexturePath t.bitmap8FileName)
				renameTexture t.bitmap
				if (t.bitmap.fileName == undefined) then
				(
					if (doesFileExist (mapPaths.getFullFilePath t.bitmap1FileName)) then (t.bitmap.fileName = t.bitmap1FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap2FileName)) then (t.bitmap.fileName = t.bitmap2FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap3FileName)) then (t.bitmap.fileName = t.bitmap3FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap4FileName)) then (t.bitmap.fileName = t.bitmap4FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap5FileName)) then (t.bitmap.fileName = t.bitmap5FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap6FileName)) then (t.bitmap.fileName = t.bitmap6FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap7FileName)) then (t.bitmap.fileName = t.bitmap7FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap8FileName)) then (t.bitmap.fileName = t.bitmap8FileName)
				)
				else if not (doesFileExist (mapPaths.getFullFilePath t.bitmap.fileName)) then
				(
					if (doesFileExist (mapPaths.getFullFilePath t.bitmap1FileName)) then (t.bitmap.fileName = t.bitmap1FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap2FileName)) then (t.bitmap.fileName = t.bitmap2FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap3FileName)) then (t.bitmap.fileName = t.bitmap3FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap4FileName)) then (t.bitmap.fileName = t.bitmap4FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap5FileName)) then (t.bitmap.fileName = t.bitmap5FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap6FileName)) then (t.bitmap.fileName = t.bitmap6FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap7FileName)) then (t.bitmap.fileName = t.bitmap7FileName)
					else if (doesFileExist (mapPaths.getFullFilePath t.bitmap8FileName)) then (t.bitmap.fileName = t.bitmap8FileName)
				)
				t.delegate.RGBOutput = 0
				t.delegate.monoOutput = 1
				t.delegate.alphasource = 2
			)
			else if (classof t == Reflect_Refract) then
			(
				if (t.bitmapName[1] != undefined) then (t.bitmapName[1] = getFixedTexturePath t.bitmapName[1])
				if (t.bitmapName[2] != undefined) then (t.bitmapName[2] = getFixedTexturePath t.bitmapName[2])
				if (t.bitmapName[3] != undefined) then (t.bitmapName[3] = getFixedTexturePath t.bitmapName[3])
				if (t.bitmapName[4] != undefined) then (t.bitmapName[4] = getFixedTexturePath t.bitmapName[4])
				if (t.bitmapName[5] != undefined) then (t.bitmapName[5] = getFixedTexturePath t.bitmapName[5])
				if (t.bitmapName[6] != undefined) then (t.bitmapName[6] = getFixedTexturePath t.bitmapName[6])
				if (t.outputname != undefined) then (t.outputname = getFixedTexturePath t.outputname)
			)
			else
			(
				if (t.fileName != undefined) then (t.fileName = getFixedTexturePath t.fileName)
				if (classof t == BitmapTexture) then
				(
					t.RGBOutput = 0
					t.monoOutput = 1
					t.alphasource = 2
				)
			)
		)
	)
	catch
	(
	
	)
)


rollout assets_png_rollout "Properties"
(
	fn do_it =
	(
		for m in getClassInstances BitmapTexture do
		(
			renameTexture m
		)
		
		for m in getClassInstances NelBitmapTexture do
		(
			renameTexture m
		)
		
		for m in getClassInstances NelMaterial do
		(
			renameTexture m.tTexture_1
			renameTexture m.tTexture_2
			renameTexture m.tTexture_3
			renameTexture m.tTexture_4
			renameTexture m.tTexture_5
			renameTexture m.tTexture_6
			renameTexture m.tTexture_7
			renameTexture m.tTexture_8
			m.delegate.DiffuseMapEnable = m.bEnableSlot_1
			m.delegate.DiffuseMap = m.tTexture_1
			m.delegate.AmbientMapEnable = m.bEnableSlot_1
			m.delegate.AmbientMap = m.tTexture_1
			m.delegate.SpecularMapEnable = m.bEnableSlot_2
			m.delegate.SpecularMap = m.tTexture_2
			m.delegate.SelfIllumMap = undefined
			m.delegate.OpacityMap = undefined
			m.delegate.FilterMap = undefined
			m.delegate.BumpMap = undefined
			m.delegate.DisplacementMap = undefined
			m.delegate.ReflectionMap = undefined
			m.delegate.RefractionMap = undefined
		)
		
		actionMan.executeAction 0 "40021"  -- Selection: Select All
		actionMan.executeAction 0 "311"  -- Tools: Zoom Extents All Selected
		actionMan.executeAction 0 "63508"  -- Views: Standard Display with Maps
		actionMan.executeAction 0 "40043"  -- Selection: Select None
		
		max views redraw
		
		return 1
	)
	
	include "nel_batched_mergesave.ms"
)

assets_png_floater = newRolloutFloater "NeL Assets PNG Database" 550 874
addrollout assets_png_rollout assets_png_floater rolledUp:false

