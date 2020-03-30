rollout nelmatconv_rollout "nel mat converter"
(
group "converter"
	(
	checkbox selectiononly "Selected objects only"  -- checked:true
	checkbox setLightMapParam_on "Set all LightMap [on] ?" checked:true
--	checkbox setbasetodiffuse "Set base color = diffuse"  checked:true
	button goforall "GO GO GO !" 

	label object_processed_label "--------"
	Label textnull01 "You can force LightMap [on]"
	Label textnull02 "with existing material"

	button unnelconv "Un-NeLificator !" 

	
	)
group "Maps"
	(
	checkbox selectiononlyforshowmap "Selected objects only"   checked:true
	button ShowMaps "Show Maps"
	button HideMaps "Hide Maps"
	)

Fn convertsinglemat thisoldmat =
(
	print "converting : "
	tempmat = NelMaterial()
	for i in getpropnames tempmat.standard do (setproperty tempmat.standard i  (getproperty thisoldmat i))
	tempmat.name = thisoldmat.name

	-- Convert the material
	if classof thisoldmat == Standard then
	(
		-- Simple texture mode
		tempmat.tTexture_1 = thisoldmat.DiffuseMap
		tempmat.bEnableSlot_1 = thisoldmat.DiffuseMapEnable

		if ((thisoldmat.SpecularMapEnable == true) and (thisoldmat.SpecularMap != undefined)) then
		(
			-- Shader specular
			tempmat.iShaderType = 5 -- ShaderSpecular

			-- Specular map
			tempmat.tTexture_2 = thisoldmat.SpecularMap
			tempmat.bEnableSlot_2 = true
		)

		-- Alpha blend ?
		tempmat.bAlphaBlend = (thisoldmat.opacity < 100) or (thisoldmat.OpacityMap != undefined)
		
		-- Alpha operator
		if (thisoldmat.opacityType == 0) then
		(
			-- Blend
			tempmat.iBlendSrcFunc = 3 -- BlendSrcAlpha
			tempmat.iBlendDestFunc = 4 -- BlendInvSrcAlpha
		)
		else if (thisoldmat.opacityType == 2) then
		(
			-- Additive
			tempmat.iBlendSrcFunc = 1 -- BlendOne
			tempmat.iBlendDestFunc = 1 -- BlendOne
		)
	)

	return tempmat
)


Fn process_obj obj = 
	(
--	if obj.material == undefined then messagebox "le script va crasher : \nun object de la scene n'a pas de materiau !"
	print obj.name
	print obj.material
	currentmat = obj.material
	print currentmat
	if classof currentmat==Multimaterial 
	then		-- traiter un multimat
		(
		print "multi-sub material"
		for submatnumber = 1 to currentmat.count do 
			(
			if classof currentmat[submatnumber] != NelMaterial
			then 
				(
				print "This is not a Nel Mat --> Let's convert"
--				messagebox (currentmat.names[submatnumber] as string)
				currentmat[submatnumber] = (convertsinglemat currentmat[submatnumber])
--				messagebox (currentmat.names[submatnumber] as string)
				if currentmat.names[submatnumber] == undefined then currentmat.names[submatnumber] = currentmat[submatnumber].name
				if setLightMapParam_on.checked == true then (currentmat[submatnumber].bLightMap = true)
				)
			else
				(
				print "Already Nel Material : do nothing" 
				if setLightMapParam_on.checked == true then (currentmat[submatnumber].bLightMap = true)
				)
			)
		)
	else		-- material simple
		(
		print "not a multi-sub material"
		if classof currentmat != NelMaterial
		then 
			(
			print "This is not a Nel Mat Let's convert"
			currentmat = (convertsinglemat currentmat)
			if setLightMapParam_on.checked == true then (currentmat.bLightMap = true)
			obj.material = currentmat

			)
		else
			(
			print "Already Nel Material : do nothing"
			)
		)
	)
------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------
--------------------------  UNPROCESS !!! ---------------------------------
------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------
Fn unconvertsinglemat thisoldmat =
	(
	print "un converting : "
	tempmat = Standard()
	for i in getpropnames tempmat do (setproperty tempmat i  (getproperty thisoldmat.standard i))
	tempmat.name = thisoldmat.name
	return tempmat
	)

Fn unprocess_obj obj = 
	(
--	if obj.material == undefined then messagebox "le script va crasher : \nun object de la scene n'a pas de materiau !"
	print "ici un-NeL-ificator !"
	print obj.name
	print obj.material
	currentmat = obj.material
	print currentmat
	if classof currentmat==Multimaterial 
	then		-- traiter un multimat
		(
		print "multi-sub material"
		for submatnumber = 1 to currentmat.count do 
			(
			if classof currentmat[submatnumber] == NelMaterial
			then 
				(
				print "This is a Nel Mat --> Let's convert"
--				messagebox (currentmat.names[submatnumber] as string)
				currentmat[submatnumber] = (unconvertsinglemat currentmat[submatnumber])
--				messagebox (currentmat.names[submatnumber] as string)
				if currentmat.names[submatnumber] == undefined then currentmat.names[submatnumber] = currentmat[submatnumber].name
				)
			else
				(
				print "Not a Nel Material : do nothing" 
				)
			)
		)
	else		-- material simple
		(
		print "not a multi-sub material"
		if classof currentmat == NelMaterial
		then 
			(
			print "This is a Nel Mat Let's convert"
			currentmat = (unconvertsinglemat currentmat)
			obj.material = currentmat
			)
		else
			(
			print "Not a Nel Material : do nothing"
			)
		)
	)
------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------

Fn Showmapinviewport obj booltoset =
	(
	print (classof booltoset)
	print booltoset
	print obj.name
	if classof obj.material == Multimaterial
	then
		(
		print "multimat"
		for u = 1 to obj.material.count do 
			(
			if classof obj.material[u] == NelMaterial
			then try (showTextureMap obj.material[u] obj.material[u].standard.diffusemap booltoset) catch(print "no texture map")
			else try (showTextureMap obj.material[u] obj.material[u].diffusemap booltoset) catch(print "no texture map")
			)
		)
	else
		(
		print "pas multimat"
		if classof obj.material == NelMaterial
		then try (showTextureMap obj.material obj.material.standard.diffusemap booltoset) catch(print "no texture map")
		else try (showTextureMap obj.material obj.material.diffusemap booltoset) catch(print "no texture map")
		)
	)

on ShowMaps pressed do
	(
	parambool = true
	if selectiononlyforshowmap.checked==true 
	then (for i in selection do Showmapinviewport i parambool)
	else (for i in $geometry do Showmapinviewport i parambool)
	)

on HideMaps pressed do
	(
	parambool = false
	if selectiononlyforshowmap.checked==true
	then (for i in selection do Showmapinviewport i parambool)
	else (for i in $geometry do Showmapinviewport i parambool)
	)


	
on goforall pressed do
	(
	if selectiononly.checked==true 
	then for curr_obj in selection do 
		(
		if curr_obj.material != undefined 
		then
			(
			object_processed_label.text = curr_obj.name 
			process_obj curr_obj
			)
		else
			(
			print "pas de materiau sur cet object : skip"
			)
		)
	else for curr_obj in $geometry do 
			(
		if curr_obj.material != undefined 
		then
			(
			object_processed_label.text = curr_obj.name 
			process_obj curr_obj
			)
		else
			(
			print "pas de materiau sur cet object : skip"
			)
		)
	object_processed_label.text = "finished"
	)
	


on unnelconv pressed do 
	(
	if selectiononly.checked==true 
	then for curr_obj in selection do 
		(
		if curr_obj.material != undefined 
		then
			(
			object_processed_label.text = curr_obj.name 
			unprocess_obj curr_obj
			)
		else
			(
			print "pas de materiau sur cet object : skip"
			)
		)
	else for curr_obj in $geometry do 
			(
		if curr_obj.material != undefined 
		then
			(
			object_processed_label.text = curr_obj.name 
			unprocess_obj curr_obj
			)
		else
			(
			print "pas de materiau sur cet object : skip"
			)
		)
	object_processed_label.text = "finished"
	)

)

------------- fin rollout ------------------
	
if nelmatconv_floater != undefined do
	(
	closerolloutfloater nelmatconv_floater
	)

nelmatconv_Floater = newRolloutFloater "nel mat conv" 178 340
addrollout nelmatconv_rollout nelmatconv_Floater

