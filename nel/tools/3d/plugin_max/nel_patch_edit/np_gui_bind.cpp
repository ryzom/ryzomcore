#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

// ------------------------------------------------------------------------------------------------------------------------------------------------------


void EPM_BindCMode::EnterMode()
	{
	if (pobj->hOpsPanel)
	{
		ICustButton *but = GetICustButton(GetDlgItem(pobj->hOpsPanel, IDC_BIND));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
		}
	}

void EPM_BindCMode::ExitMode()
	{
	if (pobj->hOpsPanel)
	{
		ICustButton *but = GetICustButton(GetDlgItem(pobj->hOpsPanel, IDC_BIND));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
		}
	}

/*-------------------------------------------------------------------*/

HCURSOR EPM_BindMouseProc::GetTransformCursor() 
	{ 
	static HCURSOR hCur = NULL;

	if (!hCur)
	{
		hCur = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_SEGREFINECUR)); 
		}

	return hCur; 
	}

BOOL EPM_BindMouseProc::HitTest(
		ViewExp *vpt, IPoint2 *p, int type, int flags, int subType)
	{
	vpt->ClearSubObjHitList();
	SetPatchHitOverride(subType);

	ip->SubObHitTest(ip->GetTime(), type, ip->GetCrossing(), flags, p, vpt);
	ClearPatchHitOverride();
	if (vpt->NumSubObjHits())
	{
		return TRUE;
	} else 
	{
		return FALSE;
		}			
	}

BOOL EPM_BindMouseProc::HitAKnot(ViewExp *vpt, IPoint2 *p, int *vert) 
{
	int first = 1;
	
	if (HitTest(vpt, p, HITTYPE_POINT, 0, 1))
	{
		HitLog &hits = vpt->GetSubObjHitList();
		HitRecord *rec = hits.First();
		DWORD best = 9999;
		HitRecord *bestRec;
		while (rec) 
		{
			PatchHitData *hit =((PatchHitData *)rec->hitData);
			// If there's an exclusion shape, this must be a part of it!
			pMesh = hit->patch;
			
			if (1)
			{
				if (hit->type == PATCH_HIT_VERTEX)
					{

				// If there's an exclusion shape, the vert & poly can't be the same!
					if (first || rec->distance < best) 
						{
						first = 0;
						best = rec->distance;
						bestRec = rec;
						}
					}
				}
			rec = rec->Next();
			}
		if (!first)
		{
			PatchHitData *hit =((PatchHitData *)bestRec->hitData);
			*vert = hit->index;
			return TRUE;
		}
	}
	return FALSE;
}


BOOL EPM_BindMouseProc::HitASegment(ViewExp *vpt, IPoint2 *p, int *seg) 
{
	int first = 1;
	
	if (HitTest(vpt, p, HITTYPE_POINT, 0, 2))
	{
		HitLog &hits = vpt->GetSubObjHitList();
		HitRecord *rec = hits.First();
		DWORD best = 9999;
		HitRecord *bestRec;
		while (rec) 
		{
			PatchHitData *hit =((PatchHitData *)rec->hitData);
			// If there's an exclusion shape, this must be a part of it!
			if (pMesh == hit->patch)
			{
				if (hit->type == PATCH_HIT_EDGE)
					{

				// If there's an exclusion shape, the vert & poly can't be the same!
					if (first || rec->distance < best) 
						{
						first = 0;
						best = rec->distance;
						bestRec = rec;
						}
					}
				}
			rec = rec->Next();
			}
		if (!first)
		{
			PatchHitData *hit =((PatchHitData *)bestRec->hitData);
			*seg = hit->index;
			return TRUE;
			}
		}
	return FALSE;
	}

static void PatchXORDottedLine(HWND hwnd, IPoint2 p0, IPoint2 p1)
{
	HDC hdc;
	hdc = GetDC(hwnd);
	SetROP2(hdc, R2_XORPEN);
	SetBkMode(hdc, TRANSPARENT);
	SelectObject(hdc, CreatePen(PS_DOT, 0, RGB(255, 255, 255)));
	MoveToEx(hdc, p0.x, p0.y, NULL);
	LineTo(hdc, p1.x, p1.y);		
	DeleteObject(SelectObject(hdc, GetStockObject(BLACK_PEN)));
	ReleaseDC(hwnd, hdc);
}


int EPM_BindMouseProc::proc(
			HWND hwnd, 
			int msg, 
			int point, 
			int flags, 
			IPoint2 m)
{
#if MAX_VERSION_MAJOR >= 19
	ViewExp *vpt = &ip->GetViewExp(hwnd);
#else
	ViewExp *vpt = ip->GetViewport(hwnd);
#endif
	int res = TRUE;
	static PatchMesh *shape1 = NULL;
	static int poly1, vert1, seg1;
	static IPoint2 anchor, lastPoint;

	switch (msg)
	{
		case MOUSE_PROPCLICK:
			ip->SetStdCommandMode(CID_OBJMOVE);
			break;

		case MOUSE_POINT:
			switch (point)
			{
				case 0:
					{
					if (HitAKnot(vpt, &m,  &vert1))
					{
						//if (rpatch->UIVertex[vert1].binded)
						{
							res = TRUE;
							anchor = lastPoint = m;
							PatchXORDottedLine(hwnd, anchor, m);	// Draw it!
	// get valid seg list
							/*knotList.SetSize(pMesh->numVerts);
							knotList.ClearAll();
							for (int i = 0; i < pMesh->numEdges; i++)
							{
								if (pMesh->edges[i].v1 == vert1) 
								{
									knotList.Set(pMesh->edges[i].v2);

								}
								if (pMesh->edges[i].v2 == vert1) 
								{
									knotList.Set(pMesh->edges[i].v1);
								}
							}*/

							tab.build (*pMesh);
						}
					}
						
					else res = FALSE;

					break;
					}
				case 1:
					PatchXORDottedLine(hwnd, anchor, lastPoint);	// Erase it!
//					if(HitAnEndpoint(vpt, &m, shape1, poly1, vert1, NULL, &poly2, &vert2))
//						ss->DoVertConnect(vpt, shape1, poly1, vert1, poly2, vert2); 
					if (HitASegment(vpt, &m,  &seg1))
					{
// if a valid segemtn change cursor
						/*int a = pMesh->edges[seg1].v1;
						int b = pMesh->edges[seg1].v2;
						if (knotList[a] && knotList[b])
							pobj->DoAddHook(pMesh, vert1, seg1);*/
						int v0, v1, v2, v3;
						switch (CheckBind (vert1, seg1, v0, v1, v2, v3, tab, *pMesh, false, true))
						{
						case 0:
							pobj->DoAddHook(pMesh, v2, vert1, v3, seg1, 0);
							break;
						case 1:
							pobj->DoAddHook(pMesh, v2, vert1, v3, seg1, 1);
							break;
						case -1:
							break;
						default:
							nlassert (0);
						}
					}
					res = FALSE;
					break;
				default:
					nlassert(0);
				}
			break;

		case MOUSE_MOVE:
			// Erase old dotted line
			PatchXORDottedLine(hwnd, anchor, lastPoint);
			// Draw new dotted line
			PatchXORDottedLine(hwnd, anchor, m);
			lastPoint = m;
			if (HitASegment(vpt, &m,  &seg1))
			{
// if a valid segemtn change cursor
				/*int a = pMesh->edges[seg1].v1;
				int b = pMesh->edges[seg1].v2;
				if (knotList[a] && knotList[b])
					SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_TH_SELCURSOR)));
				else SetCursor(LoadCursor(NULL, IDC_ARROW));*/
				int v0, v1, v2, v3;
				if (CheckBind (vert1, seg1, v0, v1, v2, v3, tab, *pMesh, false, true)!=-1)
					SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_TH_SELCURSOR)));
				else 
					SetCursor(LoadCursor(NULL, IDC_ARROW));
			}
			else 
			{
				SetCursor(LoadCursor(NULL, IDC_ARROW));
			}

			break;
					
		case MOUSE_FREEMOVE:
			if (HitAKnot(vpt, &m,  &vert1))
				{
				SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_TH_SELCURSOR)));
				}
			else 
			{
				SetCursor(LoadCursor(NULL, IDC_ARROW));
				}
/*
			if (HitTest(vpt, &m, HITTYPE_POINT, HIT_ABORTONHIT, 1))
			{
				HitLog &hits = vpt->GetSubObjHitList();
				HitRecord *rec = hits.First();
				if (rec)
					{
					SetCursor(LoadCursor(hInstance, MAKEINTRESOURCE(IDC_TH_SELCURSOR)));
					}
				}
			SetCursor(LoadCursor(NULL, IDC_ARROW));
*/
			break;
		
		case MOUSE_ABORT:
			// Erase old dotted line
			PatchXORDottedLine(hwnd, anchor, lastPoint);
			break;			
		}

#if MAX_VERSION_MAJOR < 19
		if (vpt)
		ip->ReleaseViewport(vpt);
#endif

	return res;
}

/*-------------------------------------------------------------------*/

