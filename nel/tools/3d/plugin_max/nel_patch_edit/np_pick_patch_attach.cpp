#include "stdafx.h"
#include "editpat.h"

// ------------------------------------------------------------------------------------------------------------------------------------------------------

#define DBGWELD_DUMPx
#define DBGWELD_ACTIONx
#define DBG_NAMEDSELSx

// ------------------------------------------------------------------------------------------------------------------------------------------------------

BOOL PickPatchAttach::Filter(INode *node)
{
	ModContextList mcList;		
	INodeTab nodes;
	if (node)
	{
		// Make sure the node does not depend on us
		node->BeginDependencyTest();
		ep->NotifyDependents(FOREVER, 0, REFMSG_TEST_DEPENDENCY);
		if (node->EndDependencyTest())
			return FALSE;
		
		ObjectState os = node->GetObjectRef()->Eval(ep->ip->GetTime());
		GeomObject *object =(GeomObject *)os.obj;
		// Make sure it isn't one of the nodes we're editing, for heaven's sake!
		ep->ip->GetModContexts(mcList, nodes);
		int numNodes = nodes.Count();
		for (int i = 0; i < numNodes; ++i)
		{
			if (nodes[i] == node)
			{
				nodes.DisposeTemporary();
				return FALSE;
			}
		}
		if (object->CanConvertToType(RYKOLPATCHOBJ_CLASS_ID))
		{
			nodes.DisposeTemporary();
			return TRUE;
		}
	}
	nodes.DisposeTemporary();
	return FALSE;
}

BOOL PickPatchAttach::HitTest(
		IObjParam *ip, HWND hWnd, ViewExp *vpt, IPoint2 m, int flags)
{	
	INode *node = ip->PickNode(hWnd, m, this);
	ModContextList mcList;		
	INodeTab nodes;
	
	if (node)
	{
		ObjectState os = node->GetObjectRef()->Eval(ip->GetTime());
		GeomObject *object =(GeomObject *)os.obj;
		// Make sure it isn't one of the nodes we're editing, for heaven's sake!
		ep->ip->GetModContexts(mcList, nodes);
		int numNodes = nodes.Count();
		for (int i = 0; i < numNodes; ++i)
		{
			if (nodes[i] == node)
			{
				nodes.DisposeTemporary();
				return FALSE;
			}
		}
		if (object->CanConvertToType(RYKOLPATCHOBJ_CLASS_ID))
		{
			nodes.DisposeTemporary();
			return TRUE;
		}
	}
	
	nodes.DisposeTemporary();
	return FALSE;
}

BOOL PickPatchAttach::Pick(IObjParam *ip, ViewExp *vpt)
{
	INode *node = vpt->GetClosestHit();
	nlassert(node);
	GeomObject *object =(GeomObject *)node->GetObjectRef()->Eval(ip->GetTime()).obj;
	if (object->CanConvertToType(RYKOLPATCHOBJ_CLASS_ID))
	{
		RPO *attPatch =(RPO *)object->ConvertToType(ip->GetTime(), RYKOLPATCHOBJ_CLASS_ID);
		if (attPatch)
		{
			PatchMesh patch = attPatch->patch;
			RPatchMesh rpatch = *attPatch->rpatch;
			ModContextList mcList;
			INodeTab nodes;
			ip->GetModContexts(mcList, nodes);
			BOOL res = TRUE;
			if (nodes[0]->GetMtl() && node->GetMtl() &&(nodes[0]->GetMtl() != node->GetMtl()))
				res = DoAttachMatOptionDialog(ep->ip, ep);
			if (res)
			{
				bool canUndo = TRUE;
				ep->DoAttach(node, &patch, &rpatch, canUndo);
				if (!canUndo)
					GetSystemSetting(SYSSET_CLEAR_UNDO);
			}
			nodes.DisposeTemporary();
			// Discard the copy it made, if it isn't the same as the object itself
			if (attPatch !=(PatchObject *)object)
				delete attPatch;
		}
	}
	return FALSE;
}


void PickPatchAttach::EnterMode(IObjParam *ip)
{
	if (ep->hOpsPanel)
	{
		ICustButton *but = GetICustButton(GetDlgItem(ep->hOpsPanel, IDC_ATTACH));
		but->SetCheck(TRUE);
		ReleaseICustButton(but);
	}
}

void PickPatchAttach::ExitMode(IObjParam *ip)
{
	if (ep->hOpsPanel)
	{
		ICustButton *but = GetICustButton(GetDlgItem(ep->hOpsPanel, IDC_ATTACH));
		but->SetCheck(FALSE);
		ReleaseICustButton(but);
	}
}

HCURSOR PickPatchAttach::GetHitCursor(IObjParam *ip) 
{
	return LoadCursor(hInstance, MAKEINTRESOURCE(IDC_ATTACHCUR));
}
