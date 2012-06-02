#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

#define PROMPT_TIME	2000

// ------------------------------------------------------------------------------------------------------------------------------------------------------

extern HIMAGELIST hFaceImages;
int patchHitOverride = 0;	// If zero, no override is done

// ------------------------------------------------------------------------------------------------------------------------------------------------------

class EPImageListDestroyer 
{
	~EPImageListDestroyer() 
	{
		if (hFaceImages)
			ImageList_Destroy(hFaceImages);
	}
};

PatchRightMenu pMenu;

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void LoadImages() 
{
	if (hFaceImages)
		return;
	
	HBITMAP hBitmap, hMask;
	hFaceImages = ImageList_Create(24, 23, ILC_COLOR | ILC_MASK, 6, 0);
	hBitmap     = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_PATCHSELTYPES));
	hMask       = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_PATCHSELMASK));
	ImageList_Add(hFaceImages, hBitmap, hMask);
	DeleteObject(hBitmap);
	DeleteObject(hMask);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void PatchRightMenu::Init(RightClickMenuManager* manager, HWND hWnd, IPoint2 m) 
{
	switch (ep->GetSubobjectLevel())
	{
	case EP_VERTEX:
		if (ep->RememberVertThere(hWnd, m))
		{
			int oldType = -1;
			int flags1, flags2;
			flags1 = flags2 = MF_STRING;
			switch (ep->rememberedData)
			{
			case PVERT_COPLANAR:
				flags1 |= MF_CHECKED;
				break;
			case 0:
				flags2 |= MF_CHECKED;
				break;
			}
			manager->AddMenu(this, MF_SEPARATOR, 0, NULL);
			manager->AddMenu(this, flags1, PVERT_COPLANAR, GetString(IDS_TH_COPLANAR));
			manager->AddMenu(this, flags2, 0, GetString(IDS_TH_CORNER));
		}
		break;
	case EP_PATCH:
		if (ep->RememberPatchThere(hWnd, m))
		{
			int oldType = -1;
			int flags1, flags2;
			flags1 = flags2 = MF_STRING;
			switch (ep->rememberedData)
			{
			case PATCH_AUTO:
				flags1 |= MF_CHECKED;
				break;
			case 0:
				flags2 |= MF_CHECKED;
				break;
			}
			manager->AddMenu(this, MF_SEPARATOR, 0, NULL);
			manager->AddMenu(this, flags1, PATCH_AUTO, GetString(IDS_TH_AUTOINTERIOR));
			manager->AddMenu(this, flags2, 0, GetString(IDS_TH_MANUALINTERIOR));
		}
		break;
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

void PatchRightMenu::Selected(UINT id) 
{
	switch (ep->GetSubobjectLevel())
	{
	case EP_VERTEX:
		ep->SetRememberedVertType((int)id);
		break;
	case EP_PATCH:
		ep->SetRememberedPatchType((int)id);
		break;
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------------

int EditPatchMod::HitTest(TimeValue t, INode* inode, int type, int crossing, 
		int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) 
{
	Interval valid;
	int savedLimits, res = 0;
	GraphicsWindow *gw = vpt->getGW();
	HitRegion hr;
	MakeHitRegion(hr, type, crossing, 4, p);
	gw->setHitRegion(&hr);
	Matrix3 mat = inode->GetObjectTM(t);
	gw->setTransform(mat);	
	gw->setRndLimits(((savedLimits = gw->getRndLimits()) | GW_PICK) & ~GW_ILLUM);
	gw->clearHitCode();
	
	if (mc->localData)
	{		
		EditPatchData *patchData =(EditPatchData*)mc->localData;
		RPatchMesh *rpatch;
		PatchMesh *patch = patchData->TempData(this)->GetPatch(ip->GetTime(), rpatch);
		if (!patch)
			return FALSE;
		
		SubPatchHitList hitList;
		PatchSubHitRec *rec;

		rpatch->UpdateBinding (*patch, t);

		if (selLevel!=EP_TILE)
		{
			res = patch->SubObjectHitTest(gw, gw->getMaterial(), &hr,
				flags | ((patchHitOverride) ? patchHitLevel[patchHitOverride] : patchHitLevel[selLevel]), hitList);
		}
		else
		{
			res = rpatch->SubObjectHitTest(gw, gw->getMaterial(), &hr,
				flags | ((patchHitOverride) ? patchHitLevel[patchHitOverride] : patchHitLevel[selLevel]), hitList,
				t, *patch);
		}
		
		rec = hitList.First();
		while (rec) 
		{
			vpt->LogHit(inode, mc, rec->dist, 123456, new PatchHitData(rec->patch, rec->index, rec->type));
			rec = rec->Next();
		}
	}
	
	gw->setRndLimits(savedLimits);	
	return res;
}
