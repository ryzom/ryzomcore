
----------------------------------------------------------------------------------------------------------------
-- Nel BitmapTexture properties : this provide additionnal slots to create textures sets
plugin textureMap NelBitmapTexture
	name:"Nel Multi Bitmap"
	classID:#(0x5a8003f9, 0x43e0955)
	extends:BitmapTexture
	replaceUI:false
	version:1.0
(
	local currSelectedTextureSet
	parameters pblock rollout:AdditionnalBitmaps
	(
			bitmap1FileName type:#String default:"" ui:cBitmap1FileName
			bitmap2FileName type:#String default:"" ui:cBitmap2FileName
			bitmap3FileName type:#String default:"" ui:cBitmap3FileName
			bitmap4FileName type:#String default:"" ui:cBitmap4FileName
			bitmap5FileName type:#String default:"" ui:cBitmap5FileName
			bitmap6FileName type:#String default:"" ui:cBitmap6FileName
			bitmap7FileName type:#String default:"" ui:cBitmap7FileName
			bitmap8FileName type:#String default:"" ui:cBitmap8FileName
	)
    rollout AdditionnalBitmaps "Additionnal bitmaps" rolledUp:false
    (
		edittext 	  cBitmap1FileName "ID 1:" width:230 across:3
		button 		  browse1 "Get" align:#left across:2 offset:[100, 0]
		button        view1   "View" offset:[40, 0]
		
		edittext 	  cBitmap2FileName "ID 2:" width:230 across:3
		button 		  browse2 "Get" align:#left across:2 offset:[100, 0]
		button        view2   "View" offset:[40, 0]
		
		edittext 	  cBitmap3FileName "ID 3:" width:230 across:3
		button 		  browse3 "Get" align:#left across:2 offset:[100, 0]
		button        view3   "View" offset:[40, 0]
		
		edittext 	  cBitmap4FileName "ID 4:" width:230 across:3
		button 		  browse4 "Get" align:#left across:2 offset:[100, 0]
		button        view4   "View" offset:[40, 0]
		
		edittext 	  cBitmap5FileName "ID 5:" width:230 across:3
		button 		  browse5 "Get" align:#left across:2 offset:[100, 0]
		button        view5   "View" offset:[40, 0]
		
		edittext 	  cBitmap6FileName "ID 6:" width:230 across:3
		button 		  browse6 "Get" align:#left across:2 offset:[100, 0]
		button        view6   "View" offset:[40, 0]
		
		edittext 	  cBitmap7FileName "ID 7:" width:230 across:3
		button 		  browse7 "Get" align:#left across:2 offset:[100, 0]
		button        view7   "View" offset:[40, 0]
		
		edittext 	  cBitmap8FileName "ID 8:" width:230 across:3
		button 		  browse8 "Get" align:#left across:2 offset:[100, 0]
		button        view8   "View" offset:[40, 0]



						
		on browse1 pressed do
		(
			myNewBitmap = selectBitmap()
			if myNewBitmap != undefined then
			(
				bitmap1FileName = myNewBitmap.fileName
				if currSelectedTextureSet == undefined or currSelectedTextureSet == 1 then
				(
					try
					(
						delegate.bitmap = openBitmap bitmap1FileName
					)
					catch
					(
						-- well, no luck..
					)
				)
			)			
		)
		
		on view1 pressed do
		(	
			try
			(	
				local myBitmap = openBitMap cBitmap1FileName.text
				display myBitmap
			)
			catch
			(
				messagebox "Unable to find / display bitmap"
			)
		)
		
		on browse2 pressed do
		(
			myNewBitmap = selectBitmap()
			if myNewBitmap != undefined then
			(
				bitmap2FileName = myNewBitmap.fileName
				if currSelectedTextureSet == 2
				then
				(
					try
					(
						delegate.bitmap = openBitmap bitmap2FileName
					)
					catch
					(
						-- well, no luck..
					)
				)
			)			
		)
		
		on view2 pressed do
		(	
			try
			(	
				local myBitmap = openBitMap cBitmap2FileName.text
				display myBitmap
			)
			catch
			(
				messagebox "Unable to find / display bitmap"
			)
		)
		
		on browse3 pressed do
		(
			myNewBitmap = selectBitmap()
			if myNewBitmap != undefined then
			(
				bitmap3FileName = myNewBitmap.fileName				
				if currSelectedTextureSet == 3
				then 
				(
					try
					(
						delegate.bitmap = openBitmap bitmap3FileName
					)
					catch
					(
						-- well, no luck..
					)
				)
			)			
		)
		
		on view3 pressed do
		(	
			try
			(	
				local myBitmap = openBitMap cBitmap3FileName.text
				display myBitmap
			)
			catch
			(
				messagebox "Unable to find / display bitmap"
			)
		)
		
		on browse4 pressed do
		(
			myNewBitmap = selectBitmap()
			if myNewBitmap != undefined then
			(
				bitmap4FileName = myNewBitmap.fileName
				if currSelectedTextureSet == 4
				then 
				(
					try
					(
						delegate.bitmap = openBitmap bitmap4FileName
					)
					catch
					(
						-- well, no luck..
					)
				)


			)			
		)
		
		on view4 pressed do
		(	
			try
			(	
				local myBitmap = openBitMap cBitmap4FileName.text
				display myBitmap
			)
			catch
			(
				messagebox "Unable to find / display bitmap"
			)
		)
		
		
		on browse5 pressed do
		(
			myNewBitmap = selectBitmap()
			if myNewBitmap != undefined then
			(
				bitmap5FileName = myNewBitmap.fileName
				if currSelectedTextureSet == undefined or currSelectedTextureSet == 5 then
				(
					try
					(
						delegate.bitmap = openBitmap bitmap5FileName
					)
					catch
					(
						-- well, no luck..
					)
				)
			)			
		)
		
		on view5 pressed do
		(	
			try
			(	
				local myBitmap = openBitMap cBitmap5FileName.text
				display myBitmap
			)
			catch
			(
				messagebox "Unable to find / display bitmap"
			)
		)
		
		on browse6 pressed do
		(
			myNewBitmap = selectBitmap()
			if myNewBitmap != undefined then
			(
				bitmap6FileName = myNewBitmap.fileName
				if currSelectedTextureSet == 6
				then
				(
					try
					(
						delegate.bitmap = openBitmap bitmap6FileName
					)
					catch
					(
						-- well, no luck..
					)
				)
			)			
		)
		
		on view6 pressed do
		(	
			try
			(	
				local myBitmap = openBitMap cBitmap6FileName.text
				display myBitmap
			)
			catch
			(
				messagebox "Unable to find / display bitmap"
			)
		)
		
		on browse7 pressed do
		(
			myNewBitmap = selectBitmap()
			if myNewBitmap != undefined then
			(
				bitmap7FileName = myNewBitmap.fileName				
				if currSelectedTextureSet == 7
				then 
				(
					try
					(
						delegate.bitmap = openBitmap bitmap7FileName
					)
					catch
					(
						-- well, no luck..
					)
				)
			)			
		)
		
		on view7 pressed do
		(	
			try
			(	
				local myBitmap = openBitMap cBitmap7FileName.text
				display myBitmap
			)
			catch
			(
				messagebox "Unable to find / display bitmap"
			)
		)
		
		on browse8 pressed do
		(
			myNewBitmap = selectBitmap()
			if myNewBitmap != undefined then
			(
				bitmap8FileName = myNewBitmap.fileName
				if currSelectedTextureSet == 8
				then 
				(
					try
					(
						delegate.bitmap = openBitmap bitmap8FileName
					)
					catch
					(
						-- well, no luck..
					)
				)


			)			
		)
		
		on view8 pressed do
		(	
			try
			(	
				local myBitmap = openBitMap cBitmap8FileName.text
				display myBitmap
			)
			catch
			(
				messagebox "Unable to find / display bitmap"
			)
		)

	)
)

----------------------------------------------------------------------------------------------------------------
fn convertMaxBitmapToNelBitmap maxMap =
(
	if classOf maxMap == NelBitmapTexture then
	(
		return maxMap -- no needs to change
	)
	if maxMap == undefined then 
	(
		print "undefined map"
		return undefined
	)
	print "converting bitmap"
	nelMap = NelBitmapTexture()
	base = nelMap.delegate
	-- copy args
	base.clipu         = maxMap.clipu
	base.clipv         = maxMap.clipv
	base.clipw         = maxMap.clipw
	base.cliph         = maxMap.cliph
	base.jitter        = maxMap.jitter
	base.useJitter     = maxMap.useJitter
	base.apply         = maxMap.apply
	base.cropPlace     = maxMap.cropPlace
	base.filtering     = maxMap.filtering
	base.monoOutput    = maxMap.monoOutput
	base.rgbOutput     = maxMap.rgbOutput
	base.alphaSource   = maxMap.alphaSource
	base.preMultAlpha  = maxMap.preMultAlpha
	base.bitmap	       = maxMap.bitmap
	base.coords	       = maxMap.coords
	base.output	       = maxMap.output
	base.fileName	   = maxMap.fileName
	base.startTime     = maxMap.startTime
	base.playBackRate  = maxMap.playBackRate
	base.endCondition  = maxMap.endCondition
	
	-- Duplicate the bitmap in the first bitmap slot
	nelMap.bitmap1FileName = nelMap.bitmap.fileName
	
	print "return nel map"
	return nelMap
)

----------------------------------------------------------------------------------------------------------------
-- takes a material (max standard, max Multi/SubObject, nel), and transform each of its texture that are bitmap
-- to a NeL bitmap.
fn convertMaterialChangingBitmap destMat =
(
	if (classOf destMat == NelMaterial) then -- is it a NelMaterial ?
	(
		print "converting nel material"
		destMat.tTexture_1 = convertMaxBitmapToNelBitmap(destMat.tTexture_1)
		destMat.tTexture_2 = convertMaxBitmapToNelBitmap(destMat.tTexture_2)
		destMat.tTexture_3 = convertMaxBitmapToNelBitmap(destMat.tTexture_3)
		destMat.tTexture_4 = convertMaxBitmapToNelBitmap(destMat.tTexture_4)
		destMat.tTexture_5 = convertMaxBitmapToNelBitmap(destMat.tTexture_5)
		destMat.tTexture_6 = convertMaxBitmapToNelBitmap(destMat.tTexture_6)
		destMat.tTexture_7 = convertMaxBitmapToNelBitmap(destMat.tTexture_7)
		destMat.tTexture_8 = convertMaxBitmapToNelBitmap(destMat.tTexture_8)
	)
	else if (classOf destMat == StandardMaterial) then
	(
  	    print "converting standard material"
		for j = 1 to 12 do
		(
			destMat.maps[j] =	convertMaxBitmapToNelBitmap(destMat.maps[j])
		)
	)
	else if (classOf destMat == Multimaterial) then
	(
		-- performs this on each submaterial
		for k = 1 to destMat.materialList.count do
		(
			if destMat.materialList[k] != undefined then
			(
				convertMaterialChangingBitmap destMat.materialList[k]
			)
		)
	)	 
	-- else no convertible material
)

----------------------------------------------------------------------------------------------------------------
-- This function helps to convert bitmap for an object from max Bitmap to nel BitmapTexture. This operate on all selected objects
fn convertMaxBitmapToNelBitmapForSelection =
(
	for i = 1 to selection.count do
	(		
	  	if (selection[i].material != undefined) then
		(
			convertMaterialChangingBitmap selection[i].material
		)
	)
)


----------------------------------------------------------------------------------------------------------------
-- a MacroScript That performs a call to convertMaxBitmapToNelBitmapForMesh

macroScript ConvertMaxBitmapToNelBitmapMacroScript category:"NEVRAX Tools" tooltip:"Convert all material of the selection so that they use Nel Bitmap instead of max ones"
(
	convertMaxBitmapToNelBitmapForSelection()
) 


----------------------------------------------------------------------------------------------------------------
-- Select a slot in a NeL bitmap
fn selectNelBitmap nelBitmap bitmapIndex =
(
	if (classOf nelBitmap == NelBitmapTexture) then
	(
		print "selecting bitmap : " + bitmapIndex as string
		local srcBitmapName
		if bitmapIndex == 1 then ( srcBitmapName = nelBitmap.bitmap1FileName )
		if bitmapIndex == 2 then ( srcBitmapName = nelBitmap.bitmap2FileName )
		if bitmapIndex == 3 then ( srcBitmapName = nelBitmap.bitmap3FileName )
		if bitmapIndex == 4 then ( srcBitmapName = nelBitmap.bitmap4FileName )
		if bitmapIndex == 5 then ( srcBitmapName = nelBitmap.bitmap5FileName )
		if bitmapIndex == 6 then ( srcBitmapName = nelBitmap.bitmap6FileName )
		if bitmapIndex == 7 then ( srcBitmapName = nelBitmap.bitmap7FileName )
		if bitmapIndex == 8 then ( srcBitmapName = nelBitmap.bitmap8FileName )

				
		-- If not present takes the first bitmap
		if srcBitmapName == "" then 
		(
			srcBitmapName = nelBitmap.bitmap1FileName
		)		
		if srcBitmapName == undefined then
		(
			srcBitmapName = ""
		)		
		print "fileName = " + srcBitmapName
		nelBitmap.bitmap.fileName = srcBitmapName
		nelBitmap.currSelectedTextureSet = bitmapIndex
		NelReloadTexture nelBitmap.bitmap
	)
	else
	(
		if nelBitmap != undefined then
		(
			print "selecting bitmap : not a NelBitmapTexture"
		)
	)
)


----------------------------------------------------------------------------------------------------------------
-- Select one bitmap in a nel MultiBitmap for the given material, and setup it as the current displayed bitmap
fn selectNelBitmapInMaterial destMat bitmapIndex =
(
	-- we just takes the bitmap from the given bitmap or the bitmap from the slot 0 if it doesn't exist
	if (classOf destMat == NelMaterial) then -- is it a NelMaterial ?
	(
		print "selecting textures in a NelMaterial"
		selectNelBitmap destMat.tTexture_1 bitmapIndex
		selectNelBitmap destMat.tTexture_2 bitmapIndex
		selectNelBitmap destMat.tTexture_3 bitmapIndex
		selectNelBitmap destMat.tTexture_4 bitmapIndex
		selectNelBitmap destMat.tTexture_5 bitmapIndex
		selectNelBitmap destMat.tTexture_6 bitmapIndex
		selectNelBitmap destMat.tTexture_7 bitmapIndex
		selectNelBitmap destMat.tTexture_8 bitmapIndex
	)
	else if (classOf destMat == StandardMaterial) then
	(
		print "selecting textures in a StandardMaterial"
		for j = 1 to 12 do
		(
			selectNelBitmap destMat.maps[j] bitmapIndex
		)
	)
	else if (classOf destMat == Multimaterial) then
	(
		print "selecting textures in a Multimaterial"
		-- performs this on each submaterial
		for k = 1 to destMat.materialList.count do
		(
			if destMat.materialList[k] != undefined then
			(
				selectNelBitmapInMaterial destMat.materialList[k] bitmapIndex
			)
		)
	)	 
	else
	(
		print "no convertible materials :" + (classOf destMat) as string
	)
)

----------------------------------------------------------------------------------------------------------------

fn selectNelBitmapInSelection bitmapIndex =
(
	for n = 1 to selection.count do
	(
			if selection[n].material != undefined then
			(
				selectNelBitmapInMaterial selection[n].material bitmapIndex
			)
	)
)


----------------------------------------------------------------------------------------------------------------
-- Macro scripts that performs the selection of a set of bitmaps
macroScript SelectNelBitmapSet1 category:"NEVRAX Tools" tooltip:"Select the first set of bitmap for the selected meshs"
(
	selectNelBitmapInSelection 1	
) 

macroScript SelectNelBitmapSet2 category:"NEVRAX Tools" tooltip:"Select the second set of bitmap for the selected meshs"
(
	selectNelBitmapInSelection 2
)

macroScript SelectNelBitmapSet3 category:"NEVRAX Tools" tooltip:"Select the third set of bitmap for the selected meshs"
(
	selectNelBitmapInSelection 3
)

macroScript SelectNelBitmapSet4 category:"NEVRAX Tools" tooltip:"Select the fourth set of bitmap for the selected meshs"
(
	selectNelBitmapInSelection 4
)

macroScript SelectNelBitmapSet5 category:"NEVRAX Tools" tooltip:"Select the fifth set of bitmap for the selected meshs"
(
	selectNelBitmapInSelection 5	
) 

macroScript SelectNelBitmapSet6 category:"NEVRAX Tools" tooltip:"Select the sixth set of bitmap for the selected meshs"
(
	selectNelBitmapInSelection 6
)

macroScript SelectNelBitmapSet7 category:"NEVRAX Tools" tooltip:"Select the seventh set of bitmap for the selected meshs"
(
	selectNelBitmapInSelection 7
)

macroScript SelectNelBitmapSet8 category:"NEVRAX Tools" tooltip:"Select the eigth set of bitmap for the selected meshs"
(
	selectNelBitmapInSelection 8
)


