
gc()
nodes = getCurrentSelection()
max select none
clearselection()

undo off
(
	for cnode in nodes do
	(
		if (classof cnode) == Editable_Patch or (classof cnode) == RklPatch then 
		(
			print cnode.name
			selectmore cnode
			if (classof cnode) == Editable_Patch and (classof cnode) != RklPatch then
			(
				modPanel.addModToSelection (NeL_Convert ()) ui:on
			)
			modPanel.addModToSelection (NeL_Edit ()) ui:on
			setCommandPanelTaskMode #modify
			cmod = modpanel.getCurrentObject()
			pcount = (GetRykolPatchCount cnode)
			print pcount
			for p = 1 to pcount do
			(
				--print p
				vbegin = (NeLGetPatchVertex cnode p 1)
				vend = (NeLGetPatchVertex cnode p 2)
				vref = (NeLGetPatchVertex cnode p 3)
				begin = (GetRykolVertexPos cnode vbegin)
				end = (GetRykolVertexPos cnode vend)
				ref = (GetRykolVertexPos cnode vref)
				normal = (cross (end - begin) (ref - begin))
				normal = (normalize normal)
					
				rotnormal = (normal * (rotateZMatrix -90))
				if (normal.z > 0.9) then
				(
					-- print "x normal"
					rotnormal = (normal * (rotateXMatrix -90))
				)
				-- print rotnormal
				
				-- print normal
				-- print rotnormal
				-- print begin
				-- print end
				dir = (normalize (end - begin))
				-- print dir
				score1 = (dot dir rotnormal)
					
				RykolTurnPatch cnode cmod (p)
				vbegin = (NeLGetPatchVertex cnode p 1)
				vend = (NeLGetPatchVertex cnode p 2)
				begin = (GetRykolVertexPos cnode vbegin)
				end = (GetRykolVertexPos cnode vend)
				dir = (normalize (end - begin))
				score2 = (dot dir rotnormal)
					
				RykolTurnPatch cnode cmod (p)
				vbegin = (NeLGetPatchVertex cnode p 1)
				vend = (NeLGetPatchVertex cnode p 2)
				begin = (GetRykolVertexPos cnode vbegin)
				end = (GetRykolVertexPos cnode vend)
				dir = (normalize (end - begin))
				score3 = (dot dir rotnormal)
					
				RykolTurnPatch cnode cmod (p)
				vbegin = (NeLGetPatchVertex cnode p 1)
				vend = (NeLGetPatchVertex cnode p 2)
				begin = (GetRykolVertexPos cnode vbegin)
				end = (GetRykolVertexPos cnode vend)
				dir = (normalize (end - begin))
				score4 = (dot dir rotnormal)
				
				-- print score1
				-- print score2
				-- print score3
				-- print score4
				
				if (score1 > score2 and score1 > score3 and score1 > score4) then
				(
					-- print "score 1"
					RykolTurnPatch cnode cmod (p)
				)
				else if (score2 > score3 and score2 > score4) then
				(
					-- print "score 2"
					RykolTurnPatch cnode cmod (p)
					RykolTurnPatch cnode cmod (p)
				)
				else if (score3 > score4) then
				(
					-- print "score 3"
					RykolTurnPatch cnode cmod (p)
					RykolTurnPatch cnode cmod (p)
					RykolTurnPatch cnode cmod (p)
				)
				else
				(
					-- print "score 4"
				)
			)
			maxOps.CollapseNode cnode off
			max select none
		)
	)
)
