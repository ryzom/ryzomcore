-- ********************
-- * Developper notes *
-- ********************
-- * 
-- * - Increment major version when changing the parameters structure, comment your new attributes 
-- *     to tell they have been added since version x. Backup the old script file to keep old file format.
-- * - Increment minor version when changing the code or UI
-- * - Manage new field initialisation when changing the parameters structure in the update method
-- * - In your changes, keep in mind the discreet note:
-- *     Parameter blocks that were per-instance cannot be made per-class and vice versa. 
-- *     Parameters cannot change type across redefinitions. 
-- *     Parameters cannot not move from one parameter block to another across redefinitions 
-- *
-- ********************

-- *** Some constant

-- Blend mode

BlendOne = 1
BlendZero = 2
BlendSrcAlpha = 3
BlendInvSrcAlpha = 4
BlendSrcRGB = 5
BlendInvSrcRGB = 6
BlendDestAlpha = 7
BlendInvDestAlpha = 8
BlendDestRGB = 9
BlendInvDestRGB = 10

-- Stage operator

OptorReplace = 1
OptorModulate = 2
OptorAdd = 3
OptorAddSigned = 4
OptorBlend = 5

-- Stage source

SrcBlendTexture = 1
SrcBlendPrevious = 2
SrcBlendDiffuse = 3
SrcBlendConstant = 4

Src0Texture = 1
Src0Previous = 2
Src0Diffuse = 3
Src0Constant = 4

Src1Previous = 1
Src1Diffuse = 2
Src1Constant = 3

-- Stage source operand

OprdRGB = 1
OprdInvRGB = 2
OprdAlpha = 3
OprdInvAlpha = 4

-- Stage operand

OprandSrcRGB = 1
OprandInvSrcRGB = 2
OprandSrcAlpha = 3
OprandInvSrcAlpha = 4

-- Texture Shader

TextureShaderDisable = 1

-- Normal Shader

ShaderSingleTexture = 1
ShaderAddTwoTextures = 2
ShaderMulTwoTextures = 3
ShaderBlendTwoTextures = 4
ShaderBlendMask = 5
ShaderMixTwoTextures = 6

-- Shader Type

ShaderNormal = 1
ShaderBump = 2
ShaderUserColor = 3
ShaderLightmap = 4
ShaderSpecular = 5
ShaderWater = 6
ShaderPerPixelLighting = 7
ShaderPerPixelLightingNoSpec = 7



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



----------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------

-- Nel Material properties
plugin material NelMaterial
    name:"NeL Material"
    classID:#(0x64c75fec, 0x222b9eb9)
    extends:standard 
	replaceUI:true
	version:14.0
(
	local loadShader
	local globalUpdateUI
	local updateTextureName

	local updateToVersion4 = false
	local updateToVersion14 = false
	
	parameters nlbp rollout:nelBasicParameters
	(
		bTwoSided type:#boolean
		cAmbient type:#color
		cDiffuse type:#color
		pOpacity type:#percent
		cSpecular type:#color 
		pSpecularLevel type:#percent
		pGlossiness type:#percent
		cSelfIllumColor type:#color
		pSelfIllumAmount type:#percent
		bUseSelfIllumColor type:#boolean
	)

    parameters main rollout:NelParams
    (
		-- 3d part of the material
		bLightMap type:#boolean default:false
		bUnlighted type:#boolean default:false ui:cbUnlighted
		bStainedGlassWindow type:#boolean default:false ui:cbStainedGlassWindow

		bAlphaTest 		type:#boolean default:false ui:cbAlphaTest
		bForceZWrite 	type:#boolean default:false ui:cbForceZWrite
		bForceNoZWrite 	type:#boolean default:false ui:cbForceNoZWrite
		bWater         	type:#boolean default:false

		bWaterUseSceneEnvMapAbove type:#boolean default:false ui:cbWaterUseSceneEnvMapAbove
		bWaterUseSceneEnvMapUnder type:#boolean default:false ui:cbWaterUseSceneEnvMapUnder


		fBumpUScale     type:#float default: 0.01 ui:cfBumpUScale
		fBumpVScale     type:#float default: 0.01 ui:cfBumpVScale	
		fBumpUSpeed     type:#float default: 0.01 ui:cfBumpUSpeed
		fBumpVSpeed     type:#float default: 0.01 ui:cfBumpVSpeed

		fDisplaceMapUScale     type:#float default: 0.005 ui:cfDisplaceMapUScale
		fDisplaceMapVScale     type:#float default: 0.005 ui:cfDisplaceMapVScale	
		fDisplaceMapUSpeed     type:#float default: 0.005 ui:cfDisplaceMapUSpeed
		fDisplaceMapVSpeed     type:#float default: 0.005 ui:cfDisplaceMapVSpeed
		
		fWaterHeightFactor	   type:#float default: 1.0   ui:cfWaterHeightFactor
		iWaterPoolID		   type:#integer default: 0   ui:ciWaterPoolID

		-- new in version 7
		bCalcWaterLightmap         	type:#boolean default:false ui:cbCalcWaterLightmap

		-- new in version 10
		bEnableWaterSplash         	type:#boolean default:true ui:cbEnableWaterSplash
		
	
		-- New in version 3
		bColorVertex	type:#boolean default:false ui:cbColorVertex
		bAlphaVertex	type:#boolean default:false ui:cbAlphaVertex
		iAlphaVertexChannel		type:#integer default:0 ui:ciAlphaVertexChannel
	
		-- New in version 4
		bAlphaBlend		type:#boolean default:false ui:cbAlphaBlend
		iBlendSrcFunc	type:#integer default:1
		iBlendDestFunc	type:#integer default:1
		iShader			type:#integer default:1
		fZBias			type:#float default:0 ui:cfZBias
		iLightMapChannel type:#integer default:2 ui:ciLightMapChannel
		iShaderType		type:#integer default:1
		bUserColor		type:#boolean
		bSpecular		type:#boolean
		bNormalShader	type:#boolean
		bPerPixelLightingShader type:#boolean
		cUserColor		type:#color default:[255,0,255] ui:cpUserColor
		
		-- New in version 5		
		bExportTextureMatrix    type:#boolean default:false	ui:cbExportTextureMatrix
	)

    parameters textures rollout:NelTexture
    (
		bEnableSlot_1			type:#boolean default:false ui:cbEnableSlot_1
		bEnableSlot_2			type:#boolean default:false ui:cbEnableSlot_2
		bEnableSlot_3			type:#boolean default:false ui:cbEnableSlot_3
		bEnableSlot_4			type:#boolean default:false ui:cbEnableSlot_4
		bEnableSlot_5			type:#boolean default:false ui:cbEnableSlot_5
		bEnableSlot_6			type:#boolean default:false ui:cbEnableSlot_6
		bEnableSlot_7			type:#boolean default:false ui:cbEnableSlot_7
		bEnableSlot_8			type:#boolean default:false ui:cbEnableSlot_8
		bTexGen_1				type:#boolean default:false ui:cbTexGen_1
		bTexGen_2				type:#boolean default:false ui:cbTexGen_2
		bTexGen_3				type:#boolean default:false ui:cbTexGen_3
		bTexGen_4				type:#boolean default:false ui:cbTexGen_4
		bTexGen_5				type:#boolean default:false ui:cbTexGen_5
		bTexGen_6				type:#boolean default:false ui:cbTexGen_6
		bTexGen_7				type:#boolean default:false ui:cbTexGen_7
		bTexGen_8				type:#boolean default:false ui:cbTexGen_8
		tTexture_1				type:#texturemap ui:mbTexture_1
		tTexture_2				type:#texturemap ui:mbTexture_2
		tTexture_3				type:#texturemap ui:mbTexture_3
		tTexture_4				type:#texturemap ui:mbTexture_4
		tTexture_5				type:#texturemap ui:mbTexture_5
		tTexture_6				type:#texturemap ui:mbTexture_6
		tTexture_7				type:#texturemap ui:mbTexture_7
		tTexture_8				type:#texturemap ui:mbTexture_8
	)

    parameters slot1 rollout:NelMultitextureSlot1
    (
		-- New in version 4
		cConstant_1				type:#color default:[255,255,255] ui:cpConstant
		iRgbOperation_1			type:#integer default:1
		iRgbBlendSource_1		type:#integer default:1
		iRgbArg0_1				type:#integer default:1
		iRgbArg1_1				type:#integer default:1
		iRgbArg2_1				type:#integer default:1
		iRgbArg0Operand_1		type:#integer default:1
		iRgbArg1Operand_1		type:#integer default:1
		iRgbArg2Operand_1		type:#integer default:1
		iAlphaOperation_1		type:#integer default:1
		iAlphaBlendSource_1		type:#integer default:1
		iAlphaArg0_1			type:#integer default:1
		iAlphaArg1_1			type:#integer default:1
		iAlphaArg2_1			type:#integer default:1
		iAlphaArg0Operand_1		type:#integer default:3
		iAlphaArg1Operand_1		type:#integer default:3
		iAlphaArg2Operand_1		type:#integer default:3
		iTextureShader_1		type:#integer default:1
		iConstantAlpha_1		type:#integer default:255 ui:sdConstantAlpha
	)

    parameters slot2 rollout:NelMultitextureSlot2
    (
		-- New in version 4
		cConstant_2				type:#color default:[255,255,255] ui:cpConstant
		iRgbOperation_2			type:#integer default:1
		iRgbBlendSource_2		type:#integer default:1
		iRgbArg0_2				type:#integer default:1
		iRgbArg1_2				type:#integer default:1
		iRgbArg2_2				type:#integer default:1
		iRgbArg0Operand_2		type:#integer default:1
		iRgbArg1Operand_2		type:#integer default:1
		iRgbArg2Operand_2		type:#integer default:1
		iAlphaOperation_2		type:#integer default:1
		iAlphaBlendSource_2		type:#integer default:1
		iAlphaArg0_2			type:#integer default:1
		iAlphaArg1_2			type:#integer default:1
		iAlphaArg2_2			type:#integer default:1
		iAlphaArg0Operand_2		type:#integer default:3
		iAlphaArg1Operand_2		type:#integer default:3
		iAlphaArg2Operand_2		type:#integer default:3
		iTextureShader_2		type:#integer default:1
		iConstantAlpha_2		type:#integer default:255 ui:sdConstantAlpha
	)

    parameters slot3 rollout:NelMultitextureSlot3
    (
		-- New in version 7
		cConstant_3				type:#color default:[255,255,255] ui:cpConstant
		iRgbOperation_3			type:#integer default:1
		iRgbBlendSource_3		type:#integer default:1
		iRgbArg0_3				type:#integer default:1
		iRgbArg1_3				type:#integer default:1
		iRgbArg2_3				type:#integer default:1
		iRgbArg0Operand_3		type:#integer default:1
		iRgbArg1Operand_3		type:#integer default:1
		iRgbArg2Operand_3		type:#integer default:1
		iAlphaOperation_3		type:#integer default:1
		iAlphaBlendSource_3		type:#integer default:1
		iAlphaArg0_3			type:#integer default:1
		iAlphaArg1_3			type:#integer default:1
		iAlphaArg2_3			type:#integer default:1
		iAlphaArg0Operand_3		type:#integer default:3
		iAlphaArg1Operand_3		type:#integer default:3
		iAlphaArg2Operand_3		type:#integer default:3
		iTextureShader_3		type:#integer default:1
		iConstantAlpha_3		type:#integer default:255 ui:sdConstantAlpha
	)

    parameters slot4 rollout:NelMultitextureSlot4
    (
		-- New in version 7
		cConstant_4				type:#color default:[255,255,255] ui:cpConstant
		iRgbOperation_4			type:#integer default:1
		iRgbBlendSource_4		type:#integer default:1
		iRgbArg0_4				type:#integer default:1
		iRgbArg1_4				type:#integer default:1
		iRgbArg2_4				type:#integer default:1
		iRgbArg0Operand_4		type:#integer default:1
		iRgbArg1Operand_4		type:#integer default:1
		iRgbArg2Operand_4		type:#integer default:1
		iAlphaOperation_4		type:#integer default:1
		iAlphaBlendSource_4		type:#integer default:1
		iAlphaArg0_4			type:#integer default:1
		iAlphaArg1_4			type:#integer default:1
		iAlphaArg2_4			type:#integer default:1
		iAlphaArg0Operand_4		type:#integer default:3
		iAlphaArg1Operand_4		type:#integer default:3
		iAlphaArg2Operand_4		type:#integer default:3
		iTextureShader_4		type:#integer default:1
		iConstantAlpha_4		type:#integer default:255 ui:sdConstantAlpha
	)

    parameters slot5
    (
	)

    parameters slot6
    (
	)
	
	rollout nelBasicParameters "NeL Basic Parameters" rolledUp:false
	(
		Label lblNlbpA "NeL Material" align:#center across:3
		Label lblNlbpB "http://dev.ryzom.com/" align:#center
		CheckBox cbTwoSided "2-Sided" checked:false align:#right
		
		group "Standard Lighting"
		(
			-- Label lblNlbpslA "Diffuse is the color in light, ambient in the dark." align:#right
			-- Label lblNlbpslB "Ambient and diffuse multiply with the textures and global sun." align:#right
			ColorPicker cpAmbient "Ambient" color:[63,63,63] align:#left across:3
			ColorPicker cpDiffuse "Diffuse" color:[127,127,127] align:#left
			Spinner spOpacity "Opacity" range:[0,100,100] type:#integer scale:1 align:#left
		)
		
		group "Specular"
		(
			-- Label lblNlbpsA "Increase the specular level to see the specular color." align:#right
			-- Label lblNlbpsB "Multiplies with sun. Glossiness changes the size of the specular." align:#right
			ColorPicker cpSpecular "Specular" color:[255,255,255] align:#left across:3
			Spinner spSpecularLevel "Level" range:[0,100,0] type:#integer scale:1 align:#left
			Spinner spGlossiness "Glossiness"range:[0,80,10] type:#integer scale:1 align:#left
		)
		
		group "Self Illumination"
		(
			-- Label lblNlbpsiA "Set the self illumination by color or by amount." align:#right
			ColorPicker cpSelfIllumColor "Self Illum"color:[0,0,0] align:#left across:3
			Spinner spSelfIllumAmount "Self Illum" range:[0,100,0] type:#integer scale:1 align:#left
			CheckBox cbUseSelfIllumColor "Use Color" checked:false align:#right
		)
		
		Fn updateUI update =
		(
			if (version >= 14) then
			(
				if update == true then
				(
					cbTwoSided.checked = bTwoSided
					cpAmbient.color = cAmbient
					cpDiffuse.color = cDiffuse
					spOpacity.value = pOpacity
					cpSpecular.color = cSpecular
					spSpecularLevel.value = pSpecularLevel
					spGlossiness.value = pGlossiness
					cpSelfIllumColor.color = cSelfIllumColor
					spSelfIllumAmount.value = pSelfIllumAmount
					cbUseSelfIllumColor.checked = bUseSelfIllumColor
					
					spSelfIllumAmount.visible = not bUseSelfIllumColor
					cpSelfIllumColor.visible = bUseSelfIllumColor
				)
				else
				(					
					bTwoSided = cbTwoSided.checked
					cAmbient = cpAmbient.color
					cDiffuse = cpDiffuse.color
					pOpacity = spOpacity.value
					cSpecular = cpSpecular.color
					pSpecularLevel = spSpecularLevel.value
					pGlossiness = spGlossiness.value
					cSelfIllumColor = cpSelfIllumColor.color
					pSelfIllumAmount = spSelfIllumAmount.value
					bUseSelfIllumColor = cbUseSelfIllumColor.checked
					
					delegate.twoSided = bTwoSided
					delegate.ambient = cAmbient
					delegate.diffuse = cDiffuse
					delegate.opacity = pOpacity
					delegate.specular = cSpecular
					delegate.specularLevel = pSpecularLevel
					delegate.glossiness = pGlossiness
					delegate.selfIllumColor = cSelfIllumColor
					delegate.selfIllumAmount = pSelfIllumAmount
					delegate.useSelfIllumColor = bUseSelfIllumColor
				)
				
				delegate.adTextureLock = false
				delegate.adLock = false
				delegate.dsLock = false
			)
		)
		
		on cbTwoSided changed bval do
			updateUI false
		
		on cpAmbient changed cval do
			updateUI false
		
		on cpDiffuse changed cval do
			updateUI false
		
		on spOpacity changed pval do
			updateUI false
		
		on cpSpecular changed cval do
			updateUI false
		
		on spSpecularLevel changed pval do
			updateUI false
		
		on spGlossiness changed pval do
			updateUI false
		
		on cpSelfIllumColor changed cval do
			updateUI false
		
		on spSelfIllumAmount changed bval do
			updateUI false
		
		on cbUseSelfIllumColor changed bval do
			updateUI false
		
		on nelBasicParameters open do
		(
			updateUI true
		)
	)

    rollout NelParams "NeL Material" rolledUp:true
    (
		-- UI update timer
		timer pWatch interval:450
		local activeMaterial = undefined

		dropdownlist dlShaderType "Shaders Type" items:#( "Normal (simple multitexture material)", "Not Implemented (do not use, bump material not implemented)", 
			"UserColor (display user color on texture inverse alpha)", "LightMap (material used for lightmapping)", 
			"Specular (material with specular reflection cubemap)", "Water (shiny water shader)", 
			"Per pixel lighting, require one texmap applied",  "Per pixel lighting, no specular, require one texmap applied")
		group "Light"
		(
			CheckBox cbUnlighted "Unlighted"
			Label lLightMapChannel "Lightmap Texture Channel" align:#left across:2
			Spinner   ciLightMapChannel ""  range:[0,99,0] type:#integer scale:1 align:#left width:50
			CheckBox cbStainedGlassWindow "Lightmap stained glass window"
			CheckBox cbColorVertex "Color Vertex"
		)
		group "Alpha"
		(
			CheckBox cbAlphaBlend "Alpha Blend"
			Button btSetBlend "Setup Blend" align:#left across:3
			Button btSetAdd "Setup Additive" align:#left
			Button btSetAlphaAdd "Setup Alpha Additive" align:#left
			dropdownlist dlBlendSrcFunc "Blend Source" items:#( "One", "Zero", "Source Alpha", "Inverse Source Alpha", "Source RGBA", "Inverse Source RGB", "Destination Alpha", "Inverse Destination Alpha", "Destination RGB", "Inverse Destination RGB" ) across:2
			dropdownlist dlBlendDestFunc "Blend Destination" items:#( "One", "Zero", "Source Alpha", "Inverse Source Alpha", "Source RGBA", "Inverse Source RGB", "Destination Alpha", "Inverse Destination Alpha", "Destination RGB", "Inverse Destination RGB" ) 
			CheckBox cbAlphaTest "Alpha Test"
			CheckBox cbAlphaVertex "Alpha Vertex" across:2
			Spinner   ciAlphaVertexChannel "Alpha Vertex Channel"  range:[0,99,0] type:#integer scale:1 align:#right
		)
		group "Z buffer"
		(
			Label	cfZBiasLabel "ZBias :"  align:#left across:2
			Spinner	cfZBias "" range:[-100,100,0] type:#float scale:0.001 align:#left width:70
			CheckBox cbForceZWrite "Force ZWrite" across:2
			CheckBox cbForceNoZWrite "Force No ZWrite"
		)
		group "Water"
		(
			Label	lbWaterPoolID "Water pool ID"  align:#left across:2
			Spinner	ciWaterPoolID     "" range:[0,3,1] type:#integer align:#left width:70
			Label	 lbWaterHeightFactor "Water height factor" align:#left across:2
			Spinner	cfWaterHeightFactor   "" range:[0,1000,1] type:#integer align:#left width:70

			CheckBox cbWaterUseSceneEnvMapAbove "Use scene envmap above water" across:2
			CheckBox cbWaterUseSceneEnvMapUnder "under water" align:#right
			
			Label	cfBumpScaleLabel "BumpMap Scale"  align:#left across:3
			Spinner	cfBumpUScale      "U" range:[-10,10,0.01] type:#float scale:0.001 align:#right width:70
			Spinner	cfBumpVScale      "V" range:[-10,10,0.01] type:#float scale:0.001 align:#right width:70
			Label	cfBumpSpeedLabel "BumpMap Speed" align:#left across:3 
			Spinner	cfBumpUSpeed      "U" range:[-10,10,0.01] type:#float scale:0.001 align:#right width:70
			Spinner	cfBumpVSpeed      "V" range:[-10,10,0.01] type:#float scale:0.001 align:#right width:70
			
		
			Label	lbDisplaceMapScale "DisplaceMap Scale" align:#left across:3
			Spinner   cfDisplaceMapUScale      "U"  range:[-10,10,0.005] type:#float scale:0.001 align:#right width:70
			Spinner   cfDisplaceMapVScale      "V"  range:[-10,10,0.005] type:#float scale:0.001 align:#right width:70
			Label	lbDisplaceMapSpeed "DisplaceMap Speed" align:#left across:3
			Spinner   cfDisplaceMapUSpeed      "U"  range:[-10,10,0.01] type:#float scale:0.001 align:#right width:70
			Spinner   cfDisplaceMapVSpeed      "V"  range:[-10,10,0.01] type:#float scale:0.001 align:#right width:70

			CheckBox cbCalcWaterLightmap "Compute water lightmap" across:2
			CheckBox cbEnableWaterSplash "Enable water splash"
		)
		group "Extended"
		(
			dropdownlist dlVertexProgram "Vertex Program" items:#( "Disable" ) across:2
			ColorPicker cpUserColor "User Color" color:[255,0,0] align:#right height:40
			CheckBox cbExportTextureMatrix "Export texture matrix"
		)
		
		Fn updateUI update =
		(
			if (version >= 4) then
			(
				if update == true then
				(
					dlBlendSrcFunc.selection = iBlendSrcFunc
					dlBlendDestFunc.selection = iBlendDestFunc
					dlShaderType.selection = iShaderType
				)
				else
				(
					iBlendSrcFunc = dlBlendSrcFunc.selection
					iBlendDestFunc = dlBlendDestFunc.selection
					iShaderType = dlShaderType.selection
				)

				-- Setup boolean value
				bNormalShader = false
				bPerPixelLightingShader = false;
				bUserColor = false
				bLightmap = false
				bSpecular = false
				bWater = false
				if iShaderType == ShaderNormal then
					bNormalShader = true
				else if iShaderType == ShaderPerPixelLighting or iShaderType == ShaderPerPixelLightingNoSpec then
					bPerPixelLightingShader = true;
				else if iShaderType == ShaderUserColor then
					bUserColor = true
				else if iShaderType == ShaderLightmap then
					bLightmap = true
				else if iShaderType == ShaderSpecular then
					bSpecular = true
				else if iShaderType == ShaderWater then
					bWater = true

				dlVertexProgram.enabled = (not bWater) and (not bLightmap)

				cbUnlighted.enabled = (not bWater) and (not bLightmap)
				lLightMapChannel.enabled = (not bWater) and bLightmap
				ciLightMapChannel.enabled = (not bWater) and bLightmap
				cbStainedGlassWindow.enabled = not bWater

				-- Yoyo: allow alphaBlend when Lightmap is enabled, but Only std blend is possible, without alphaVertex etc...
				cbAlphaBlend.enabled = (not bWater) and (not bAlphaTest)
				btSetBlend.enabled = ((not bWater) and (not bLightmap)) and bAlphaBlend and (not bAlphaTest)
				btSetAdd.enabled = ((not bWater) and (not bLightmap)) and bAlphaBlend and (not bAlphaTest)
				btSetAlphaAdd.enabled = ((not bWater) and (not bLightmap)) and bAlphaBlend and (not bAlphaTest)
				dlBlendSrcFunc.enabled = ((not bWater) and (not bLightmap)) and bAlphaBlend and (not bAlphaTest)
				dlBlendDestFunc.enabled = ((not bWater) and (not bLightmap)) and bAlphaBlend and (not bAlphaTest)
				cbAlphaTest.enabled = (not bWater) and (not bAlphaBlend)

				-- Yoyo: allow ZWrite modif when lightmap is used
				cbForceZWrite.enabled = (not bWater) and bAlphaBlend
				cbForceNoZWrite.enabled = (not bWater) and (not bAlphaBlend)
				cbAlphaVertex.enabled = (not bWater) and (not bLightmap) and (not bLightMap)
				ciAlphaVertexChannel.enabled = (not bWater) and bAlphaVertex and (not bLightMap)
				cbColorVertex.enabled = not bWater
				
				cfBumpUScale.enabled = bWater and (not bLightMap)
				cfBumpVScale.enabled = bWater and (not bLightMap)
				cfBumpUSpeed.enabled = bWater and (not bLightMap)
				cfBumpVSpeed.enabled = bWater and (not bLightMap)
			
				cfDisplaceMapUScale.enabled = bWater and (not bLightMap)
				cfDisplaceMapVScale.enabled = bWater and (not bLightMap)
				cfDisplaceMapUSpeed.enabled = bWater and (not bLightMap)
				cfDisplaceMapVSpeed.enabled = bWater and (not bLightMap)				
				ciWaterPoolID.enabled = bWater and (not bLightMap)
				cfWaterHeightFactor.enabled = bWater and (not bLightMap)

				if (bWater == true) then
				(
					cbUnlighted.checked = false 
					cbStainedGlassWindow.checked = false
					cbAlphaTest.checked = false
					cbForceZWrite.checked = false
					cbForceNoZWrite.checked = false
				)			
			)
		)

		on NelParams reload do
		(
		)

		on NelParams open do 
		(
			updateUI true
		)

		on NelParams close do 
		(
		)

		-- Update ui

		on dlShaderType selected newState do
		(
			globalUpdateUI false
		)

		on cbUnlighted changed bNewState do
		(
			updateUI false
		)

		on cbAlphaBlend changed bNewState do
        (
			if bAlphaBlend == true then
				bAlphaTest = false
			updateUI false
        )

		on cbAlphaTest changed bNewState do
        (
			if bAlphaTest == true then
				bAlphaBlend = false
			updateUI false
        )

		on cbAlphaVertex changed bNewState do
        (
			updateUI false
        )

		on cbAlphaVertex changed bNewState do
        (
			updateUI false
        )

		on btSetBlend pressed do
		(
			dlBlendSrcFunc.selection = 3
			dlBlendDestFunc.selection = 4
			updateUI false
		)

		on btSetAdd pressed do
		(
			dlBlendSrcFunc.selection = 1
			dlBlendDestFunc.selection = 1
			updateUI false
		)

		on btSetAlphaAdd pressed do
		(
			dlBlendSrcFunc.selection = 3
			dlBlendDestFunc.selection = 1
			updateUI false
		)

		on pWatch tick do
		(
			if (activeMaterial != this) then
			(
				activeMaterial = this
				globalUpdateUI true
			)
		)
	)

    rollout NelTexture "NeL Textures" rolledUp:true
    (
		group "Textures"
		(
			Label lblNttA "Texture format must be TGA or DDS for ingame usage." align:#right
			Label lblNttBl "Enabled" align:#left across:4
			Label lblNttBcA "" align:#center
			Label lblNttBcB "Texture    " align:#center
			Label lblNttBr "EnvMap" align:#right
			CheckBox cbEnableSlot_1 "Texture 1...................." width:110 align:#left across:3
			MapButton mbTexture_1 "" toolTips:"Select the texture 1" width:160 align:#left
			CheckBox cbTexGen_1 "" align:#right
			CheckBox cbEnableSlot_2 "Texture 2...................." width:110 align:#left across:3
			MapButton mbTexture_2 "" toolTips:"Select the texture 2" width:160 align:#left
			CheckBox cbTexGen_2 "" align:#right
			CheckBox cbEnableSlot_3 "Texture 3...................." width:110 align:#left across:3
			MapButton mbTexture_3 "" toolTips:"Select the texture 3" width:160 align:#left
			CheckBox cbTexGen_3 "" align:#right
			CheckBox cbEnableSlot_4 "Texture 4...................." width:110 align:#left across:3
			MapButton mbTexture_4 "" toolTips:"Select the texture 4" width:160 align:#left
			CheckBox cbTexGen_4 "" align:#right
			CheckBox cbEnableSlot_5 "Texture 5...................." width:110 align:#left across:3
			MapButton mbTexture_5 "" toolTips:"Select the texture 5" width:160 align:#left
			CheckBox cbTexGen_5 "" align:#right
			CheckBox cbEnableSlot_6 "Texture 6...................." width:110 align:#left across:3
			MapButton mbTexture_6 "" toolTips:"Select the texture 6" width:160 align:#left
			CheckBox cbTexGen_6 "" align:#right
			CheckBox cbEnableSlot_7 "Texture 7...................." width:110 align:#left across:3
			MapButton mbTexture_7 "" toolTips:"Select the texture 7" width:160 align:#left
			CheckBox cbTexGen_7 "" align:#right
			CheckBox cbEnableSlot_8 "Texture 8...................." width:110 align:#left across:3
			MapButton mbTexture_8 "" toolTips:"Select the texture 8" width:160 align:#left
			CheckBox cbTexGen_8 "" align:#right
		)

		group "Setup Stage"
		(
			Label lblNtssA "Automatic configuration of the texture slots with simple material." align:#right
			dropdownlist dlShader "" items:#( "Single Texture (RGBA1) (A1)", "Addition Two Textures (RGB1+RGB2) (A2)", "Multiply Two Textures (RGB1*RGB2) (A2)", "Blend Two Textures (RGB1*(1-A1)+RGB2*A1) (A2)", "Blend Texture (RGB1)(A2)", "Mix Two Textures (RGB1*0.5+RGB2*0.5) (A1*0.5+A2*0.5)" ) 
			Button btLoadShader "Setup Texture Slots" align:#left
		)

		fn updateUI update =
		(
			if (version >= 4) then
			(
				cbEnableSlot_1.enabled = true
				cbEnableSlot_2.enabled = (not bLightmap) and (not bPerPixelLightingShader)
				cbEnableSlot_3.enabled = (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bPerPixelLightingShader)
				cbEnableSlot_4.enabled = (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bPerPixelLightingShader)
				cbEnableSlot_5.enabled = (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bNormalShader) and (not bPerPixelLightingShader)
				cbEnableSlot_6.enabled = (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bNormalShader) and (not bPerPixelLightingShader)
				cbEnableSlot_7.enabled = (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bNormalShader) and (not bPerPixelLightingShader)
				cbEnableSlot_8.enabled = (not bWater) and (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bNormalShader) and (not bPerPixelLightingShader)

				mbTexture_1.enabled = bEnableSlot_1
				mbTexture_2.enabled = bEnableSlot_2 and (not bLightmap) and (not bUserColor) and (not bPerPixelLightingShader)
				mbTexture_3.enabled = bEnableSlot_3 and (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bPerPixelLightingShader)
				mbTexture_4.enabled = bEnableSlot_4 and (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bPerPixelLightingShader)
				mbTexture_5.enabled = bEnableSlot_5 and (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bNormalShader) and (not bPerPixelLightingShader)
				mbTexture_6.enabled = bEnableSlot_6 and (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bNormalShader) and (not bPerPixelLightingShader)
				mbTexture_7.enabled = bEnableSlot_7 and (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bNormalShader) and (not bPerPixelLightingShader)
				mbTexture_8.enabled = bEnableSlot_8 and (not bWater) and (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bNormalShader) and (not bPerPixelLightingShader)

				cbTexGen_1.enabled = bNormalShader and bEnableSlot_1
				cbTexGen_2.enabled = bNormalShader and bEnableSlot_2 and (not bLightmap) and (not bUserColor) and (not bPerPixelLightingShader)
				cbTexGen_3.enabled = bNormalShader and bEnableSlot_3 and (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bPerPixelLightingShader)
				cbTexGen_4.enabled = bNormalShader and bEnableSlot_4 and (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bPerPixelLightingShader)
				cbTexGen_5.enabled = bNormalShader and bEnableSlot_5 and (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bNormalShader) and (not bPerPixelLightingShader)
				cbTexGen_6.enabled = bNormalShader and bEnableSlot_6 and (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bNormalShader) and (not bPerPixelLightingShader)
				cbTexGen_7.enabled = bNormalShader and bEnableSlot_7 and (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bNormalShader) and (not bPerPixelLightingShader)
				cbTexGen_8.enabled = bNormalShader and bEnableSlot_8 and (not bLightmap) and (not bSpecular) and (not bUserColor) and (not bNormalShader) and (not bPerPixelLightingShader)

				delegate.DiffuseMapEnable = bEnableSlot_1
				delegate.DiffuseMap = tTexture_1
				delegate.AmbientMapEnable = bEnableSlot_1
				delegate.AmbientMap = tTexture_1
				delegate.SpecularMapEnable = bEnableSlot_2
				delegate.SpecularMap = tTexture_2
				delegate.SelfIllumMap = undefined
				delegate.OpacityMap = undefined
				delegate.FilterMap = undefined
				delegate.BumpMap = undefined
				delegate.DisplacementMap = undefined
				delegate.ReflectionMap = undefined
				delegate.RefractionMap = undefined

				btLoadShader.enabled = bNormalShader
				dlShader.enabled = bNormalShader

				if bWater == true then
				(
					cbEnableSlot_1.text = "EnvDay......................."
					cbEnableSlot_2.text = "EnvNight (opt)..............."
					cbEnableSlot_3.text = "EnvDayUnder (opt)............"
					cbEnableSlot_4.text = "EnvNightUnder (opt).........."
					cbEnableSlot_5.text = "Bump........................."
					cbEnableSlot_6.text = "Displace....................."
					cbEnableSlot_7.text = "ColorAlpha (opt)............."
					cbEnableSlot_8.text = "Unused......................."
				)
				else
				(
					if bLightmap == true then
					(
						cbEnableSlot_1.text = "Diffuse......................"
						cbEnableSlot_2.text = "Unused......................."
						cbEnableSlot_3.text = "Unused......................."
						cbEnableSlot_4.text = "Unused......................."
						cbEnableSlot_5.text = "Unused......................."
						cbEnableSlot_6.text = "Unused......................."
						cbEnableSlot_7.text = "Unused......................."
						cbEnableSlot_8.text = "Unused......................."
					)
					else
					(
						if bUserColor == true then
						(
							cbEnableSlot_1.text = "Diffuse......................"
							cbEnableSlot_2.text = "UsercolorMask................"
							cbEnableSlot_3.text = "Unused......................."
							cbEnableSlot_4.text = "Unused......................."
							cbEnableSlot_5.text = "Unused......................."
							cbEnableSlot_6.text = "Unused......................."
							cbEnableSlot_7.text = "Unused......................."
							cbEnableSlot_8.text = "Unused......................."
						)
						else
						(
							if bSpecular == true then
							(
								cbEnableSlot_1.text = "Diffuse......................"
								cbEnableSlot_2.text = "SpecularMap.................."
								cbEnableSlot_3.text = "Unused......................."
								cbEnableSlot_4.text = "Unused......................."
								cbEnableSlot_5.text = "Unused......................."
								cbEnableSlot_6.text = "Unused......................."
								cbEnableSlot_7.text = "Unused......................."
								cbEnableSlot_8.text = "Unused......................."
							)
							else
							(
								cbEnableSlot_1.text = "Texture 1...................."
								cbEnableSlot_2.text = "Texture 2...................."
								cbEnableSlot_3.text = "Texture 3...................."
								cbEnableSlot_4.text = "Texture 4...................."
								cbEnableSlot_5.text = "Texture 5...................."
								cbEnableSlot_6.text = "Texture 6...................."
								cbEnableSlot_7.text = "Texture 7...................."
								cbEnableSlot_8.text = "Texture 8...................."
							)
						)
					)
				)

				if (update == true) then
				(
					dlShader.selection = iShader
				)
				else
				(
					iShader = dlShader.selection
				)

				-- Update texture name
				updateTextureName ()
			)
		)

		on NelTexture open do
		(
			updateUI false
		)

		on NelTexture close do 
		(
		)
		
		on tTexture_1 changed newState do
		(
			delegate.DiffuseMap = tTexture_1
			delegate.AmbientMap = tTexture_1
		)
		
		on tTexture_2 changed newState do
		(
			delegate.SpecularMap = tTexture_2
		)

		on cbEnableSlot_1 changed newState do
		(
			globalUpdateUI false
		)

		on cbEnableSlot_2 changed newState do
		(
			globalUpdateUI false
		)

		on cbEnableSlot_3 changed newState do
		(
			globalUpdateUI false
		)

		on cbEnableSlot_4 changed newState do
		(
			globalUpdateUI false
		)

		on cbEnableSlot_5 changed newState do
		(
			globalUpdateUI false
		)

		on cbEnableSlot_6 changed newState do
		(
			globalUpdateUI false
		)

		on cbEnableSlot_7 changed newState do
		(
			globalUpdateUI false
		)

		on cbEnableSlot_8 changed newState do
		(
			globalUpdateUI false
		)

		on dlShader selected newState do
		(
			updateUI false
		)

		on btLoadShader pressed do
		(
			loadShader dlShader.selection
		)
	)

    rollout NelMultitextureSlot1 "NeL Slot 1" rolledUp:true
    (
		group "RGB"
		(
			dropdownlist dlRgbOperation		"Operation" items:#( "Arg0", "Arg0*Arg1", "Arg0+Arg1", "Arg0+Arg1-0.5", "Alpha Blend", "Arg0*Arg1+Arg2" ) across:2
			dropdownlist dlRgbBlendSource	"Blend Source" items:#( "Texture", "Previous", "Diffuse", "Constant" )
			dropdownlist dlRgbArg0			"Arg0" items:#( "Texture", "Previous", "Diffuse", "Constant" ) across:6
			dropdownlist dlRgbArg0Operand	"Oper0" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
			dropdownlist dlRgbArg1			"Arg1" items:#( "Previous", "Diffuse", "Constant", "Texture" )
			dropdownlist dlRgbArg1Operand	"Oper1 Operand" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
			dropdownlist dlRgbArg2			"Arg2" items:#( "Previous", "Diffuse", "Constant", "Texture" )
			dropdownlist dlRgbArg2Operand	"Oper2" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
		)
		group "Alpha"
		(
			dropdownlist dlAlphaOperation	"Operation" items:#( "Arg0", "Arg0*Arg1", "Arg0+Arg1", "Arg0+Arg1-0.5", "Alpha Blend", "Arg0*Arg1+Arg2" ) across:2
			dropdownlist dlAlphaBlendSource	"Blend Source" items:#( "Texture", "Previous", "Diffuse", "Constant" )
			dropdownlist dlAlphaArg0		"Arg0" items:#( "Texture", "Previous", "Diffuse", "Constant", "Texture" ) across:6
			dropdownlist dlAlphaArg0Operand	"Oper0" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
			dropdownlist dlAlphaArg1		"Arg1" items:#( "Previous", "Diffuse", "Constant", "Texture" )
			dropdownlist dlAlphaArg1Operand	"Oper1" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
			dropdownlist dlAlphaArg2		"Arg2" items:#( "Previous", "Diffuse", "Constant", "Texture" )
			dropdownlist dlAlphaArg2Operand	"Oper2" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
		)
		group "Extended"
		(
			dropdownlist dlTextureShader "Texture Shader" items:#( "Disable", "TextureOff", "FetchTexture", "PassThrough", "CullFragment", "OffsetTexture", "OffsetTextureScale", "DependentARTexture", "DependentGBTexture", "DP3", "DP3Texture2D", "DP3CubeMap", "DP3ReflectCubeMap", "DP3ConstEyeReflectCubeMap", "DP3DiffuseCubeMap", "DP3DepthReplace" ) width:100 across:2
			ColorPicker cpConstant "Constant Color" align:#right height:40
			Label		ldConstantAlpha "Constant Alpha" align:#left across:2
			Spinner		 sdConstantAlpha ""  range:[0,255,255] type:#integer scale:1 align:#left width:50
		)

		fn updateUI update =
		(
			if (version >= 4) then
			(
				-- update droplist
				if update == true then
				(
					dlRgbOperation.selection = iRgbOperation_1
					dlRgbBlendSource.selection = iRgbBlendSource_1
					dlRgbArg0.selection = iRgbArg0_1
					dlRgbArg1.selection = iRgbArg1_1
					dlRgbArg2.selection = iRgbArg2_1
					dlRgbArg0Operand.selection = iRgbArg0Operand_1
					dlRgbArg1Operand.selection = iRgbArg1Operand_1
					dlRgbArg2Operand.selection = iRgbArg2Operand_1
					dlAlphaOperation.selection = iAlphaOperation_1
					dlAlphaBlendSource.selection = iAlphaBlendSource_1
					dlAlphaArg0.selection = iAlphaArg0_1 
					dlAlphaArg1.selection = iAlphaArg1_1 
					dlAlphaArg2.selection = iAlphaArg2_1 
					dlAlphaArg0Operand.selection = iAlphaArg0Operand_1
					dlAlphaArg1Operand.selection = iAlphaArg1Operand_1
					dlAlphaArg2Operand.selection = iAlphaArg2Operand_1
					dlTextureShader.selection = iTextureShader_1 
				)
				else
				(
					iRgbOperation_1 = dlRgbOperation.selection
					iRgbBlendSource_1 = dlRgbBlendSource.selection
					iRgbArg0_1 = dlRgbArg0.selection
					iRgbArg1_1 = dlRgbArg1.selection
					iRgbArg2_1 = dlRgbArg2.selection
					iRgbArg0Operand_1 = dlRgbArg0Operand.selection
					iRgbArg1Operand_1 = dlRgbArg1Operand.selection
					iRgbArg2Operand_1 = dlRgbArg2Operand.selection
					iAlphaOperation_1 = dlAlphaOperation.selection
					iAlphaBlendSource_1 = dlAlphaBlendSource.selection
					iAlphaArg0_1 = dlAlphaArg0.selection
					iAlphaArg1_1 = dlAlphaArg1.selection
					iAlphaArg2_1 = dlAlphaArg2.selection
					iAlphaArg0Operand_1 = dlAlphaArg0Operand.selection
					iAlphaArg1Operand_1 = dlAlphaArg1Operand.selection
					iAlphaArg2Operand_1 = dlAlphaArg2Operand.selection
					iTextureShader_1 = dlTextureShader.selection
				)

				cpConstant.enabled = bEnableSlot_1 and bNormalShader
				dlRgbOperation.enabled = bEnableSlot_1 and bNormalShader
				dlRgbBlendSource.enabled = (bEnableSlot_1 == true) and (iRgbOperation_1 == 5) and bNormalShader
				dlRgbArg0.enabled = bEnableSlot_1 and bNormalShader
				dlRgbArg1.enabled = bEnableSlot_1 and bNormalShader
				dlRgbArg2.enabled = bEnableSlot_1 and bNormalShader and (iRgbOperation_1 == 6)
				dlRgbArg0Operand.enabled = bEnableSlot_1 and bNormalShader
				dlRgbArg1Operand.enabled = bEnableSlot_1 and bNormalShader
				dlRgbArg2Operand.enabled = dlRgbArg2.enabled
				dlAlphaOperation.enabled = bEnableSlot_1 and bNormalShader
				dlAlphaBlendSource.enabled = (bEnableSlot_1 == true) and (iAlphaOperation_1 == 5) and bNormalShader
				dlAlphaArg0.enabled = bEnableSlot_1 and bNormalShader
				dlAlphaArg1.enabled = bEnableSlot_1 and bNormalShader
				dlAlphaArg2.enabled = bEnableSlot_1 and bNormalShader and (iAlphaOperation_1 == 6)
				dlAlphaArg0Operand.enabled = bEnableSlot_1 and bNormalShader
				dlAlphaArg1Operand.enabled = bEnableSlot_1 and bNormalShader
				dlAlphaArg2Operand.enabled = dlAlphaArg2.enabled
				dlTextureShader.enabled = bEnableSlot_1 and bNormalShader
				sdConstantAlpha.enabled = bEnableSlot_1 and bNormalShader
			)
		)

		on NelMultitextureSlot1 open do
		(
			updateUI true
		)

		on dlRgbOperation selected newState do
		(
			updateUI false
		)

		on dlAlphaOperation selected newState do
		(
			updateUI false
		)

		on dlRgbArg0Operand selected newState do
		(
			updateUI false
		)

		on dlRgbArg1Operand selected newState do
		(
			updateUI false
		)

		on dlRgbArg2Operand selected newState do
		(
			updateUI false
		)

		on dlAlphaArg0Operand selected newState do
		(
			updateUI false
		)

		on dlAlphaArg1Operand selected newState do
		(
			updateUI false
		)

		on dlAlphaArg2Operand selected newState do
		(
			updateUI false
		)

		on dlMappingCoordinates selected newState do
		(
			updateUI false
		)

		on dlTextureShader selected newState do
		(
			updateUI false
		)

		on dlRgbBlendSource selected newState do
		(
			updateUI false
		)

		on dlRgbArg0 selected newState do
		(
			updateUI false
		)

		on dlRgbArg1 selected newState do
		(
			updateUI false
		)

		on dlRgbArg2 selected newState do
		(
			updateUI false
		)

		on dlAlphaBlendSource selected newState do
		(
			updateUI false
		)

		on dlAlphaArg0 selected newState do
		(
			updateUI false
		)

		on dlAlphaArg1 selected newState do
		(
			updateUI false
		)

		on dlAlphaArg2 selected newState do
		(
			updateUI false
		)
	)
    
	rollout NelMultitextureSlot2 "NeL Slot 2" rolledUp:true
    (
		group "RGB"
		(
			dropdownlist dlRgbOperation		"Operation" items:#( "Arg0", "Arg0*Arg1", "Arg0+Arg1", "Arg0+Arg1-0.5", "Alpha Blend", "Arg0*Arg1+Arg2" ) across:2
			dropdownlist dlRgbBlendSource	"Blend Source" items:#( "Texture", "Previous", "Diffuse", "Constant" )
			dropdownlist dlRgbArg0			"Arg0" items:#( "Texture", "Previous", "Diffuse", "Constant", "Texture" ) across:6
			dropdownlist dlRgbArg0Operand	"Oper0" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
			dropdownlist dlRgbArg1			"Arg1" items:#( "Previous", "Diffuse", "Constant", "Texture" )
			dropdownlist dlRgbArg1Operand	"Oper1" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
			dropdownlist dlRgbArg2			"Arg2" items:#( "Previous", "Diffuse", "Constant", "Texture" )
			dropdownlist dlRgbArg2Operand	"Oper2" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
		)
		group "Alpha"
		(
			dropdownlist dlAlphaOperation	"Operation" items:#( "Arg0", "Arg0*Arg1", "Arg0+Arg1", "Arg0+Arg1-0.5", "Alpha Blend", "Arg0*Arg1+Arg2" ) across:2
			dropdownlist dlAlphaBlendSource	"Blend Source" items:#( "Texture", "Previous", "Diffuse", "Constant" )
			dropdownlist dlAlphaArg0		"Arg0" items:#( "Texture", "Previous", "Diffuse", "Constant" ) across:6
			dropdownlist dlAlphaArg0Operand	"Oper0" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
			dropdownlist dlAlphaArg1		"Arg1" items:#( "Previous", "Diffuse", "Constant", "Texture" )
			dropdownlist dlAlphaArg1Operand	"Oper1" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
			dropdownlist dlAlphaArg2		"Arg2" items:#( "Previous", "Diffuse", "Constant", "Texture" )
			dropdownlist dlAlphaArg2Operand	"Oper2" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
		)
		group "Extended"
		(
			dropdownlist dlTextureShader "Texture Shader" items:#( "Disable", "TextureOff", "FetchTexture", "PassThrough", "CullFragment", "OffsetTexture", "OffsetTextureScale", "DependentARTexture", "DependentGBTexture", "DP3", "DP3Texture2D", "DP3CubeMap", "DP3ReflectCubeMap", "DP3ConstEyeReflectCubeMap", "DP3DiffuseCubeMap", "DP3DepthReplace" ) width:100 across:2
			ColorPicker cpConstant "Constant Color" align:#right height:40
			Label		ldConstantAlpha "Constant Alpha" align:#left across:2
			Spinner		 sdConstantAlpha ""  range:[0,255,255] type:#integer scale:1 align:#left width:50
		)

		fn updateUI update =
		(
			if (version >= 4) then
			(
				-- update droplist
				if update == true then
				(
					dlRgbOperation.selection = iRgbOperation_2
					dlRgbBlendSource.selection = iRgbBlendSource_2
					dlRgbArg0.selection = iRgbArg0_2
					dlRgbArg1.selection = iRgbArg1_2
					dlRgbArg2.selection = iRgbArg2_2
					dlRgbArg0Operand.selection = iRgbArg0Operand_2
					dlRgbArg1Operand.selection = iRgbArg1Operand_2
					dlRgbArg2Operand.selection = iRgbArg2Operand_2
					dlAlphaOperation.selection = iAlphaOperation_2
					dlAlphaBlendSource.selection = iAlphaBlendSource_2
					dlAlphaArg0.selection = iAlphaArg0_2 
					dlAlphaArg1.selection = iAlphaArg1_2 
					dlAlphaArg2.selection = iAlphaArg2_2 
					dlAlphaArg0Operand.selection = iAlphaArg0Operand_2
					dlAlphaArg1Operand.selection = iAlphaArg1Operand_2
					dlAlphaArg2Operand.selection = iAlphaArg2Operand_2
					dlTextureShader.selection = iTextureShader_2 
				)
				else
				(
					iRgbOperation_2 = dlRgbOperation.selection
					iRgbBlendSource_2 = dlRgbBlendSource.selection
					iRgbArg0_2 = dlRgbArg0.selection
					iRgbArg1_2 = dlRgbArg1.selection
					iRgbArg2_2 = dlRgbArg2.selection
					iRgbArg0Operand_2 = dlRgbArg0Operand.selection
					iRgbArg1Operand_2 = dlRgbArg1Operand.selection
					iRgbArg2Operand_2 = dlRgbArg2Operand.selection
					iAlphaOperation_2 = dlAlphaOperation.selection
					iAlphaBlendSource_2 = dlAlphaBlendSource.selection
					iAlphaArg0_2 = dlAlphaArg0.selection
					iAlphaArg1_2 = dlAlphaArg1.selection
					iAlphaArg2_2 = dlAlphaArg2.selection
					iAlphaArg0Operand_2 = dlAlphaArg0Operand.selection
					iAlphaArg1Operand_2 = dlAlphaArg1Operand.selection
					iAlphaArg2Operand_2 = dlAlphaArg2Operand.selection
					iTextureShader_2 = dlTextureShader.selection
				)

				cpConstant.enabled = bEnableSlot_2 and bNormalShader
				dlRgbOperation.enabled = bEnableSlot_2 and bNormalShader
				dlRgbBlendSource.enabled = (bEnableSlot_2 == true) and (iRgbOperation_2 == 5) and bNormalShader
				dlRgbArg0.enabled = bEnableSlot_2 and bNormalShader
				dlRgbArg1.enabled = bEnableSlot_2 and bNormalShader
				dlRgbArg2.enabled = bEnableSlot_2 and bNormalShader and (iRgbOperation_2 == 6)
				dlRgbArg0Operand.enabled = bEnableSlot_2 and bNormalShader
				dlRgbArg1Operand.enabled = bEnableSlot_2 and bNormalShader
				dlRgbArg2Operand.enabled = dlRgbArg2.enabled
				dlAlphaOperation.enabled = bEnableSlot_2 and bNormalShader
				dlAlphaBlendSource.enabled = (bEnableSlot_2 == true) and (iAlphaOperation_2 == 5) and bNormalShader
				dlAlphaArg0.enabled = bEnableSlot_2 and bNormalShader
				dlAlphaArg1.enabled = bEnableSlot_2 and bNormalShader
				dlAlphaArg2.enabled = bEnableSlot_2 and bNormalShader and (iAlphaOperation_2 == 6)
				dlAlphaArg0Operand.enabled = bEnableSlot_2 and bNormalShader
				dlAlphaArg1Operand.enabled = bEnableSlot_2 and bNormalShader
				dlAlphaArg2Operand.enabled = dlAlphaArg2.enabled
				dlTextureShader.enabled = bEnableSlot_2 and bNormalShader
				sdConstantAlpha.enabled = bEnableSlot_2 and bNormalShader
			)
		)

		on NelMultitextureSlot2 open do
		(
			updateUI true
		)

		on dlRgbOperation selected newState do
		(
			updateUI false
		)

		on dlAlphaOperation selected newState do
		(
			updateUI false
		)

		on dlRgbArg0Operand selected newState do
		(
			updateUI false
		)

		on dlRgbArg1Operand selected newState do
		(
			updateUI false
		)

		on dlRgbArg2Operand selected newState do
		(
			updateUI false
		)

		on dlAlphaArg0Operand selected newState do
		(
			updateUI false
		)

		on dlAlphaArg1Operand selected newState do
		(
			updateUI false
		)

		on dlAlphaArg2Operand selected newState do
		(
			updateUI false
		)

		on dlMappingCoordinates selected newState do
		(
			updateUI false
		)

		on dlTextureShader selected newState do
		(
			updateUI false
		)

		on dlRgbBlendSource selected newState do
		(
			updateUI false
		)

		on dlRgbArg0 selected newState do
		(
			updateUI false
		)

		on dlRgbArg1 selected newState do
		(
			updateUI false
		)

		on dlRgbArg2 selected newState do
		(
			updateUI false
		)

		on dlAlphaBlendSource selected newState do
		(
			updateUI false
		)

		on dlAlphaArg0 selected newState do
		(
			updateUI false
		)

		on dlAlphaArg1 selected newState do
		(
			updateUI false
		)

		on dlAlphaArg2 selected newState do
		(
			updateUI false
		)
	)

	rollout NelMultitextureSlot3 "NeL Slot 3" rolledUp:true
    (
		group "RGB"
		(
			dropdownlist dlRgbOperation		"Operation" items:#( "Arg0", "Arg0*Arg1", "Arg0+Arg1", "Arg0+Arg1-0.5", "Alpha Blend", "Arg0*Arg1+Arg2" ) across:2
			dropdownlist dlRgbBlendSource	"Blend Source" items:#( "Texture", "Previous", "Diffuse", "Constant" )
			dropdownlist dlRgbArg0			"Arg0" items:#( "Texture", "Previous", "Diffuse", "Constant" ) across:6
			dropdownlist dlRgbArg0Operand	"Oper0" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
			dropdownlist dlRgbArg1			"Arg1" items:#( "Previous", "Diffuse", "Constant", "Texture" )
			dropdownlist dlRgbArg1Operand	"Oper1" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
			dropdownlist dlRgbArg2			"Arg2" items:#( "Previous", "Diffuse", "Constant", "Texture" )
			dropdownlist dlRgbArg2Operand	"Oper2" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
		)
		group "Alpha"
		(
			dropdownlist dlAlphaOperation	"Operation" items:#( "Arg0", "Arg0*Arg1", "Arg0+Arg1", "Arg0+Arg1-0.5", "Alpha Blend", "Arg0*Arg1+Arg2" ) across:2
			dropdownlist dlAlphaBlendSource	"Blend Source" items:#( "Texture", "Previous", "Diffuse", "Constant" )
			dropdownlist dlAlphaArg0		"Arg0" items:#( "Texture", "Previous", "Diffuse", "Constant", "Texture" ) across:6
			dropdownlist dlAlphaArg0Operand	"Oper0" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
			dropdownlist dlAlphaArg1		"Arg1" items:#( "Previous", "Diffuse", "Constant", "Texture" )
			dropdownlist dlAlphaArg1Operand	"Oper1" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
			dropdownlist dlAlphaArg2		"Arg2" items:#( "Previous", "Diffuse", "Constant", "Texture" )
			dropdownlist dlAlphaArg2Operand	"Oper2" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
		)
		group "Extended"
		(
			dropdownlist dlTextureShader "Texture Shader" items:#( "Disable", "TextureOff", "FetchTexture", "PassThrough", "CullFragment", "OffsetTexture", "OffsetTextureScale", "DependentARTexture", "DependentGBTexture", "DP3", "DP3Texture2D", "DP3CubeMap", "DP3ReflectCubeMap", "DP3ConstEyeReflectCubeMap", "DP3DiffuseCubeMap", "DP3DepthReplace" ) width:100 across:2
			ColorPicker cpConstant "Constant Color" align:#right height:40
			Label		ldConstantAlpha "Constant Alpha" align:#left across:2
			Spinner		 sdConstantAlpha ""  range:[0,255,255] type:#integer scale:1 align:#left width:50
		)

		fn updateUI update =
		(
			if (version >= 4) then
			(
				-- update droplist
				if update == true then
				(
					dlRgbOperation.selection = iRgbOperation_3
					dlRgbBlendSource.selection = iRgbBlendSource_3
					dlRgbArg0.selection = iRgbArg0_3
					dlRgbArg1.selection = iRgbArg1_3
					dlRgbArg2.selection = iRgbArg2_3
					dlRgbArg0Operand.selection = iRgbArg0Operand_3
					dlRgbArg1Operand.selection = iRgbArg1Operand_3
					dlRgbArg2Operand.selection = iRgbArg2Operand_3
					dlAlphaOperation.selection = iAlphaOperation_3
					dlAlphaBlendSource.selection = iAlphaBlendSource_3
					dlAlphaArg0.selection = iAlphaArg0_3 
					dlAlphaArg1.selection = iAlphaArg1_3 
					dlAlphaArg2.selection = iAlphaArg2_3 
					dlAlphaArg0Operand.selection = iAlphaArg0Operand_3
					dlAlphaArg1Operand.selection = iAlphaArg1Operand_3
					dlAlphaArg2Operand.selection = iAlphaArg2Operand_3
					dlTextureShader.selection = iTextureShader_3 
				)
				else
				(
					iRgbOperation_3 = dlRgbOperation.selection
					iRgbBlendSource_3 = dlRgbBlendSource.selection
					iRgbArg0_3 = dlRgbArg0.selection
					iRgbArg1_3 = dlRgbArg1.selection
					iRgbArg2_3 = dlRgbArg2.selection
					iRgbArg0Operand_3 = dlRgbArg0Operand.selection
					iRgbArg1Operand_3 = dlRgbArg1Operand.selection
					iRgbArg2Operand_3 = dlRgbArg2Operand.selection
					iAlphaOperation_3 = dlAlphaOperation.selection
					iAlphaBlendSource_3 = dlAlphaBlendSource.selection
					iAlphaArg0_3 = dlAlphaArg0.selection
					iAlphaArg1_3 = dlAlphaArg1.selection
					iAlphaArg2_3 = dlAlphaArg2.selection
					iAlphaArg0Operand_3 = dlAlphaArg0Operand.selection
					iAlphaArg1Operand_3 = dlAlphaArg1Operand.selection
					iAlphaArg2Operand_3 = dlAlphaArg2Operand.selection
					iTextureShader_3 = dlTextureShader.selection
				)

				cpConstant.enabled = bEnableSlot_3 and bNormalShader
				dlRgbOperation.enabled = bEnableSlot_3 and bNormalShader
				dlRgbBlendSource.enabled = (bEnableSlot_3 == true) and (iRgbOperation_3 == 5) and bNormalShader
				dlRgbArg0.enabled = bEnableSlot_3 and bNormalShader
				dlRgbArg1.enabled = bEnableSlot_3 and bNormalShader
				dlRgbArg2.enabled = bEnableSlot_3 and bNormalShader and (iRgbOperation_3 == 6)
				dlRgbArg0Operand.enabled = bEnableSlot_3 and bNormalShader
				dlRgbArg1Operand.enabled = bEnableSlot_3 and bNormalShader
				dlRgbArg2Operand.enabled = dlRgbArg2.enabled
				dlAlphaOperation.enabled = bEnableSlot_3 and bNormalShader
				dlAlphaBlendSource.enabled = (bEnableSlot_3 == true) and (iAlphaOperation_3 == 5) and bNormalShader
				dlAlphaArg0.enabled = bEnableSlot_3 and bNormalShader
				dlAlphaArg1.enabled = bEnableSlot_3 and bNormalShader
				dlAlphaArg2.enabled = bEnableSlot_3 and bNormalShader and (iAlphaOperation_3 == 6)
				dlAlphaArg0Operand.enabled = bEnableSlot_3 and bNormalShader
				dlAlphaArg1Operand.enabled = bEnableSlot_3 and bNormalShader
				dlAlphaArg2Operand.enabled = dlAlphaArg2.enabled
				dlTextureShader.enabled = bEnableSlot_3 and bNormalShader
				sdConstantAlpha.enabled = bEnableSlot_3 and bNormalShader
			)
		)

		on NelMultitextureSlot3 open do
		(
			updateUI true
		)

		on dlRgbOperation selected newState do
		(
			updateUI false
		)

		on dlAlphaOperation selected newState do
		(
			updateUI false
		)

		on dlRgbArg0Operand selected newState do
		(
			updateUI false
		)

		on dlRgbArg1Operand selected newState do
		(
			updateUI false
		)

		on dlRgbArg2Operand selected newState do
		(
			updateUI false
		)

		on dlAlphaArg0Operand selected newState do
		(
			updateUI false
		)

		on dlAlphaArg1Operand selected newState do
		(
			updateUI false
		)

		on dlAlphaArg2Operand selected newState do
		(
			updateUI false
		)

		on dlMappingCoordinates selected newState do
		(
			updateUI false
		)

		on dlTextureShader selected newState do
		(
			updateUI false
		)

		on dlRgbBlendSource selected newState do
		(
			updateUI false
		)

		on dlRgbArg0 selected newState do
		(
			updateUI false
		)

		on dlRgbArg1 selected newState do
		(
			updateUI false
		)

		on dlRgbArg2 selected newState do
		(
			updateUI false
		)

		on dlAlphaBlendSource selected newState do
		(
			updateUI false
		)

		on dlAlphaArg0 selected newState do
		(
			updateUI false
		)

		on dlAlphaArg1 selected newState do
		(
			updateUI false
		)

		on dlAlphaArg2 selected newState do
		(
			updateUI false
		)
	)

	rollout NelMultitextureSlot4 "NeL Slot 4" rolledUp:true
    (
		group "RGB"
		(
			dropdownlist dlRgbOperation		"Operation" items:#( "Arg0", "Arg0*Arg1", "Arg0+Arg1", "Arg0+Arg1-0.5", "Alpha Blend", "Arg0*Arg1+Arg2" ) across:2
			dropdownlist dlRgbBlendSource	"Blend Source" items:#( "Texture", "Previous", "Diffuse", "Constant" )
			dropdownlist dlRgbArg0			"Arg0" items:#( "Texture", "Previous", "Diffuse", "Constant", "Texture" ) across:6
			dropdownlist dlRgbArg0Operand	"Oper0" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
			dropdownlist dlRgbArg1			"Arg1" items:#( "Previous", "Diffuse", "Constant", "Texture" )
			dropdownlist dlRgbArg1Operand	"Oper1" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
			dropdownlist dlRgbArg2			"Arg2" items:#( "Previous", "Diffuse", "Constant", "Texture" )
			dropdownlist dlRgbArg2Operand	"Oper2" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
		)
		group "Alpha"
		(
			dropdownlist dlAlphaOperation	"Operation" items:#( "Arg0", "Arg0*Arg1", "Arg0+Arg1", "Arg0+Arg1-0.5", "Alpha Blend", "Arg0*Arg1+Arg2" ) across:2
			dropdownlist dlAlphaBlendSource	"Blend Source" items:#( "Texture", "Previous", "Diffuse", "Constant" )
			dropdownlist dlAlphaArg0		"Arg0" items:#( "Texture", "Previous", "Diffuse", "Constant", "Texture" ) across:6
			dropdownlist dlAlphaArg0Operand	"Oper0" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
			dropdownlist dlAlphaArg1		"Arg1" items:#( "Previous", "Diffuse", "Constant", "Texture" )
			dropdownlist dlAlphaArg1Operand	"Oper1" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
			dropdownlist dlAlphaArg2		"Arg2" items:#( "Previous", "Diffuse", "Constant", "Texture" )
			dropdownlist dlAlphaArg2Operand	"Oper2" items:#( "RGB", "Inv RGB", "Alpha", "Inv Alpha" )
		)
		group "Extended"
		(
			dropdownlist dlTextureShader "Texture Shader" items:#( "Disable", "TextureOff", "FetchTexture", "PassThrough", "CullFragment", "OffsetTexture", "OffsetTextureScale", "DependentARTexture", "DependentGBTexture", "DP3", "DP3Texture2D", "DP3CubeMap", "DP3ReflectCubeMap", "DP3ConstEyeReflectCubeMap", "DP3DiffuseCubeMap", "DP3DepthReplace" ) width:100 across:2
			ColorPicker cpConstant "Constant Color" align:#right height:40
			Label		ldConstantAlpha "Constant Alpha" align:#left across:2
			Spinner		 sdConstantAlpha ""  range:[0,255,255] type:#integer scale:1 align:#left width:50
		)

		fn updateUI update =
		(
			if (version >= 4) then
			(
				-- update droplist
				if update == true then
				(
					dlRgbOperation.selection = iRgbOperation_4
					dlRgbBlendSource.selection = iRgbBlendSource_4
					dlRgbArg0.selection = iRgbArg0_4
					dlRgbArg1.selection = iRgbArg1_4
					dlRgbArg2.selection = iRgbArg1_4
					dlRgbArg0Operand.selection = iRgbArg0Operand_4
					dlRgbArg1Operand.selection = iRgbArg1Operand_4
					dlRgbArg2Operand.selection = iRgbArg1Operand_4
					dlAlphaOperation.selection = iAlphaOperation_4
					dlAlphaBlendSource.selection = iAlphaBlendSource_4
					dlAlphaArg0.selection = iAlphaArg0_4 
					dlAlphaArg1.selection = iAlphaArg1_4 
					dlAlphaArg2.selection = iAlphaArg1_4 
					dlAlphaArg0Operand.selection = iAlphaArg0Operand_4
					dlAlphaArg1Operand.selection = iAlphaArg1Operand_4
					dlAlphaArg2Operand.selection = iAlphaArg1Operand_4
					dlTextureShader.selection = iTextureShader_4 
				)
				else
				(
					iRgbOperation_4 = dlRgbOperation.selection
					iRgbBlendSource_4 = dlRgbBlendSource.selection
					iRgbArg0_4 = dlRgbArg0.selection
					iRgbArg1_4 = dlRgbArg1.selection
					iRgbArg2_4 = dlRgbArg2.selection
					iRgbArg0Operand_4 = dlRgbArg0Operand.selection
					iRgbArg1Operand_4 = dlRgbArg1Operand.selection
					iRgbArg2Operand_4 = dlRgbArg2Operand.selection
					iAlphaOperation_4 = dlAlphaOperation.selection
					iAlphaBlendSource_4 = dlAlphaBlendSource.selection
					iAlphaArg0_4 = dlAlphaArg0.selection
					iAlphaArg1_4 = dlAlphaArg1.selection
					iAlphaArg2_4 = dlAlphaArg2.selection
					iAlphaArg0Operand_4 = dlAlphaArg0Operand.selection
					iAlphaArg1Operand_4 = dlAlphaArg1Operand.selection
					iAlphaArg2Operand_4 = dlAlphaArg2Operand.selection
					iTextureShader_4 = dlTextureShader.selection
				)

				cpConstant.enabled = bEnableSlot_4 and bNormalShader
				dlRgbOperation.enabled = bEnableSlot_4 and bNormalShader
				dlRgbBlendSource.enabled = (bEnableSlot_4 == true) and (iRgbOperation_4 == 5) and bNormalShader
				dlRgbArg0.enabled = bEnableSlot_4 and bNormalShader
				dlRgbArg1.enabled = bEnableSlot_4 and bNormalShader
				dlRgbArg2.enabled = bEnableSlot_4 and bNormalShader and (iRgbOperation_4 == 6)
				dlRgbArg0Operand.enabled = bEnableSlot_4 and bNormalShader
				dlRgbArg1Operand.enabled = bEnableSlot_4 and bNormalShader
				dlRgbArg2Operand.enabled = dlRgbArg2.enabled
				dlAlphaOperation.enabled = bEnableSlot_4 and bNormalShader
				dlAlphaBlendSource.enabled = (bEnableSlot_4 == true) and (iAlphaOperation_4 == 5) and bNormalShader
				dlAlphaArg0.enabled = bEnableSlot_4 and bNormalShader
				dlAlphaArg1.enabled = bEnableSlot_4 and bNormalShader
				dlAlphaArg2.enabled = bEnableSlot_4 and bNormalShader and (iAlphaOperation_4 == 6)
				dlAlphaArg0Operand.enabled = bEnableSlot_4 and bNormalShader
				dlAlphaArg1Operand.enabled = bEnableSlot_4 and bNormalShader
				dlAlphaArg2Operand.enabled = dlAlphaArg2.enabled
				dlTextureShader.enabled = bEnableSlot_4 and bNormalShader
				sdConstantAlpha.enabled = bEnableSlot_4 and bNormalShader
			)
		)

		on NelMultitextureSlot4 open do
		(
			updateUI true
		)

		on dlRgbOperation selected newState do
		(
			updateUI false
		)

		on dlAlphaOperation selected newState do
		(
			updateUI false
		)

		on dlRgbArg0Operand selected newState do
		(
			updateUI false
		)

		on dlRgbArg1Operand selected newState do
		(
			updateUI false
		)

		on dlRgbArg2Operand selected newState do
		(
			updateUI false
		)

		on dlAlphaArg0Operand selected newState do
		(
			updateUI false
		)

		on dlAlphaArg1Operand selected newState do
		(
			updateUI false
		)

		on dlAlphaArg2Operand selected newState do
		(
			updateUI false
		)

		on dlMappingCoordinates selected newState do
		(
			updateUI false
		)

		on dlTextureShader selected newState do
		(
			updateUI false
		)

		on dlRgbBlendSource selected newState do
		(
			updateUI false
		)

		on dlRgbArg0 selected newState do
		(
			updateUI false
		)

		on dlRgbArg1 selected newState do
		(
			updateUI false
		)

		on dlRgbArg2 selected newState do
		(
			updateUI false
		)

		on dlAlphaBlendSource selected newState do
		(
			updateUI false
		)

		on dlAlphaArg0 selected newState do
		(
			updateUI false
		)

		on dlAlphaArg1 selected newState do
		(
			updateUI false
		)

		on dlAlphaArg2 selected newState do
		(
			updateUI false
		)
	)

	fn loadShader shader =
	(
		-- Disable all slot
		bEnableSlot_1 = false
		bEnableSlot_2 = false
		bEnableSlot_3 = false
		bEnableSlot_4 = false
		bEnableSlot_5 = false
		bEnableSlot_6 = false
		bEnableSlot_7 = false
		bEnableSlot_8 = false

		if shader == ShaderSingleTexture then
		(
			-- Single texture
			bEnableSlot_1 = true
			iRgbOperation_1	= OptorModulate
			iRgbArg0_1	= Src0Texture
			iRgbArg1_1	= Src1Diffuse
			iRgbArg0Operand_1	= OprdRGB
			iRgbArg1Operand_1	= OprdRGB
			iAlphaOperation_1 = OptorModulate
			iAlphaArg0_1 = Src0Texture
			iAlphaArg1_1 = Src1Diffuse
			iAlphaArg0Operand_1	= OprdAlpha
			iAlphaArg1Operand_1	= OprdAlpha
			iTextureShader_1 = TextureShaderDisable
		)
		else if shader == ShaderAddTwoTextures then
		(
			-- Two texture texture

			bEnableSlot_1 = true
			iRgbOperation_1	= OptorModulate
			iRgbArg0_1	= Src0Texture
			iRgbArg1_1	= Src1Diffuse
			iRgbArg0Operand_1	= OprdRGB
			iRgbArg1Operand_1	= OprdRGB
			iAlphaOperation_1 = OptorModulate
			iAlphaArg0_1 = Src0Texture
			iAlphaArg1_1 = Src1Diffuse
			iAlphaArg0Operand_1	= OprdAlpha
			iAlphaArg1Operand_1	= OprdAlpha
			iTextureShader_1 = TextureShaderDisable

			bEnableSlot_2 = true
			iRgbOperation_2	= OptorAdd
			iRgbArg0_2	= Src0Texture
			iRgbArg1_2	= Src1Previous
			iRgbArg0Operand_2	= OprdRGB
			iRgbArg1Operand_2	= OprdRGB
			iAlphaOperation_2 = OptorModulate
			iAlphaArg0_2 = Src0Texture
			iAlphaArg1_2 = Src1Diffuse
			iAlphaArg0Operand_2	= OprdAlpha
			iAlphaArg1Operand_2	= OprdAlpha
			iTextureShader_2 = TextureShaderDisable
		)
		else if shader == ShaderMulTwoTextures then
		(
			-- Two texture texture

			bEnableSlot_1 = true
			iRgbOperation_1	= OptorModulate
			iRgbArg0_1	= Src0Texture
			iRgbArg1_1	= Src1Diffuse
			iRgbArg0Operand_1	= OprdRGB
			iRgbArg1Operand_1	= OprdRGB
			iAlphaOperation_1 = OptorModulate
			iAlphaArg0_1 = Src0Texture
			iAlphaArg1_1 = Src1Diffuse
			iAlphaArg0Operand_1	= OprdAlpha
			iAlphaArg1Operand_1	= OprdAlpha
			iTextureShader_1 = TextureShaderDisable

			bEnableSlot_2 = true
			iRgbOperation_2	= OptorModulate
			iRgbArg0_2	= Src0Texture
			iRgbArg1_2	= Src1Previous
			iRgbArg0Operand_2	= OprdRGB
			iRgbArg1Operand_2	= OprdRGB
			iAlphaOperation_2 = OptorModulate
			iAlphaArg0_2 = Src0Texture
			iAlphaArg1_2 = Src1Diffuse
			iAlphaArg0Operand_2	= OprdAlpha
			iAlphaArg1Operand_2	= OprdAlpha
			iTextureShader_2 = TextureShaderDisable
		)
		else if shader == ShaderBlendTwoTextures then
		(
			-- Two texture texture

			bEnableSlot_1 = true
			iRgbOperation_1	= OptorModulate
			iRgbArg0_1	= Src0Texture
			iRgbArg1_1	= Src1Diffuse
			iRgbArg0Operand_1	= OprdRGB
			iRgbArg1Operand_1	= OprdRGB
			iAlphaOperation_1 = OptorModulate
			iAlphaArg0_1 = Src0Texture
			iAlphaArg1_1 = Src1Diffuse
			iAlphaArg0Operand_1	= OprdAlpha
			iAlphaArg1Operand_1	= OprdAlpha
			iTextureShader_1 = TextureShaderDisable

			bEnableSlot_2 = true
			iRgbOperation_2	= OptorBlend
			iRgbBlendSource_2 = SrcBlendPrevious
			iRgbArg0_2	= Src0Texture
			iRgbArg1_2	= Src1Previous
			iRgbArg0Operand_2	= OprdRGB
			iRgbArg1Operand_2	= OprdRGB
			iAlphaOperation_2 = OptorReplace
			iAlphaArg0_2 = Src0Texture
			iAlphaArg1_2 = Src1Diffuse
			iAlphaArg0Operand_2	= OprdAlpha
			iAlphaArg1Operand_2	= OprdAlpha
			iTextureShader_2 = TextureShaderDisable
		)
		else if shader == ShaderBlendMask then
		(
			-- Two texture texture

			bEnableSlot_1 = true
			iRgbOperation_1	= OptorModulate
			iRgbArg0_1	= Src0Texture
			iRgbArg1_1	= Src1Diffuse
			iRgbArg0Operand_1	= OprdRGB
			iRgbArg1Operand_1	= OprdRGB
			iAlphaOperation_1 = OptorReplace
			iAlphaArg0_1 = Src0Texture
			iAlphaArg0Operand_1	= OprdAlpha
			iTextureShader_1 = TextureShaderDisable

			bEnableSlot_2 = true
			iRgbOperation_2	= OptorReplace
			iRgbArg0_2	= Src0Previous
			iRgbArg0Operand_2	= OprdRGB
			iAlphaOperation_2 = OptorModulate
			iAlphaArg0_2 = Src0Texture
			iAlphaArg1_2 = Src1Diffuse
			iAlphaArg0Operand_2	= OprdAlpha
			iAlphaArg1Operand_2	= OprdAlpha
			iTextureShader_2 = TextureShaderDisable
		)
		else if shader == ShaderMixTwoTextures then
		(
			-- Two texture texture

			bEnableSlot_1 = true
			iRgbOperation_1	= OptorModulate
			iRgbArg0_1	= Src0Texture
			iRgbArg1_1	= Src1Diffuse
			iRgbArg0Operand_1	= OprdRGB
			iRgbArg1Operand_1	= OprdRGB
			iAlphaOperation_1 = OptorModulate
			iAlphaArg0_1 = Src0Texture
			iAlphaArg0_2 = Src1Diffuse
			iAlphaArg0Operand_1	= OprdAlpha
			iAlphaArg1Operand_1	= OprdAlpha
			iTextureShader_1 = TextureShaderDisable

			bEnableSlot_2 = true
			iRgbOperation_2	= OptorBlend
			iRgbBlendSource_2 = SrcBlendConstant
			iRgbArg0_2	= Src0Texture
			iRgbArg1_2	= Src1Previous
			iRgbArg0Operand_2	= OprdRGB
			iAlphaOperation_2	= OptorBlend
			iAlphaBlendSource_2 = SrcBlendConstant
			iAlphaArg0_2 = Src0Texture
			iAlphaArg1_2 = Src1Previous
			iAlphaArg0Operand_2	= OprdAlpha
			iAlphaArg1Operand_2	= OprdAlpha
			iTextureShader_2 = TextureShaderDisable
			iConstantAlpha_2 = 127
		)

		-- Update the user interface
		globalUpdateUI true
	)

	on load do 
	(
		-- Trap Patch
		if version >= 4 and version < 14 then
		(
			if (delegate.DiffuseMap != undefined) and (tTexture_1 == undefined) then
			(
				tTexture_1 = delegate.DiffuseMap
			)
		)
		
		-- Update to version 4 ?
		if updateToVersion4 == true then
		(

			-- Water
			if bWater == true then
			(
				-- Shader Type
				iShaderType = ShaderWater

				tTexture_1 = delegate.ReflectionMap
				tTexture_2 = delegate.RefractionMap
				tTexture_3 = delegate.OpacityMap
				tTexture_4 = delegate.FilterMap
				tTexture_5 = delegate.BumpMap
				tTexture_6 = delegate.DisplacementMap
				tTexture_7 = delegate.DiffuseMap

				bEnableSlot_1 = delegate.ReflectionMapEnable
				bEnableSlot_2 = delegate.RefractionMapEnable
				bEnableSlot_3 = delegate.OpacityMapEnable
				bEnableSlot_4 = delegate.FilterMapEnable
				bEnableSlot_5 = delegate.BumpMapEnable
				bEnableSlot_6 = delegate.DisplacementMapEnable
				bEnableSlot_7 = delegate.DiffuseMapEnable
			)
			else
			(
				-- Simple texture mode
				tTexture_1 = delegate.DiffuseMap
				bEnableSlot_1 = delegate.DiffuseMapEnable

				-- Lightmap ?
				if bLightmap == true then
				(
					-- Shader Type
					iShaderType = ShaderLightmap
				)
				else
				(
					if ((delegate.SpecularMapEnable == true) and (delegate.SpecularMap != undefined)) then
					(
						-- Shader specular
						iShaderType = ShaderSpecular

						-- Specular map
						tTexture_2 = delegate.SpecularMap
						bEnableSlot_2 = true
					)
					else
					(
						-- Shader Type
						iShaderType = ShaderNormal
						loadShader ShaderSingleTexture
					)

					-- Alpha blend ?
					bAlphaBlend = (bAlphaTest == false) and ((delegate.opacity < 100) or (delegate.OpacityMap != undefined) or bAlphaVertex)
					
					-- Alpha operator
					if (delegate.opacityType == 0) then
					(
						-- Blend
						iBlendSrcFunc = BlendSrcAlpha
						iBlendDestFunc = BlendInvSrcAlpha
					)
					else if (delegate.opacityType == 2) then
					(
						-- Additive
						iBlendSrcFunc = BlendOne
						iBlendDestFunc = BlendOne
					)
				)
			)
		)
		
		updateToVersion4 = false
		
		if updateToVersion14 == true then
		(			
			bTwoSided = delegate.twoSided
			cAmbient = delegate.ambient
			cDiffuse = delegate.diffuse
			pOpacity = delegate.opacity
			cSpecular = delegate.specular
			pSpecularLevel = delegate.specularLevel
			pGlossiness = delegate.glossiness
			cSelfIllumColor = delegate.selfIllumColor
			pSelfIllumAmount = delegate.selfIllumAmount
			bUseSelfIllumColor = delegate.useSelfIllumColor
		)
		
		updateToVersion14 = false
		
	)

	on create do 
	(
		-- Single shader
		loadShader ShaderSingleTexture
	)

	on update do 
	(
		-- Initialise V3 fields.
		if (version < 3) and (version >= 1) then
		(
			-- No color vertex
			bColorVertex = false

			if bLightMap == true then
				bColorVertex = true
			if bUnlighted == true then
				bColorVertex = true
			
			-- No alpha vertex
			bAlphaVertex = false
			iAlphaVertexChannel	= 0
		)

		-- Initialise V4 fields.
		if (version < 4) and (version >= 1) then
		(
			updateToVersion4 = true
		)
		
		if (version < 14) and (version >= 1) then
		(
			updateToVersion14 = true
		)
	) 

	fn globalUpdateUI update =
	(
		nelBasicParameters.updateUI update
		NelParams.updateUI update
		NelTexture.updateUI update
		NelMultitextureSlot1.updateUI update
		NelMultitextureSlot2.updateUI update
		NelMultitextureSlot3.updateUI update
		NelMultitextureSlot4.updateUI update
	)

	fn updateTextureName =
	(
		-- Set the name
		if (tTexture_1 != undefined) then
		(
			if (classof tTexture_1) == Bitmaptexture then
			(
				NelTexture.mbTexture_1.text = (tTexture_1.name+" ("+(filenameFromPath (tTexture_1.filename))+")")
			)
			else
			(
				NelTexture.mbTexture_1.text = (tTexture_1.name+" ("+((classof tTexture_1) as string)+")")
			)
		)
		else
		(
			NelTexture.mbTexture_1.text = "None"
		)
		if (tTexture_2 != undefined) then
		(
			if (classof tTexture_2) == Bitmaptexture then
			(
				NelTexture.mbTexture_2.text = (tTexture_2.name+" ("+(filenameFromPath (tTexture_2.filename))+")")
			)
			else
			(
				NelTexture.mbTexture_2.text = (tTexture_2.name+" ("+((classof tTexture_2) as string)+")")
			)
		)
		else
		(
			if (iShaderType == ShaderUserColor) and (tTexture_1 != undefined) and ((classof tTexture_1) == Bitmaptexture) then
			(
				NelTexture.mbTexture_2.text = (getFilenameFile (filenameFromPath (tTexture_1.filename))) + "_usercolor" + (getFilenameType (filenameFromPath (tTexture_1.filename)))
			)
			else
			(
				NelTexture.mbTexture_2.text = "None"
			)
		)
		if (tTexture_3 != undefined) then
		(
			if (classof tTexture_3) == Bitmaptexture then
			(
				NelTexture.mbTexture_3.text = (tTexture_3.name+" ("+(filenameFromPath (tTexture_3.filename))+")")
			)
			else
			(
				NelTexture.mbTexture_3.text = (tTexture_3.name+" ("+((classof tTexture_3) as string)+")")
			)
		)
		else
		(
			NelTexture.mbTexture_3.text = "None"
		)
		if (tTexture_4 != undefined) then
		(
			if (classof tTexture_4) == Bitmaptexture then
			(
				NelTexture.mbTexture_4.text = (tTexture_4.name+" ("+(filenameFromPath (tTexture_4.filename))+")")
			)
			else
			(
				NelTexture.mbTexture_4.text = (tTexture_4.name+" ("+((classof tTexture_4) as string)+")")
			)
		)
		else
		(
			NelTexture.mbTexture_4.text = "None"
		)
		if (tTexture_5 != undefined) then
		(
			if (classof tTexture_5) == Bitmaptexture then
			(
				NelTexture.mbTexture_5.text = (tTexture_5.name+" ("+(filenameFromPath (tTexture_5.filename))+")")
			)
			else
			(
				NelTexture.mbTexture_5.text = (tTexture_5.name+" ("+((classof tTexture_5) as string)+")")
			)
		)
		else
		(
			NelTexture.mbTexture_5.text = "None"
		)
		if (tTexture_6 != undefined) then
		(
			if (classof tTexture_6) == Bitmaptexture then
			(
				NelTexture.mbTexture_6.text = (tTexture_6.name+" ("+(filenameFromPath (tTexture_6.filename))+")")
			)
			else
			(
				NelTexture.mbTexture_6.text = (tTexture_6.name+" ("+((classof tTexture_6) as string)+")")
			)
		)
		else
		(
			NelTexture.mbTexture_6.text = "None"
		)
		if (tTexture_7 != undefined) then
		(
			if (classof tTexture_7) == Bitmaptexture then
			(
				NelTexture.mbTexture_7.text = (tTexture_7.name+" ("+(filenameFromPath (tTexture_7.filename))+")")
			)
			else
			(
				NelTexture.mbTexture_7.text = (tTexture_7.name+" ("+((classof tTexture_7) as string)+")")
			)
		)
		else
		(
			NelTexture.mbTexture_7.text = "None"
		)
		if (tTexture_8 != undefined) then
		(
			if (classof tTexture_8) == Bitmaptexture then
			(
				NelTexture.mbTexture_8.text = (tTexture_8.name+" ("+(filenameFromPath (tTexture_8.filename))+")")
			)
			else
			(
				NelTexture.mbTexture_8.text = (tTexture_8.name+" ("+((classof tTexture_8) as string)+")")
			)
		)
		else
		(
			NelTexture.mbTexture_8.text = "None"
		)
	)

)
