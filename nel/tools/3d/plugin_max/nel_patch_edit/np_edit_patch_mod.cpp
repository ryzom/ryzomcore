#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME 2000

// ------------------------------------------------------------------------------------------------------------------------------------------------------

class EPSelSetNameRestore : public RestoreObj 
{
public:
	TSTR undo, redo;
	TSTR *target;
	EditPatchMod *mod;
	EPSelSetNameRestore(EditPatchMod *m, TSTR *t, TSTR &newName) 
	{
		mod = m;
		undo = *t;
		target = t;
	}
	void Restore(int isUndo) 
	{			
		if (isUndo)
			redo = *target;
		*target = undo;
		if (mod->ip)
			mod->ip->NamedSelSetListChanged();
	}
	void Redo() 
	{
		*target = redo;
		if (mod->ip)
			mod->ip->NamedSelSetListChanged();
	}
				
	TSTR Description() {return TSTR(_T("Sel Set Name"));}
};

// ------------------------------------------------------------------------------------------------------------------------------------------------------

extern int		attachReorient;
extern Point3	zeroPoint;
extern BOOL		filterVerts;
extern int		lockedHandles;

extern void ChangePatchType(PatchMesh *patch, int index, int type);

HWND				EditPatchMod::hSelectPanel    = NULL;
HWND				EditPatchMod::hOpsPanel       = NULL;
HWND				EditPatchMod::hSurfPanel      = NULL;
HWND				EditPatchMod::hTilePanel      = NULL;
HWND				EditPatchMod::hEdgePanel      = NULL;
BOOL				EditPatchMod::rsSel           = TRUE;
BOOL				EditPatchMod::rsOps           = TRUE;
BOOL				EditPatchMod::rsSurf          = TRUE;
BOOL				EditPatchMod::rsTile          = TRUE;
BOOL				EditPatchMod::rsEdge          = TRUE;
IObjParam*          EditPatchMod::ip              = NULL;
MoveModBoxCMode*    EditPatchMod::moveMode        = NULL;
RotateModBoxCMode*  EditPatchMod::rotMode 	      = NULL;
UScaleModBoxCMode*  EditPatchMod::uscaleMode      = NULL;
NUScaleModBoxCMode* EditPatchMod::nuscaleMode     = NULL;
SquashModBoxCMode *	EditPatchMod::squashMode      = NULL;
SelectModBoxCMode*  EditPatchMod::selectMode      = NULL;
ISpinnerControl*	EditPatchMod::weldSpin        = NULL;
ISpinnerControl*	EditPatchMod::stepsSpin       = NULL;
ISpinnerControl*	EditPatchMod::tileSpin        = NULL;
ISpinnerControl*	EditPatchMod::transitionSpin        = NULL;

// 3-18-99 to suport render steps and removal of the mental tesselator
ISpinnerControl*	EditPatchMod::stepsRenderSpin       = NULL;

BOOL				EditPatchMod::settingViewportTess = FALSE;
BOOL				EditPatchMod::settingDisp     = FALSE;
ISpinnerControl*	EditPatchMod::uSpin           = NULL;
ISpinnerControl*	EditPatchMod::vSpin           = NULL;
ISpinnerControl*	EditPatchMod::edgeSpin        = NULL;
ISpinnerControl*	EditPatchMod::distSpin        = NULL;
ISpinnerControl*	EditPatchMod::angSpin         = NULL;
ISpinnerControl*	EditPatchMod::mergeSpin       = NULL;
ISpinnerControl*	EditPatchMod::matSpin         = NULL;
ISpinnerControl*	EditPatchMod::tessUSpin       = NULL;
ISpinnerControl*	EditPatchMod::tessVSpin       = NULL;
ISpinnerControl*	EditPatchMod::tileNum         = NULL;
ISpinnerControl*	EditPatchMod::tileRot	      = NULL;
BOOL				EditPatchMod::patchUIValid    = TRUE;
BOOL				EditPatchMod::tileUIValid     = TRUE;
BOOL				EditPatchMod::edgeUIValid     = TRUE;
PickPatchAttach		EditPatchMod::pickCB;
int					EditPatchMod::condenseMat     = FALSE;
int					EditPatchMod::attachMat       = ATTACHMAT_IDTOMAT;
int					EditPatchMod::channelModified = EDITPAT_CHANNELS;

EPM_BindCMode*		EditPatchMod::bindMode   = NULL;
EPM_ExtrudeCMode*	EditPatchMod::extrudeMode   = NULL;
EPM_BevelCMode*		EditPatchMod::bevelMode   = NULL;
int					EditPatchMod::CurrentTileSet	= -1;
int					EditPatchMod::brushSize		= 0;		// Default 1 tile
int					EditPatchMod::ColorBushSize	= 0;
int					EditPatchMod::tileSize		= 1;		// Default 256
bool				EditPatchMod::additiveTile	= false;	// 
int					EditPatchMod::TileGroup=0;				// Default all tiles
int					EditPatchMod::DisplaceTile=0;			// Default displace 0
int					EditPatchMod::DisplaceTileSet=-1;		// 
uint				EditPatchMod::TileFillRotation=0;
bool				EditPatchMod::TileTrick=false;
bool				EditPatchMod::automaticLighting=false;

// ------------------------------------------------------------------------------------------------------------------------------------------------------

//--- Named Selection Set Methods ------------------------------------

// Used by EditPatchMod destructor to free pointers
void EditPatchMod::ClearSetNames()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < namedSel[i].Count(); j++)
		{
			delete namedSel[i][j];
			namedSel[i][j] = NULL;
		}
	}
}

int EditPatchMod::FindSet(TSTR &setName, int level)
{	
	nlassert(level>0 && level < 4);
	for (int i = 0; i < namedSel[level - 1].Count(); i++)
	{
		if (setName == *namedSel[level - 1][i])
		{
			return i;			
		}
	}
	return -1;
}

void EditPatchMod::AddSet(TSTR &setName, int level)
{
	nlassert(level>0 && level < 4);
	TSTR *name = new TSTR(setName);
	namedSel[level - 1].Append(1, &name);
}

void EditPatchMod::RemoveSet(TSTR &setName, int level)
{
	MaybeFixupNamedSels();
	nlassert(level>0 && level < 4);
	int i = FindSet(setName, level);
	if (i >= 0)
	{
		delete namedSel[level - 1][i];
		namedSel[level - 1].Delete(i, 1);
	}
}



void EditPatchMod::SetupNamedSelDropDown()
{
	// Setup named selection sets	
	if (selLevel == EP_OBJECT)
		return;
	ip->ClearSubObjectNamedSelSets();
	for (int i = 0; i < namedSel[selLevel - 1].Count(); i++)
		ip->AppendSubObjectNamedSelSet(*namedSel[selLevel - 1][i]);
}

int EditPatchMod::NumNamedSelSets() 
{
	if (GetSubobjectLevel() == PO_OBJECT)
		return 0;
	if (GetSubobjectLevel() == PO_TILE)
		return 0;
	return namedSel[selLevel - 1].Count();
}

TSTR EditPatchMod::GetNamedSelSetName(int i) 
{
	return *namedSel[selLevel - 1][i];
}


void EditPatchMod::SetNamedSelSetName(int index, TSTR &newName) 
{
	if (!ip)
		return;
	MaybeFixupNamedSels();
	
	// First do the master name list
	if (theHold.Holding())
		theHold.Put(new EPSelSetNameRestore(this, namedSel[selLevel - 1][index], newName));
	
	// Save the old name so we can change those in the EditPatchData
	TSTR oldName = *namedSel[selLevel - 1][index];
	*namedSel[selLevel - 1][index] = newName;
	
	ModContextList mcList;
	INodeTab nodes;
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (patchData)
			patchData->GetSelSet(this).RenameSet(oldName, newName);
	}
	nodes.DisposeTemporary();
}

void EditPatchMod::NewSetByOperator(TSTR &newName, Tab < int> &sets, int op) 
{
	MaybeFixupNamedSels();
	
	// First do it in the master name list
	AddSet(newName, selLevel);		
	// TO DO: Undo?
	ModContextList mcList;
	INodeTab nodes;	
	
	if (!ip)
		return;
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		GenericNamedSelSetList &set = patchData->GetSelSet(this);
		BitArray bits = *set.GetSetByIndex(sets[0]);
		for (i = 1; i < sets.Count(); i++)
		{
			BitArray *bit2 = set.GetSetByIndex(sets[i]);
			switch (op)
			{
			case NEWSET_MERGE:
				bits |= *bit2;
				break;
				
			case NEWSET_INTERSECTION:
				bits &= *bit2;
				break;
				
			case NEWSET_SUBTRACT:
				bits &= ~(*bit2);
				break;
			}
		}
		set.AppendSet(bits, 0, newName);
	}	
	
	nodes.DisposeTemporary();
}
// -----------------------------------------------------------------------------------------------------------------------------------------------

// Named selection set copy/paste methods follow...

static INT_PTR CALLBACK PickSetNameDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static TSTR *name;
	
	switch (msg)
	{
	case WM_INITDIALOG: 
		{
			name =(TSTR*)lParam;
			ICustEdit *edit =GetICustEdit(GetDlgItem(hWnd, IDC_SET_NAME));
			edit->SetText(*name);
			ReleaseICustEdit(edit);
			break;
		}
		
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK: 
			{
				ICustEdit *edit =GetICustEdit(GetDlgItem(hWnd, IDC_SET_NAME));
				TCHAR buf[256];
				edit->GetText(buf, 256);
				*name = TSTR(buf);
				ReleaseICustEdit(edit);
				EndDialog(hWnd, 1);
				break;
			}
			
		case IDCANCEL:
			EndDialog(hWnd, 0);
			break;
		}
		break;
		
		default:
			return FALSE;
	};
	return TRUE;
}

BOOL EditPatchMod::GetUniqueSetName(TSTR &name) 
{
	while (1) 
	{		
		if (FindSet(name, selLevel) < 0)
			break;
		
		if (!DialogBoxParam(
			hInstance, 
			MAKEINTRESOURCE(IDD_PASTE_NAMEDSET),
			ip->GetMAXHWnd(), 
			PickSetNameDlgProc,
			(LPARAM)&name))
			return FALSE;		
	}
	return TRUE;
}





static INT_PTR CALLBACK PickSetDlgProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{	
	switch (msg)
	{
	case WM_INITDIALOG:	
		{
			Tab<TSTR*> &names = *((Tab < TSTR*>*)lParam);
			for (int i = 0; i < names.Count(); i++)
			{
				int pos  = SendDlgItemMessage(hWnd, IDC_NS_LIST, LB_ADDSTRING, 0, (LPARAM)(*names[i]->data()));
				SendDlgItemMessage(hWnd, IDC_NS_LIST, LB_SETITEMDATA, pos, i);
			}
			break;
		}
		
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_NS_LIST:
			if (HIWORD(wParam) != LBN_DBLCLK)
				break;
			// fall through
		case IDOK: 
			{
				int sel = SendDlgItemMessage(hWnd, IDC_NS_LIST, LB_GETCURSEL, 0, 0);
				if (sel != LB_ERR)
				{
					int res =SendDlgItemMessage(hWnd, IDC_NS_LIST, LB_GETITEMDATA, sel, 0);
					EndDialog(hWnd, res);
					break;
				}
				// fall through
			}
			
		case IDCANCEL:
			EndDialog(hWnd, - 1);
			break;
		}
		break;
		
		default:
			return FALSE;
	};
	return TRUE;
}

// -------------------------------------------------------------------------------------------------------------------------------------


int EditPatchMod::SelectNamedSet() 
{
	Tab < TSTR*> names = namedSel[selLevel - 1];
	return DialogBoxParam(
		hInstance, 
		MAKEINTRESOURCE(IDD_SEL_NAMEDSET),
		ip->GetMAXHWnd(), 
		PickSetDlgProc,
		(LPARAM)&names);
}

void EditPatchMod::NSCopy() 
{
	MaybeFixupNamedSels();
	if (selLevel == EP_OBJECT)
		return;
	int index = SelectNamedSet();
	if (index < 0)
		return;
	if (!ip)
		return;
	// Get the name for that index
	int nsl = namedSetLevel[selLevel];
	TSTR setName = *namedSel[nsl][index];
	PatchNamedSelClip *clip = new PatchNamedSelClip(setName);
	
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		
		GenericNamedSelSetList &setList = patchData->GetSelSet(this);
		BitArray *set = setList.GetSet(setName);
		if (set)
		{
			BitArray *bits = new BitArray(*set);
			clip->sets.Append(1, &bits);
		}
	}
	SetPatchNamedSelClip(clip, namedClipLevel[selLevel]);
	
	// Enable the paste button
	ICustButton *but = GetICustButton(GetDlgItem(hSelectPanel, IDC_NS_PASTE));
	but->Enable();
	ReleaseICustButton(but);
}

void EditPatchMod::NSPaste() 
{
	MaybeFixupNamedSels();
	if (selLevel == EP_OBJECT)
		return;
	int nsl = namedSetLevel[selLevel];
	PatchNamedSelClip *clip = GetPatchNamedSelClip(namedClipLevel[selLevel]);
	if (!clip)
		return;
	TSTR name = clip->name;
	if (!GetUniqueSetName(name))
		return;
	if (!ip)
		return;
	
	ModContextList mcList;
	INodeTab nodes;
	
	AddSet(name, selLevel);
	
	ip->GetModContexts(mcList, nodes);
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		
		GenericNamedSelSetList & setList = patchData->GetSelSet(this);
		
		if (i >= clip->sets.Count())
		{
			BitArray bits;
			setList.AppendSet(bits, 0, name);
		}
		else
			setList.AppendSet(*clip->sets[i], 0, name);
	}	
	
	ActivateSubSelSet(name);
	ip->SetCurNamedSelSet(name);
	SetupNamedSelDropDown();
}

// Old MAX files (pre-r3) have EditPatchData named selections without names assigned.  This
// assigns them their proper names for r3 and later code.  If no fixup is required, this does nothing.
void EditPatchMod::MaybeFixupNamedSels() 
{
	int i;
	if (!ip)
		return;
	
	// Go thru the modifier contexts, and stuff the named selection names into the EditPatchData
	ModContextList mcList;
	INodeTab nodes;
	
	ip->GetModContexts(mcList, nodes);
	
#ifdef DBG_NAMEDSELS
	DebugPrint("Context/named sels:\n");
	for (i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		
		// Go thru each subobject level
		for (int j = 0; j < 3; ++j)
		{
			GenericNamedSelSetList &pdSel = patchData->GetSelSet(j);
			for (int k = 0; k < pdSel.Count(); ++k)
				DebugPrint("Context %d, level %d, set %d: [%s]\n", i, j, k, *pdSel.names[k]);
		}
	}	
#endif // DBG_NAMEDSELS
	
	if (!namedSelNeedsFixup)
	{
#ifdef DBG_NAMEDSELS
		DebugPrint("!!! NO FIXUP REQUIRED !!!\n");
#endif // DBG_NAMEDSELS
		return;
	}
	
#ifdef DBG_NAMEDSELS
	DebugPrint("*** Fixing up named sels ***\n");
#endif // DBG_NAMEDSELS
	
	for (i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		
		// Go thru each subobject level
		for (int j = 0; j < 3; ++j)
		{
			Tab < TSTR*> &mSel = namedSel[j];
			GenericNamedSelSetList &pdSel = patchData->GetSelSet(j);
			// Some old files may have improper counts in the EditPatchData.  Limit the counter
			int mc = mSel.Count();
			int pdc = pdSel.Count();
			int limit =(mc < pdc) ? mc : pdc;
#ifdef DBG_NAMEDSELS
			if (mc != pdc)
				DebugPrint("****** mSel.Count=%d, pdSel.Count=%d ******\n", mc, pdc);
#endif // DBG_NAMEDSELS
			for (int k = 0; k < limit; ++k)
				*pdSel.names[k] = *mSel[k];
		}
	}	
	
	nodes.DisposeTemporary();
	namedSelNeedsFixup = FALSE;
}

void EditPatchMod::RemoveAllSets()
{
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		
		int j;
		for (j = patchData->vselSet.Count() - 1; j >= 0; j--)
		{
			patchData->vselSet.DeleteSet(j);
		}
		for (j = patchData->pselSet.Count() - 1; j >= 0; j--)
		{
			patchData->pselSet.DeleteSet(j);
		}
		for (j = patchData->eselSet.Count() - 1; j >= 0; j--)
		{
			patchData->eselSet.DeleteSet(j);
		}		
	}	
	
	for (int j = 0; j < 3; j++)
	{
		for (int i = 0; i < namedSel[j].Count(); i++)
		{
			delete namedSel[j][i];		
		}
		namedSel[j].Resize(0);
	}
	
	ip->ClearCurNamedSelSet();
	ip->ClearSubObjectNamedSelSets();
	nodes.DisposeTemporary();
}

Interval EditPatchMod::LocalValidity(TimeValue t)
{
	// Force a cache if being edited.
	if (TestAFlag(A_MOD_BEING_EDITED))
		return NEVER;  			   
	return FOREVER;
}

RefTargetHandle EditPatchMod::Clone(RemapDir& remap) 
{
	EditPatchMod* newmod = new EditPatchMod();	
	newmod->selLevel = selLevel;
	newmod->displaySurface = displaySurface;
	newmod->displayLattice = displayLattice;
	newmod->meshSteps = meshSteps;
	// 3-18-99 to suport render steps and removal of the mental tesselator
	newmod->meshStepsRender = meshStepsRender;
	newmod->showInterior = showInterior;
	
	//	newmod->meshAdaptive = meshAdaptive;	// Future use (Not used now)
	newmod->transitionType = transitionType;
	newmod->tileLevel = tileLevel;
	newmod->tileMode = tileMode;
	newmod->includeMeshes = includeMeshes;
	newmod->keepMapping = keepMapping;
	newmod->viewTess = viewTess;
	newmod->prodTess = prodTess;
	newmod->dispTess = dispTess;
	newmod->mViewTessNormals = mViewTessNormals;
	newmod->mProdTessNormals = mProdTessNormals;
	newmod->mViewTessWeld = mViewTessWeld;
	newmod->mProdTessWeld = mProdTessWeld;
	newmod->propagate = propagate;
	return (newmod);
}

void EditPatchMod::ClearPatchDataFlag(ModContextList& mcList, DWORD f)
{
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;
		patchData->SetFlag(f, FALSE);
	}
}

void EditPatchMod::XFormHandles(XFormProc *xproc, TimeValue t, Matrix3& partm, Matrix3& tmAxis,	int object, int handleIndex)
{	
ModContextList mcList;		
INodeTab nodes;
Matrix3 mat, imat, theMatrix;
Interval valid;
int numAxis;
Point3 oldpt, newpt, oldin, oldout, rel;
BOOL shiftPressed = FALSE;
static BOOL wasBroken;
Point3 theKnot;
Point3 oldVector;
Point3 newVector;
float oldLen;
float newLen;
// DebugPrint("XFormHandles\n");
shiftPressed =(GetKeyState(VK_SHIFT) & 0x8000) ? TRUE : FALSE;

if (!ip)
	return;

ip->GetModContexts(mcList, nodes);
numAxis = ip->GetNumAxis();
ClearPatchDataFlag(mcList, EPD_BEENDONE);

EditPatchData *patchData =(EditPatchData*)mcList[object]->localData;
if (!patchData)
{
	nodes.DisposeTemporary();
	return;
}

// If the mesh isn't yet cache, this will cause it to get cached.
RPatchMesh *rpatch;
PatchMesh *patch = patchData->TempData(this)->GetPatch(t, rpatch);
if (!patch)
{
	nodes.DisposeTemporary();
	return;
}

// If this is the first edit, then the delta arrays will be allocated
patchData->BeginEdit(t);

// Create a change record for this object and store a pointer to its delta info in this EditPatchData
if (!TestAFlag(A_HELD))
{
	patchData->vdelta.SetSize(*patch, FALSE);
	if (theHold.Holding())
	{
		theHold.Put(new PatchRestore(patchData, this, patch, rpatch, _T("XFormHandles")));
	}
	patchData->vdelta.Zero();		// Reset all deltas
	patchData->ClearHandleFlag();
	wasBroken = FALSE;
}
else 
{
	if (wasBroken && !shiftPressed)
		wasBroken = FALSE;
	if (patchData->DoingHandles())
		patchData->ApplyHandlesAndZero(*patch);		// Reapply the slave handle deltas
	else
		patchData->vdelta.Zero();
}

patchData->SetHandleFlag(handleIndex);
int primaryKnot = patch->vecs[handleIndex].vert;
Point3Tab &pDeltas = patchData->vdelta.dtab.vtab;

tmAxis = ip->GetTransformAxis(nodes[object], primaryKnot);
mat    = nodes[object]->GetObjectTM(t, &valid) * Inverse(tmAxis);
imat   = Inverse(mat);
xproc->SetMat(mat);
			
// XForm the cache vertices
oldpt = patch->vecs[handleIndex].p;
newpt = xproc->proc(oldpt, mat, imat);

// Update the vector being moved
patch->vecs[handleIndex].p = newpt;

// Move the delta's vertices.
patchData->vdelta.SetVec(handleIndex, newpt - oldpt);

if (primaryKnot >= 0)
{
	PatchVert &vert = patch->verts[primaryKnot];
	theKnot = vert.p;
	// If locked handles, turn the movement into a transformation matrix
	// and transform all the handles attached to the owner vertex
	if (lockedHandles)
	{
		if (!wasBroken && shiftPressed)
			wasBroken = TRUE;
		goto locked_handles;
	}
	else 
	{
		if (shiftPressed)
		{
			wasBroken = TRUE;
			vert.flags &= ~PVERT_COPLANAR;
			// Need to record this for undo!
			patchData->vdelta.SetVertType(primaryKnot, PVERT_COPLANAR);
		}
		// If a coplanar knot, do the other vectors!
		// If at the same point as knot, do nothing!
		if ((vert.flags & PVERT_COPLANAR) &&(vert.vectors.Count() > 2) && !(newpt == theKnot))
		{
locked_handles:
		oldVector = oldpt - theKnot;
		newVector = newpt - theKnot;
		oldLen = Length(oldVector);
		newLen = Length(newVector);
		Point3 oldNorm = Normalize(oldVector);
		Point3 newNorm = Normalize(newVector);
		theMatrix.IdentityMatrix();
		Point3 axis;
		float angle = 0.0f;
		int owner = patch->vecs[handleIndex].vert;
		if (owner >= 0)
		{
			PatchVert &vert = patch->verts[owner];
			int vectors = vert.vectors.Count();
			// Watch out for cases where the vectors are exactly opposite -- This
			// results in an invalid axis for transformation!
			// In this case, we look for a vector to one of the other handles that
			// will give us a useful vector for the rotational axis
			if (newNorm == -oldNorm)
			{
				for (int v = 0; v < vectors; ++v)
				{
					int theVec = vert.vectors[v];
					// Ignore the vector being moved!
					if (theVec != handleIndex)
					{
						Point3 testVec = patch->vecs[theVec].p - pDeltas[theVec] - theKnot;
						if (testVec != zeroPoint)
						{
							Point3 testNorm = Normalize(testVec);
							if (!(testNorm == newNorm) && !(testNorm == oldNorm))
							{
								// Cross product gives us the normal of the rotational axis
								axis = Normalize(testNorm ^ newNorm);
								// The angle is 180 degrees
								angle = PI;
								goto build_matrix;
							}
						}
					}
				}
			}
			else 
			{
				// Get a matrix that will transform the old point to the new one
				// Cross product gives us the normal of the rotational axis
				axis = Normalize(oldNorm ^ newNorm);
				// Dot product gives us the angle
				float dot = DotProd(oldNorm, newNorm);
				if (dot >= -1.0f && dot <= 1.0f)
					angle =(float) - acos(dot);
			}
build_matrix:
			if (angle != 0.0f)
			{
				// Now let's build a matrix that'll do this for us!
				Quat quat = QFromAngAxis(angle, axis);
				quat.MakeMatrix(theMatrix);
				if (lockedHandles)
				{
					// If need to break the vector, 
					if (shiftPressed && vert.flags & PVERT_COPLANAR)
					{
						vert.flags &= ~PVERT_COPLANAR;
						patchData->vdelta.SetVertType(primaryKnot, PVERT_COPLANAR);
					}
				}
			}
			// Process all other handles through the matrix
			for (int v = 0; v < vectors; ++v)
			{
				int theVec = vert.vectors[v];
				// Ignore the vector being moved!
				if (theVec != handleIndex)
				{
					Point3 oldpt2 = patch->vecs[theVec].p - pDeltas[theVec];
					Point3 newpt2 =(oldpt2 - theKnot) * theMatrix + theKnot;
					patch->vecs[theVec].p = newpt2;
					// Move the delta's vertices.
					patchData->vdelta.SetVec(theVec, newpt2 - oldpt2);
				}
			}
		}
		}
	}
	}
	
	// Really only need to do this if neighbor knots are non-bezier
	patch->computeInteriors();
	
	patchData->UpdateChanges(patch, rpatch);
	rpatch->InvalidateBindingPos ();
	patchData->TempData(this)->Invalidate(PART_GEOM);
	patchData->SetFlag(EPD_BEENDONE, TRUE);
	
	// Mark all objects in selection set
	SetAFlag(A_HELD);
	
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
}

void EditPatchMod::XFormVerts(
		XFormProc *xproc, 
		TimeValue t, 
		Matrix3& partm, 
		Matrix3& tmAxis) 
{	
ModContextList mcList;		
INodeTab nodes;
Matrix3 mat, imat;	
Interval valid;
int numAxis;
Point3 oldpt, newpt, rel, delta;
int shiftPressed =(GetKeyState(VK_SHIFT) & 0x8000) ? 1 : 0;
static BOOL wasBroken;
static BOOL handleEdit = FALSE;
static int handleObject;
static int handleIndex;

if (!ip)
	return;

ip->GetModContexts(mcList, nodes);
numAxis = ip->GetNumAxis();
ClearPatchDataFlag(mcList, EPD_BEENDONE);

if (!TestAFlag(A_HELD))
{
	handleEdit = FALSE;
	// DebugPrint("Handle edit cleared\n");
	// Check all patches to see if they are altering a bezier vector handle...
	if (selLevel == EP_VERTEX)
	{
		for (int i = 0; i < mcList.Count(); i++)
		{
			EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
			if (!patchData)
				continue;
			if (patchData->GetFlag(EPD_BEENDONE))
				continue;
			
			// If the mesh isn't yet cache, this will cause it to get cached.
			RPatchMesh *rpatch;
			PatchMesh *patch = patchData->TempData(this)->GetPatch(t, rpatch);
			if (!patch)
				continue;
			
			if (!ip->SelectionFrozen() && patch->bezVecVert >= 0)
			{
				// Editing a bezier handle -- Go do it!
				handleEdit = TRUE;
				handleObject = i;
				handleIndex = patch->bezVecVert;
				goto edit_handles;
			}
			patchData->SetFlag(EPD_BEENDONE, TRUE);
		}
	}
}

// If editing the handles, cut to the chase!
if (handleEdit)
{
edit_handles:
XFormHandles(xproc, t, partm, tmAxis, handleObject, handleIndex);
nodes.DisposeTemporary();
return;
}

// Not doing handles, just plain ol' verts
ClearPatchDataFlag(mcList, EPD_BEENDONE);	// Clear these out again
for (int i = 0; i < mcList.Count(); i++)
{
	EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
	if (!patchData)
		continue;
	if (patchData->GetFlag(EPD_BEENDONE))
		continue;
	
	// If the mesh isn't yet cache, this will cause it to get cached.
	RPatchMesh *rpatch;
	PatchMesh *patch = patchData->TempData(this)->GetPatch(t, rpatch);
	if (!patch)
		continue;
	
	// If this is the first edit, then the delta arrays will be allocated
	patchData->BeginEdit(t);
	
	// Create a change record for this object and store a pointer to its delta info in this EditPatchData
	if (!TestAFlag(A_HELD))
	{
		patchData->vdelta.SetSize(*patch, FALSE);
		if (theHold.Holding())
		{
			// Hulud: here, i pass a NULL pointer because rpatch are not modified by xform
			theHold.Put(new PatchRestore(patchData, this, patch, NULL, _T("XFormVerts")));
		}
		patchData->vdelta.Zero();		// Reset all deltas
		patchData->ClearHandleFlag();
		wasBroken = FALSE;
	}
	else 
	{
		if (wasBroken)
			shiftPressed = TRUE;
		if (patchData->DoingHandles())
			patchData->ApplyHandlesAndZero(*patch);		// Reapply the slave handle deltas
		else
			patchData->vdelta.Zero();
	}
	
	// Compute the transforms
	if (numAxis == NUMAXIS_INDIVIDUAL)
	{
		switch (selLevel)
		{
		case EP_VERTEX: 
			{
				// Selected vertices - either directly or indirectly through selected faces or edges.
				BitArray sel = patch->VertexTempSel();
				int verts = patch->numVerts;
				for (int vert = 0; vert < verts; vert++)
				{
					if (sel[vert])
					{
						tmAxis = ip->GetTransformAxis(nodes[i], vert);
						mat    = nodes[i]->GetObjectTM(t, &valid) * Inverse(tmAxis);
						imat   = Inverse(mat);
						xproc->SetMat(mat);
						
						// XForm the cache vertices
						oldpt = patch->verts[vert].p;
						newpt = xproc->proc(oldpt, mat, imat);
						patch->verts[vert].p = newpt;
						delta = newpt - oldpt;
						
						// Move the delta's vertices.
						patchData->vdelta.MoveVert(vert, delta);
						
						// Also affect its vectors
						int vecs = patch->verts[vert].vectors.Count();
						for (int vec = 0; vec < vecs; ++vec)
						{
							int index = patch->verts[vert].vectors[vec];
							// XForm the cache vertices
							oldpt = patch->vecs[index].p;
							newpt = xproc->proc(oldpt, mat, imat);
							patch->vecs[index].p = newpt;
							delta = newpt - oldpt;
							
							// Move the delta's vertices.
							patchData->vdelta.MoveVec(index, delta);
						}
					}
				}
				patch->computeInteriors();	// Kind of broad-spectrum -- only need to recompute affected patches
			}
			break;
		case EP_EDGE:
		case EP_PATCH: 
			{
				// Selected vertices - either directly or indirectly through selected faces or edges.
				BitArray sel = patch->VertexTempSel();
				int verts = patch->numVerts;
				for (int vert = 0; vert < verts; vert++)
				{
					if (sel[vert])
					{
						tmAxis = ip->GetTransformAxis(nodes[i], vert);
						mat    = nodes[i]->GetObjectTM(t, &valid) * Inverse(tmAxis);
						imat   = Inverse(mat);
						xproc->SetMat(mat);
						
						// XForm the cache vertices
						oldpt = patch->verts[vert].p;
						newpt = xproc->proc(oldpt, mat, imat);
						patch->verts[vert].p = newpt;
						delta = newpt - oldpt;
						
						// Move the delta's vertices.
						patchData->vdelta.MoveVert(vert, delta);
						
						// Also affect its vectors
						int vecs = patch->verts[vert].vectors.Count();
						for (int vec = 0; vec < vecs; ++vec)
						{
							int index = patch->verts[vert].vectors[vec];
							// XForm the cache vertices
							oldpt = patch->vecs[index].p;
							newpt = xproc->proc(oldpt, mat, imat);
							patch->vecs[index].p = newpt;
							delta = newpt - oldpt;
							
							// Move the delta's vertices.
							patchData->vdelta.MoveVec(index, delta);
						}
					}
				}
				patch->computeInteriors();
			}
			break;
		}			
	}
	else 
	{
		mat = nodes[i]->GetObjectTM(t, &valid) * Inverse(tmAxis);
		imat = Inverse(mat);
		xproc->SetMat(mat);
		
		// Selected vertices - either directly or indirectly through selected faces or edges.
		BitArray sel = patch->VertexTempSel();
		int verts = patch->numVerts;
		for (int vert = 0; vert < verts; vert++)
		{
			if (sel[vert])
			{
				// XForm the cache vertices
				oldpt = patch->verts[vert].p;
				newpt = xproc->proc(oldpt, mat, imat);
				patch->verts[vert].p = newpt;
				delta = newpt - oldpt;
				
				// Move the delta's vertices.
				patchData->vdelta.MoveVert(vert, delta);
				
				// Also affect its vectors
				int vecs = patch->verts[vert].vectors.Count();
				for (int vec = 0; vec < vecs; ++vec)
				{
					int index = patch->verts[vert].vectors[vec];
					// XForm the cache vertices
					oldpt = patch->vecs[index].p;
					newpt = xproc->proc(oldpt, mat, imat);
					patch->vecs[index].p = newpt;
					delta = newpt - oldpt;
					
					// Move the delta's vertices.
					patchData->vdelta.MoveVec(index, delta);
				}
			}
		}
		patch->computeInteriors();
	}
	rpatch->InvalidateBindingPos ();
	patchData->UpdateChanges(patch, NULL);					// Hulud: here, i pass a NULL pointer because rpatch are not modified by xform
	patchData->TempData(this)->Invalidate(PART_GEOM);
	patchData->SetFlag(EPD_BEENDONE, TRUE);
	}
	
	// Mark all objects in selection set
	SetAFlag(A_HELD);
	
	nodes.DisposeTemporary();
	ClearPatchDataFlag(mcList, EPD_BEENDONE);
	NotifyDependents(FOREVER, PART_GEOM, REFMSG_CHANGE);
}

void EditPatchMod::Move(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin)
{
	MoveXForm proc(val);
	XFormVerts(&proc, t, partm, tmAxis); 	
}

void EditPatchMod::Rotate(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin)
{
	RotateXForm proc(val);
	XFormVerts(&proc, t, partm, tmAxis); 	
}

void EditPatchMod::Scale(TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin)
{
	ScaleXForm proc(val);
	XFormVerts(&proc, t, partm, tmAxis); 	
}

void EditPatchMod::TransformStart(TimeValue t)
{
	if (ip)
		ip->LockAxisTripods(TRUE);
}

void EditPatchMod::TransformFinish(TimeValue t)
{
	if (ip)
		ip->LockAxisTripods(FALSE);
	UpdateSelectDisplay();
	
	if (!ip)
		return;	
	ModContextList mcList;
	INodeTab nodes;
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		patchData->UpdateChanges(patch, rpatch, FALSE);
	}
}

void EditPatchMod::TransformCancel(TimeValue t)
{
	if (ip)
		ip->LockAxisTripods(FALSE);
}

void EditPatchMod::ModifyObject(TimeValue t, ModContext &mc, ObjectState * os, INode *node) 
{		
	// Alert(_T("in ModifyObject"));
	nlassert(os->obj->ClassID() == RYKOLPATCHOBJ_CLASS_ID);
	// Alert(_T("ModifyObject class ID is OK"));
	
	RPO *patchOb =(RPO*)os->obj;
	EditPatchData *patchData;

	if (!mc.localData)
	{
		mc.localData = new EditPatchData(this);
		patchData =(EditPatchData*)mc.localData;
		meshSteps = patchData->meshSteps = patchOb->GetMeshSteps();
		// 3-18-99 to suport render steps and removal of the mental tesselator
		meshStepsRender = patchData->meshStepsRender = patchOb->GetMeshStepsRender();
		showInterior = patchData->showInterior = patchOb->GetShowInterior();
		
		//		meshAdaptive = patchData->meshAdaptive = patchOb->GetAdaptive();	// Future use (Not used now)
		tileLevel = patchData->tileLevel = patchOb->rpatch->rTess.TileTesselLevel;
		transitionType = patchData->transitionType = patchOb->rpatch->rTess.TransitionType;
		tileMode = patchData->tileMode = patchOb->rpatch->rTess.ModeTile;
		keepMapping = patchData->keepMapping = patchOb->rpatch->rTess.KeepMapping;
		viewTess = patchData->viewTess = patchOb->GetViewTess();
		prodTess = patchData->prodTess = patchOb->GetProdTess();
		dispTess = patchData->dispTess = patchOb->GetDispTess();
		mViewTessNormals = patchData->mViewTessNormals = patchOb->GetViewTessNormals();
		mProdTessNormals = patchData->mProdTessNormals = patchOb->GetProdTessNormals();
		mViewTessWeld = patchData->mViewTessWeld = patchOb->GetViewTessWeld();
		mProdTessWeld = patchData->mProdTessWeld = patchOb->GetProdTessWeld();
		displayLattice = patchData->displayLattice = patchOb->ShowLattice();
		displaySurface = patchData->displaySurface = patchOb->showMesh;
	} else 
	{
		patchData =(EditPatchData*)mc.localData;
	}
	
	PatchMesh &pmesh = patchOb->patch;
	nlassert(pmesh.numVerts == pmesh.vertSel.GetSize());
	nlassert(pmesh.getNumEdges() == pmesh.edgeSel.GetSize());
	nlassert(pmesh.numPatches == pmesh.patchSel.GetSize());
	
	patchData->Apply(t, patchOb, selLevel);
}

void EditPatchMod::NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc)
{
	if (mc->localData)
	{
		EditPatchData *patchData =(EditPatchData*)mc->localData;
		if (patchData)
		{
			// The FALSE parameter indicates the the mesh cache itself is
			// invalid in addition to any other caches that depend on the
			// mesh cache.
			patchData->Invalidate(partID, FALSE);
		}
	}
}

void EditPatchMod::SetDisplaySurface(BOOL sw) 
{
	sw = TRUE;
	displaySurface = sw;
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		patchData->displaySurface = sw;
		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
		}
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

void EditPatchMod::SetDisplayLattice(BOOL sw) 
{
	displayLattice = sw;
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		patchData->displayLattice = sw;
		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		if (sw)
			patch->SetDispFlag(DISP_LATTICE);
		else
			patch->ClearDispFlag(DISP_LATTICE);
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
		}
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

void EditPatchMod::SetPropagate(BOOL sw) 
{
	propagate = sw;
}

void EditPatchMod::SetMeshSteps(int steps) 
{
	meshSteps = steps;
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		
		patch->SetMeshSteps(steps);
		patchData->meshSteps = steps;
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
		}
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

void EditPatchMod::SetTileMode (bool bTile) 
{
	tileMode=bTile;
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		
		rpatch->rTess.ModeTile=bTile;
		patchData->tileMode=bTile;
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
		}
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

void EditPatchMod::SetKeepMapping (bool bKeep) 
{
	keepMapping=bKeep;
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		
		rpatch->rTess.KeepMapping=bKeep;
		patchData->keepMapping=bKeep;
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
		}
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

void EditPatchMod::SetTileSteps(int steps) 
{
	tileLevel=steps;
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		
		rpatch->rTess.TileTesselLevel=steps;
		patchData->tileLevel=steps;
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
		}
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

void EditPatchMod::SetTransitionLevel(int transition) 
{
	transitionType=transition;
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		
		rpatch->rTess.TransitionType=transition;
		patchData->transitionType=transition;
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
		}
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

// 3-18-99 to suport render steps and removal of the mental tesselator
void EditPatchMod::SetMeshStepsRender(int steps) 
{
	meshStepsRender = steps;
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		
		patch->SetMeshStepsRender(steps);
		patchData->meshStepsRender = steps;
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
		}
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

void EditPatchMod::SetShowInterior(BOOL si) 
{
	showInterior = si;
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;	
	
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			continue;
		
		patch->SetShowInterior(si);
		patchData->showInterior = si;
		if (patchData->tempData)
		{
			patchData->TempData(this)->Invalidate(PART_DISPLAY);
		}
	}
	nodes.DisposeTemporary();
	NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
	ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
}

/*
// Future use (Not used now)
void EditPatchMod::SetMeshAdaptive(BOOL sw) 
{
meshAdaptive = sw;
ModContextList mcList;
INodeTab nodes;

if (!ip)
return;	

ip->GetModContexts(mcList, nodes);

  for (int i = 0; i < mcList.Count(); i++)
  {
  EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
  if (!patchData)
  continue;		
  PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime());
  patch->SetAdaptive(sw);
  patchData->meshAdaptive = sw;
  if (patchData->tempData)
  {
  patchData->TempData(this)->Invalidate(PART_DISPLAY);
  }
  }
  NotifyDependents(FOREVER, PART_DISPLAY, REFMSG_CHANGE);
  ip->RedrawViews(ip->GetTime(), REDRAW_NORMAL);
  }
*/


int EditPatchMod::SubObjectIndex(HitRecord *hitRec)
{	
	EditPatchData *patchData =(EditPatchData*)hitRec->modContext->localData;
	if (!patchData)
		return 0;
	if (!ip)
		return 0;
	TimeValue t = ip->GetTime();
	PatchHitData *hit =(PatchHitData *)(hitRec->hitData);
	switch (selLevel)
	{
	case EP_VERTEX: 
		{
			if (hit->type != PATCH_HIT_VERTEX)
				return 0;
			int hitIndex = hit->index;
			return hitIndex;
		}
	case EP_EDGE: 
		{
			int hitIndex = hit->index;
			return hitIndex;
		}
	case EP_PATCH: 
		{
			int hitIndex = hit->index;
			return hitIndex;
		}
	case EP_TILE: 
		{
			int hitIndex = hit->index;
			return hitIndex;
		}
	default:
		return 0;
	}
}

void EditPatchMod::GetSubObjectTMs(
		SubObjAxisCallback *cb, TimeValue t, INode *node, ModContext *mc)
{
	Interval valid;
	if (mc->localData)
	{
		EditPatchData *patchData =(EditPatchData*)mc->localData;
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(t, rpatch);
		// Watch out -- The system can call us even if we didn't get a valid patch object
		if (!patch)
			return;
		
		switch (selLevel)
		{
		case EP_VERTEX: 
			{
				Matrix3 otm = node->GetObjectTM(t, &valid);
				Matrix3 tm = node->GetNodeTM(t, &valid);
				BitArray sel = patch->VertexTempSel();
				int count = sel.GetSize();
				for (int i = 0; i < count; ++i)
				{
					if (sel[i])
					{
						tm.SetTrans(patch->verts[i].p * otm);
						cb->TM(tm, i);
					}
				}
				break;
			}
		case EP_EDGE:
		case EP_PATCH: 
			{
				Matrix3 otm = node->GetObjectTM(t, &valid);
				Matrix3 tm = node->GetNodeTM(t, &valid);
				Box3 box;
				BitArray sel = patch->VertexTempSel();
				int count = sel.GetSize();
				for (int i = 0; i < count; i++)
				{
					if (sel[i])
						box += patch->verts[i].p;
				}
				tm.SetTrans(otm * box.Center());
				cb->TM(tm, 0);
				break;
			}
		case EP_TILE: 
			{
				bool bHasSel;
				Matrix3 pt=rpatch->GetSelTileTm(*patch, t, node, bHasSel);
				if (bHasSel)
					cb->TM(pt, 0);
				break;
			}
		}
	}
}

void EditPatchMod::GetSubObjectCenters(
		SubObjAxisCallback *cb, TimeValue t, INode *node, ModContext *mc)
{
	Interval valid;
	Matrix3 tm = node->GetObjectTM(t, &valid);	
	
	nlassert(ip);
	if (mc->localData)
	{	
		EditPatchData *patchData =(EditPatchData*)mc->localData;		
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);

		rpatch->UpdateBinding (*patch, t);

		// Watch out -- The system can call us even if we didn't get a valid patch object
		if (!patch)
			return;
		
		switch (selLevel)
		{
		case EP_VERTEX: 
			{
				Box3 box;
				BitArray sel = patch->VertexTempSel();
				int verts = patch->numVerts;
				for (int i = 0; i < verts; i++)
				{
					if (sel[i])
						cb->Center(patch->verts[i].p * tm, i);
				}
				break;
			}
		case EP_EDGE:
		case EP_PATCH: 
			{
				Box3 box;
				BOOL bHasSel = FALSE;
				BitArray sel = patch->VertexTempSel();
				int verts = patch->numVerts;
				for (int i = 0; i < verts; i++)
				{
					if (sel[i])
					{
						box += patch->verts[i].p * tm;
						bHasSel = TRUE;
					}
				}
				if (bHasSel)
					cb->Center(box.Center(), 0);
				break;
			}
		case EP_TILE:
			{
				bool bHasSel;
				Point3 pt=rpatch->GetSelTileCenter(*patch, t, node, bHasSel);
				if (bHasSel)
					cb->Center(pt, 0);
				break;
			}
		default:
			cb->Center(tm.GetTrans(), 0);
			break;
		}		
	}
}


BOOL EditPatchMod::DependOnTopology(ModContext &mc)
{
	EditPatchData *patchData =(EditPatchData*)mc.localData;
	if (patchData)
	{
		if (patchData->GetFlag(EPD_HASDATA))
		{
			return TRUE;
		}
	}
	return FALSE;
}

void EditPatchMod::DeletePatchDataTempData()
{
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;		
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;				
		if (patchData->tempData)
		{
			delete patchData->tempData;
		}
		patchData->tempData = NULL;
	}
	nodes.DisposeTemporary();
}


void EditPatchMod::CreatePatchDataTempData()
{
	ModContextList mcList;
	INodeTab nodes;
	
	if (!ip)
		return;		
	ip->GetModContexts(mcList, nodes);
	
	for (int i = 0; i < mcList.Count(); i++)
	{
		EditPatchData *patchData =(EditPatchData*)mcList[i]->localData;
		if (!patchData)
			continue;				
		if (!patchData->tempData)
		{
			patchData->tempData = new EPTempData(this, patchData);
		}		
	}
	nodes.DisposeTemporary();
}

//--------------------------------------------------------------

void EditPatchMod::SetRememberedVertType(int type) 
{
	if (rememberedPatch)
		ChangeRememberedVert(type);
	else
		ChangeSelVerts(type);
}

