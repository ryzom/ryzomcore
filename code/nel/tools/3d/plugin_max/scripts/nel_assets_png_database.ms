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
	else
	(
		return t
	)
)

fn getFixedTexturePath t =
(
	if (doesFileExist (getAbsoluteSource t)) then
	(
		if not (doesFileExist (getAbsoluteDestination t)) then
		(
			makeDir (getFilenamePath (getAbsoluteDestination t)) all:true
			renameFile (getAbsoluteSource t) (getAbsoluteDestination t)
		)
		if (doesFileExist (getAbsoluteDestination t)) then
		(
			if (getAbsoluteDestination t) != (getAbsoluteSource t) then
			(
				deleteFile (getAbsoluteSource t)
			)
		)
	)
	return (getAbsoluteDestination t)
)

fn renameTexture t =
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
