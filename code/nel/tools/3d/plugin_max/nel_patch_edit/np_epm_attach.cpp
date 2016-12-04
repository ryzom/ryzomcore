#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000

// ------------------------------------------------------------------------------------------------------------------------------------------------------

extern int		attachReorient;

// ------------------------------------------------------------------------------------------------------------------------------------------------------

int EditPatchMod::DoAttach(INode *node, PatchMesh *attPatch, RPatchMesh *rattPatch, bool & canUndo) 
{
	ModContextList mcList;	
	INodeTab nodes;	

	if (!ip)
		return 0;

	ip->GetModContexts(mcList, nodes);

	if (mcList.Count() != 1)
	{
		nodes.DisposeTemporary();
		return 0;
	}

	EditPatchData *patchData =(EditPatchData*)mcList[0]->localData;
	if (!patchData)
	{
		nodes.DisposeTemporary();
		return 0;
	}

	// If the mesh isn't yet cached, this will cause it to get cached.
	RPatchMesh *rpatch;
	PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
	if (!patch)
	{
		nodes.DisposeTemporary();
		return 0;
	}
	patchData->RecordTopologyTags(patch);
	RecordTopologyTags();
	patchData->BeginEdit(ip->GetTime());

	// Transform the shape for attachment:
	// If reorienting, just translate to align pivots
	// Otherwise, transform to match our transform
	Matrix3 attMat(1);
	if (attachReorient)
	{
		Matrix3 thisTM = nodes[0]->GetNodeTM(ip->GetTime());
		Matrix3 thisOTMBWSM = nodes[0]->GetObjTMBeforeWSM(ip->GetTime());
		Matrix3 thisPivTM = thisTM * Inverse(thisOTMBWSM);
		Matrix3 otherTM = node->GetNodeTM(ip->GetTime());
		Matrix3 otherOTMBWSM = node->GetObjTMBeforeWSM(ip->GetTime());
		Matrix3 otherPivTM = otherTM * Inverse(otherOTMBWSM);
		Point3 otherObjOffset = node->GetObjOffsetPos();
		attMat = Inverse(otherPivTM) * thisPivTM;
	}
	else 
	{
		attMat = node->GetObjectTM(ip->GetTime()) *
			Inverse(nodes[0]->GetObjectTM(ip->GetTime()));
	}

	// RB 3-17-96 : Check for mirroring
	AffineParts parts;
	decomp_affine(attMat, &parts);
	if (parts.f < 0.0f)
	{
		int v[8], ct, ct2, j;
		int tvInteriors[4], tvHandles[8];
		Point3 p[9];
		
		for (int i = 0; i < attPatch->numPatches; i++)
		{

			// Re-order rpatch
			if (attPatch->patches[i].type == PATCH_QUAD)
			{
				UI_PATCH rpatch=rattPatch->getUIPatch (i);
				int ctU=rpatch.NbTilesU<<1;
				int ctV=rpatch.NbTilesV<<1;
				int nU;
				for (nU=0; nU<ctU; nU++)
				{
					for (int nV=0; nV<ctV; nV++)
					{
						rattPatch->getUIPatch (i).getTileDesc (nU+nV*ctU)=rpatch.getTileDesc (ctU-1-nU+(ctV-1-nV)*ctU);
					}
				}
				for (nU=0; nU<ctU+1; nU++)
				{
					for (int nV=0; nV<ctV+1; nV++)
					{
						rattPatch->getUIPatch (i).setColor (nU+nV*(ctU+1), rpatch.getColor (ctU-nU+(ctV-nV)*ctU));
					}
				}
			}

			// Re-order vertices
			ct = attPatch->patches[i].type == PATCH_QUAD ? 4 : 3;
			for (j = 0; j < ct; j++)
			{
				v[j] = attPatch->patches[i].v[j];
			}
			for (j = 0; j < ct; j++)
			{
				attPatch->patches[i].v[j] = v[ct - j - 1];
			}
			
			// Re-order vecs
			ct  = attPatch->patches[i].type == PATCH_QUAD ? 8 : 6;
			ct2 = attPatch->patches[i].type == PATCH_QUAD ? 5 : 3;
			for (j = 0; j < ct; j++)
			{
				v[j] = attPatch->patches[i].vec[j];
			}
			for (j = 0; j < ct; j++, ct2--)
			{
				if (ct2 < 0)
					ct2 = ct - 1;
				attPatch->patches[i].vec[j] = v[ct2];
			}
			
			// Re-order enteriors
			if (attPatch->patches[i].type == PATCH_QUAD)
			{
				ct = 4;
				for (j = 0; j < ct; j++)
				{
					v[j] = attPatch->patches[i].interior[j];
				}
				for (j = 0; j < ct; j++)
				{
					attPatch->patches[i].interior[j] = v[ct - j - 1];
				}
			}
			
			// Re-order aux
			if (attPatch->patches[i].type == PATCH_TRI)
			{
				ct = 9;
				for (j = 0; j < ct; j++)
				{
					p[j] = attPatch->patches[i].aux[j];
				}
				for (j = 0; j < ct; j++)
				{
					attPatch->patches[i].aux[j] = p[ct - j - 1];
				}
			}
			
#if (MAX_RELEASE < 4000) /* #if (MAX_RELEASE < 4000) */
			// Re-order TV faces if present
			for (int chan = 0; chan < patch->getNumMaps(); ++chan)
			{
				if (attPatch->tvPatches[chan])
				{
					ct = 4;
					for (j = 0; j < ct; j++)
					{
						v[j] = attPatch->tvPatches[chan][i].tv[j];
					}
					for (j = 0; j < ct; j++)
					{
						attPatch->tvPatches[chan][i].tv[j] = v[ct - j - 1];
					}
				}
			}
#else /* #if (MAX_RELEASE < 4000) #else */
			// Re-order TV faces if present
			for(int chan = -NUM_HIDDENMAPS; chan < patch->getNumMaps(); ++chan) 
			{
				if (chan < attPatch->getNumMaps() && attPatch->mapPatches(chan)) 
				{
					ct = attPatch->patches[i].type==PATCH_QUAD ? 4 : 3;
					for (j=0; j<ct; j++) 
					{
						v[j] = attPatch->mapPatches(chan)[i].tv[j];
						tvInteriors[j] = attPatch->mapPatches(chan)[i].interiors[j];
						int a;
						int b;
						a = j*2-1;
						b = j*2;
						if (a<0) a = ct*2-1;
						tvHandles[j*2] = attPatch->mapPatches(chan)[i].handles[a];
						tvHandles[j*2+1] = attPatch->mapPatches(chan)[i].handles[b];
					}
					for (j=0; j<ct; j++) 
					{
						attPatch->mapPatches(chan)[i].tv[j] = v[ct-j-1];
						attPatch->mapPatches(chan)[i].interiors[j] = tvInteriors[(ct)-(j)-1];
						int index = ct-j-1;
						int a;
						int b;
						a = j*2-1;
						b = j*2;
						if (a<0) a = ct*2-1;
						attPatch->mapPatches(chan)[i].handles[b] = tvHandles[index*2];
						attPatch->mapPatches(chan)[i].handles[a] = tvHandles[index*2+1];
					}
				}
			}
#endif /* #if (MAX_RELEASE < 4000) #else #endif */
		}
	}

	int i;
	for (i = 0; i < attPatch->numVerts; ++i)
		attPatch->verts[i].p = attPatch->verts[i].p * attMat;
	for (i = 0; i < attPatch->numVecs; ++i)
		attPatch->vecs[i].p = attPatch->vecs[i].p * attMat;
	attPatch->computeInteriors();

	theHold.Begin();

	// Combine the materials of the two nodes.
	int mat2Offset = 0;
	Mtl *m1 = nodes[0]->GetMtl();
	Mtl *m2 = node->GetMtl();
	bool condenseMe = FALSE;
	if (m1 && m2 &&(m1 != m2))
	{
		if (attachMat == ATTACHMAT_IDTOMAT)
		{
			int ct = 1;
			if (m1->IsMultiMtl())
				ct = m1->NumSubMtls();
			for (int i = 0; i < patch->numPatches; ++i)
			{
				int mtid = patch->getPatchMtlIndex(i);
				if (mtid >= ct)
					patch->setPatchMtlIndex(i, mtid % ct);
			}
			FitPatchIDsToMaterial(*attPatch, m2);
			if (condenseMat)
				condenseMe = TRUE;
		}
		// the theHold calls here were a vain attempt to make this all undoable.
		// This should be revisited in the future so we don't have to use the SYSSET_CLEAR_UNDO.
		theHold.Suspend();
		if (attachMat == ATTACHMAT_MATTOID)
		{
			m1 = FitMaterialToPatchIDs(*patch, m1);
			m2 = FitMaterialToPatchIDs(*attPatch, m2);
		}
		
		Mtl *multi = CombineMaterials(m1, m2, mat2Offset);
		if (attachMat == ATTACHMAT_NEITHER)
			mat2Offset = 0;
		theHold.Resume();
		// We can't be in face subobject mode, else we screw up the materials:
		DWORD oldSL = patch->selLevel;
		DWORD roldSL = patch->selLevel;
		patch->selLevel = PATCH_OBJECT;
		rpatch->SetSelLevel (EP_OBJECT);
		nodes[0]->SetMtl(multi);
		patch->selLevel = oldSL;
		rpatch->SetSelLevel (roldSL);
		m1 = multi;
		canUndo = FALSE;	// Absolutely cannot undo material combinations.
	}
	if (!m1 && m2)
	{
		// We can't be in face subobject mode, else we screw up the materials:
		DWORD oldSL = patch->selLevel;
		DWORD roldSL = rpatch->GetSelLevel();
		patch->selLevel = PATCH_OBJECT;
		rpatch->SetSelLevel (EP_OBJECT);
		nodes[0]->SetMtl(m2);
		patch->selLevel = oldSL;
		rpatch->SetSelLevel (roldSL);
		m1 = m2;
	}

	// Start a restore object...
	if (theHold.Holding())
		theHold.Put(new PatchRestore(patchData, this, patch, rpatch, _T("DoAttach")));

	// Do the attach
	patch->Attach(attPatch, mat2Offset);
	rpatch->Attach(rattPatch, *patch);
	patchData->UpdateChanges(patch, rpatch);
	patchData->TempData(this)->Invalidate(PART_TOPO | PART_GEOM);

	// Get rid of the original node
	ip->DeleteNode(node);

	ResolveTopoChanges();
	theHold.Accept(GetString(IDS_TH_ATTACH));

	if (m1 && condenseMe)
	{
		// Following clears undo stack.
		patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		m1 = CondenseMatAssignments(*patch, m1);
	}

	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_TOPO | PART_GEOM, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
	return 1;
}
