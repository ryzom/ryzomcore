-- Function to reload bitmaps in a texture
fn reload_material mat =
(
	if classof mat == Multimaterial then
	(
		for submat = 1 to mat.count do 
		(
			reload_material mat[submat]
		)
	)
	else if classof mat == Standard then
	(
		-- Has a texture ?
		if mat.DiffuseMap != undefined then
		(
			-- Is it a bitmaptex ?
			if classof mat.DiffuseMap == bitmapTex then
			(
				NelReloadTexture mat.DiffuseMap
			)
		)
	)
)

-- For each material in the material editor
for mate in meditMaterials do
(
	reload_material mate
	forceCompleteRedraw ()
)
