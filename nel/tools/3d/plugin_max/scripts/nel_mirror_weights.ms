----------------------------
-- DEFINE SOME GLOBALS
----------------------------

global Physique_Tool

----------------------------
-- INTERFACE FUN
----------------------------

(
if Physique_Tool != undefined do ( closerolloutfloater Physique_Tool )
Physique_Tool = newrolloutfloater "Physique Tool" 190 190

rollout PhyMirror "Physique Tool"
(

group "Mirror Options" 
	(
		button MirrorPhy "Mirror Selected" width:140 height:20
		spinner thresh "Threshold  "  scale:.01 align:#center type:#float range:[0,5,.2]
	)
	label info1 "Updates will be" align:#center
	label info2 "given here." align:#center
	

----------------------------
-- DO IT MIROR METHOD
----------------------------
fn MirrorPhyFN = 
(
	local selverts= #()

	-- Yoyo: 'as array' crash on max 4... ???
	local seltam = getVertSelection $
	for i in seltam do append selverts i


	local threshold = thresh.value	
	NelMirrorPhysique $ selverts threshold
)


----------------------------------
-- MIRROR_PHY BUTTON
----------------------------------
on PhyMirror open do 
(
)

on MirrorPhy pressed do 
(
	GoAhead = false

	info1.text = "STARTING MIRROR"
	info2.text = ""

	if $ != undefined and getCommandPanelTaskMode() == #modify and subObjectLevel != 0 then 
	(
		GoAhead = true 
	)
				
	if GoAhead then 
	(
		try
		(
			if subObjectLevel != 1 do 
			(
				$.EditablePoly.ConvertSelection subobjectLevel #Vertex
				subObjectLevel = 1
			)
		)
		catch()

		with redraw off
		(
			-- undo off 
			(
				setWaitCursor()
				MirrorPhyFN();
				setArrowCursor()
			)
		)
		
		info1.text = "DONE! SELECTING"
		info2.text = "SUCCESSFUL TRANSFERS."
	)
	else 
	(
		MessageBox "To Mirror: Select verts in Top_Level Subobject mode."
	)
)
) --End rollout PhyMirror
addRollout PhyMirror Physique_Tool

) --End roulloutfloater


