// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


// particle_tree_ctrl.cpp : implementation file
//



#include "std_afx.h"
#include "object_viewer.h"
#include "particle_tree_ctrl.h"
#include "located_bindable_dialog.h"
#include "emitter_dlg.h"
#include "main_frame.h"
#include "particle_system_edit.h"
#include "particle_dlg.h"
#include "start_stop_particle_system.h"
#include "edit_ps_sound.h"
#include "edit_ps_light.h"
#include "dup_ps.h"
#include "object_viewer.h"
#include "ps_mover_dlg.h"
#include "set_value_dlg.h"
#include "create_file_dlg.h"
#include "located_properties.h"
#include "located_bindable_dialog.h"
#include "located_target_dlg.h"
#include "lb_extern_id_dlg.h"
#include "skippable_message_box.h"
// 
#include "nel/3d/particle_system.h"
#include "nel/3d/particle_system_model.h"
#include "nel/3d/particle_system_shape.h"
#include "nel/3d/ps_located.h"
#include "nel/3d/ps_particle.h"
#include "nel/3d/ps_mesh.h"
#include "nel/3d/ps_force.h"
#include "nel/3d/ps_zone.h"
#include "nel/3d/ps_sound.h"
#include "nel/3d/ps_emitter.h"
#include "nel/3d/ps_edit.h"
#include "nel/3d/nelu.h"
//
#include "nel/misc/path.h"
#include "nel/misc/file.h"







using NL3D::CParticleSystem;
using NL3D::CParticleSystemModel;
using NL3D::CPSLocated;
using NL3D::CPSLocatedBindable;
using NL3D::CNELU;





/////////////////////////////////////////////////////////////////////////////
// CParticleTreeCtrl


enum TPSIcon 
{
	PSIconForce			          = 0, 
	PSIconParticle		          = 1, 
	PSIconEmitter		          = 2, 
	PSIconLight                   = 3, 
	PSIconCollisionZone           = 4, 
	PSIconSound                   = 5, 
	PSIconParticleSystem          = 6, 
	PSIconLocated		          = 7,
	PSIconLocatedInstance         = 8,
	PSIconWorkspace				  = 9,
	PSIconParticleSystemNotLoaded = 10
};

static const uint IconIDs[] = 
{ 
	IDB_FORCE, 
	IDB_PARTICLE, 
	IDB_EMITTER, 
	IDB_LIGHT, 
	IDB_COLLISION_ZONE, 
	IDB_SOUND, 
	IDB_PARTICLE_SYSTEM, 
	IDB_LOCATED, 
	IDB_LOCATED_INSTANCE, 
	IDB_PS_WORKSPACE, 
	IDB_PARTICLE_SYSTEM_NOT_LOADED
};
static const uint NumIconIDs = sizeof(IconIDs) / sizeof(uint);

// this map is used to create increasing names
static std::map<std::string,  uint> _PSElementIdentifiers;


//****************************************************************************************************************
CParticleTreeCtrl::CParticleTreeCtrl(CParticleDlg *pdlg)
{
	CBitmap bm[NumIconIDs];
	_ImageList.Create(16,  16,  ILC_COLOR4,  0,  NumIconIDs);
	for (uint k = 0; k  < NumIconIDs; ++k)
	{
		bm[k].LoadBitmap(IconIDs[k]);
		_ImageList.Add(&bm[k],  RGB(1,  1,  1));
	}
	_ParticleDlg = pdlg;
	_LastClickedPS = NULL;
	_LastActiveNode = NULL;
	_ViewFilenameFlag = true;
}

//****************************************************************************************************************
CParticleTreeCtrl::~CParticleTreeCtrl()
{
	reset();	
}

//****************************************************************************************************************
void CParticleTreeCtrl::reset()
{
	if (IsWindow(*this))
	{	
		DeleteAllItems();
	}
	for (std::vector<CNodeType *>::iterator it = _NodeTypes.begin(); it != _NodeTypes.end(); ++it)
	{
		delete *it;
	}
	_NodeTypes.clear();
}

//****************************************************************************************************************
void CParticleTreeCtrl::rebuildLocatedInstance(CParticleWorkspace::CNode &node)
{
	HTREEITEM currPS = getTreeItem(&node);
	nlassert(currPS);
	HTREEITEM currLocated = this->GetChildItem(currPS);
	while(currLocated)
	{
		CNodeType *nt = (CNodeType *) GetItemData(currLocated);
		nlassert(nt->Type == CNodeType::located);
		CPSLocated *loc = nt->Loc;
		for (uint32 k = 0; k < loc->getSize(); ++k)
		{
			CNodeType *newNt = new CNodeType(loc,  k);
			_NodeTypes.push_back(newNt);
			// bind located instance icon
			InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT,  _T("instance"),  PSIconLocatedInstance,  PSIconLocatedInstance,  0,  0,  (LPARAM) newNt,  currLocated,  TVI_LAST);
		}
		currLocated = GetNextItem(currLocated,  TVGN_NEXT);
	}
	Invalidate();
}

//=====================================================================================================
void CParticleTreeCtrl::suppressLocatedInstanceNbItem(CParticleWorkspace::CNode &node,  uint32 newSize)
{
	HTREEITEM currPS = getTreeItem(&node);
	nlassert(currPS);
	HTREEITEM currLocated,  currLocElement,  nextCurrLocElement;
	currLocated = this->GetChildItem(currPS);
	while(currLocated)
	{				
		currLocElement = GetChildItem(currLocated);			
		while (currLocElement)
		{
			CNodeType *nt = (CNodeType *) GetItemData(currLocElement);

			nextCurrLocElement = GetNextItem(currLocElement,  TVGN_NEXT);
			// remove instance item
			if (nt->Type == CNodeType::locatedInstance)
			{
				if (nt->LocatedInstanceIndex >= newSize)
				{
					removeTreePart(currLocElement);					
				}
			}
			currLocElement = nextCurrLocElement;		
		}
		
		currLocated = GetNextItem(currLocated,  TVGN_NEXT);
	}	
	Invalidate();
}


//=====================================================================================================
HTREEITEM CParticleTreeCtrl::buildTreeFromPS(CParticleWorkspace::CNode &node,  HTREEITEM rootHandle,  HTREEITEM prevSibling /*= TVI_LAST*/)
{		
	// for now,  there's only one root ...			
	CNodeType *nt = new CNodeType(&node);
	_NodeTypes.push_back(nt);
	if (node.isLoaded())
	{	
		// bind particle system icon
		HTREEITEM psRoot = InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT, nlUtf8ToTStr(computeCaption(node)), PSIconParticleSystem, PSIconParticleSystem, 0, 0, NULL, rootHandle, prevSibling);
		// set the param (doesn't seems to work during first creation)
		SetItemData(psRoot,  (LPARAM) nt);
		// now,  create each located		
		for (uint k = 0; k < node.getPSPointer()->getNbProcess(); k++)
		{				
			CPSLocated *loc = dynamic_cast<CPSLocated *>(node.getPSPointer()->getProcess(k));		
			if (loc) createNodeFromLocated(loc,  psRoot);
		}
		rebuildLocatedInstance(node);
		return psRoot;
	}
	else
	{
		// bind a bitmap that say that the PS hasn't been loaded
		HTREEITEM psRoot = InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT, nlUtf8ToTStr(computeCaption(node)), PSIconParticleSystemNotLoaded, PSIconParticleSystemNotLoaded, 0, 0, NULL, rootHandle, prevSibling);
		SetItemData(psRoot,  (LPARAM) nt);
		return psRoot;
	}
}

//=====================================================================================================
void CParticleTreeCtrl::buildTreeFromWorkSpace(CParticleWorkspace &ws)
{
	reset();
	DeleteAllItems();
	CNodeType *nt = new CNodeType(&ws);
	_NodeTypes.push_back(nt);
	// bind particle system icon
	HTREEITEM rootHandle = InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT, nlUtf8ToTStr(computeCaption(ws)), PSIconWorkspace, PSIconWorkspace, 0, 0, NULL, NULL, TVI_LAST);
	// set the param (doesn't seems to work during first creation)
	SetItemData(rootHandle,  (LPARAM) nt);
	// now,  create each particle system
	for (uint k = 0; k < ws.getNumNode(); ++k)
	{				
		buildTreeFromPS(*ws.getNode(k),  rootHandle);
	}
}

//=====================================================================================================
void CParticleTreeCtrl::createNodeFromLocated(NL3D::CPSLocated *loc,  HTREEITEM rootHandle)
{	
	// insert an item for the located
	CNodeType *nt = new CNodeType(loc);
	_NodeTypes.push_back(nt);
	// bind located icon
	HTREEITEM nodeHandle = InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT, nlUtf8ToTStr(loc->getName()), PSIconLocated, PSIconLocated, 0, 0, (LPARAM)nt, rootHandle, TVI_LAST);
	// now,  insert each object that is bound to the located	
	for (uint l = 0; l < loc->getNbBoundObjects(); ++l)
	{
		createNodeFromLocatedBindable(loc->getBoundObject(l),  nodeHandle);				
	}
}

//=====================================================================================================
void CParticleTreeCtrl::createNodeFromLocatedBindable(NL3D::CPSLocatedBindable *lb,  HTREEITEM rootHandle)
{
	// we ordered the image so that they match the type for a located bindable (force,  particles,  collision zones...)
	CNodeType *nt = new CNodeType(lb);
	_NodeTypes.push_back(nt);
	InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT, nlUtf8ToTStr(lb->getName()), lb->getType(), lb->getType(), PSIconForce, PSIconForce, (LPARAM)nt, rootHandle, TVI_LAST);
}



BEGIN_MESSAGE_MAP(CParticleTreeCtrl,  CTreeCtrl)
	//{{AFX_MSG_MAP(CParticleTreeCtrl)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED,  OnSelchanged)
	ON_WM_RBUTTONDOWN()
	ON_NOTIFY_REFLECT(TVN_ENDLABELEDIT,  OnEndlabeledit)
	ON_NOTIFY_REFLECT(TVN_BEGINLABELEDIT, OnBeginlabeledit)
	ON_WM_LBUTTONDBLCLK()
	ON_NOTIFY_REFLECT(TVN_KEYDOWN, OnKeydown)
	ON_WM_KEYUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=====================================================================================================
void CParticleTreeCtrl::init(void)
{
	this->SetImageList(&_ImageList,  TVSIL_NORMAL);
}

//=====================================================================================================
void CParticleTreeCtrl::updateRightPane(CNodeType &nt)
{
	switch (nt.Type)
	{
		case CNodeType::located:
		{	
			nlassert(getOwnerNode(&nt));
			CLocatedProperties *lp = new CLocatedProperties(getOwnerNode(&nt), nt.Loc,  _ParticleDlg);	
			lp->init(0,  0);	
			_ParticleDlg->setRightPane(lp);
			if (_LastClickedPS)
			{
				_LastClickedPS->setCurrentEditedElement(NULL);
			}
			return;
		}
		break;
		case CNodeType::locatedBindable:
		{
			if (dynamic_cast<NL3D::CPSEmitter *>(nt.Bind))
			{
				CEmitterDlg *ed =  new CEmitterDlg(getOwnerNode(&nt), static_cast<NL3D::CPSEmitter *>(nt.Bind),  _ParticleDlg);
				ed->init(_ParticleDlg);
				_ParticleDlg->setRightPane(ed);
			}
			else
			if (dynamic_cast<NL3D::CPSTargetLocatedBindable *>(nt.Bind))
			{
				CLocatedTargetDlg *ltd =  new CLocatedTargetDlg(getOwnerNode(&nt),  static_cast<NL3D::CPSTargetLocatedBindable *>(nt.Bind),  _ParticleDlg);
				ltd->init(_ParticleDlg);
				_ParticleDlg->setRightPane(ltd);
			}
			else
			if (dynamic_cast<NL3D::CPSSound *>(nt.Bind))
			{
				nlassert(getOwnerNode(&nt));
				CEditPSSound *epss =  new CEditPSSound(getOwnerNode(&nt),  static_cast<NL3D::CPSSound *>(nt.Bind));
				epss->init(_ParticleDlg);
				_ParticleDlg->setRightPane(epss);
			}
			else
			if (dynamic_cast<NL3D::CPSLight *>(nt.Bind))
			{
				nlassert(getOwnerNode(&nt));
				CEditPSLight *epsl =  new CEditPSLight(getOwnerNode(&nt),  static_cast<NL3D::CPSLight *>(nt.Bind));
				epsl->init(_ParticleDlg);
				_ParticleDlg->setRightPane(epsl);
			}
			else
			{
				nlassert(getOwnerNode(&nt));
				CLocatedBindableDialog *lbd = new CLocatedBindableDialog(getOwnerNode(&nt), nt.Bind);
				lbd->init(_ParticleDlg);
				_ParticleDlg->setRightPane(lbd);			
			}
			if (_LastClickedPS)
			{
				_LastClickedPS->setCurrentEditedElement(NULL);
			}
			return;
		}
		break;
		case CNodeType::particleSystem:
		{
			// see if the particle system has been loaded
			if (nt.PS->isLoaded())
			{			
				CParticleSystemEdit *pse = new CParticleSystemEdit(nt.PS,  this);
				pse->init(_ParticleDlg);
				_ParticleDlg->setRightPane(pse);
			}
			else
			{
				_ParticleDlg->setRightPane(NULL);
			}
			if (_LastClickedPS)
			{
				_LastClickedPS->setCurrentEditedElement(NULL);
			}
		}
		break;
		case CNodeType::locatedInstance:
		{			
			NL3D::CParticleSystem *ps = nt.Loc->getOwner();									
			_LastClickedPS = ps;			
			CPSMoverDlg *moverDlg = new CPSMoverDlg(getOwnerNode(&nt),  this,  &_ParticleDlg->MainFrame->ObjView->getMouseListener(),  nt.Loc,  nt.LocatedInstanceIndex);			
			moverDlg->init(_ParticleDlg);
			_ParticleDlg->setRightPane(moverDlg);
			CObjectViewer *ov = _ParticleDlg->MainFrame->ObjView;
			if(_ParticleDlg->MainFrame->isMoveElement())
			{
				ov->getMouseListener().setModelMatrix(_ParticleDlg->getElementMatrix());
			}
			ps->setCurrentEditedElement(nt.Loc,  nt.LocatedInstanceIndex,  moverDlg->getLocatedBindable());			
		}
		break;
		case CNodeType::workspace:
		{
			if (_LastClickedPS)
			{
				_LastClickedPS->setCurrentEditedElement(NULL);
			}
			_ParticleDlg->setRightPane(NULL);
		}
		break;
	}		
}

/////////////////////////////////////////////////////////////////////////////
// CParticleTreeCtrl message handlers


void CParticleTreeCtrl::OnSelchanged(NMHDR* pNMHDR,  LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;	
	*pResult = 0;
	CNodeType *nt = (CNodeType  *) pNMTreeView->itemNew.lParam;	
	nlassert(nt);
	updateRightPane(*nt);
}

//****************************************************************************************************************
void CParticleTreeCtrl::OnRButtonDown(UINT nFlags,  CPoint point) 
{

	if (_LastClickedPS)
	{
		_LastClickedPS->setCurrentEditedElement(NULL);
	}
	// test whether there is an item under that point
	UINT flags;
	HTREEITEM item  = this->HitTest(point,  &flags);
	if (item)
	{
		this->SelectItem(item);
		RECT r;
		GetWindowRect(&r);

		CMenu  menu;
		CMenu* subMenu;

		TVITEM item;
		item.mask = TVIF_HANDLE | TVIF_PARAM;
		item.hItem = GetSelectedItem();
		GetItem(&item);

		CNodeType *nt =  (CNodeType *) item.lParam;
		UINT bIsRunning = MF_BYCOMMAND | MF_DISABLED | MF_GRAYED;
		CParticleWorkspace::CNode *node = getOwnerNode(nt);
		if (node)
		{		
			if (!node->isStateMemorized() && _ParticleDlg->StartStopDlg->getState() != CStartStopParticleSystem::RunningMultiple)
			{			
				bIsRunning = MF_ENABLED;
			}
		}

		switch (nt->Type)
		{
			case CNodeType::located:
			{
				 menu.LoadMenu(IDR_LOCATED_MENU);
				 menu.EnableMenuItem(ID_INSTANCIATE_LOCATED,   bIsRunning);
				 menu.EnableMenuItem(IDM_COPY_LOCATED,   bIsRunning);
				 menu.EnableMenuItem(IDM_PASTE_BINDABLE,   bIsRunning);
			}
			break;
			case CNodeType::locatedBindable:
				menu.LoadMenu(IDR_LOCATED_BINDABLE_MENU);		 				
				menu.GetSubMenu(0)->CheckMenuItem(IDM_LB_LOD1N2,  MF_UNCHECKED | MF_BYCOMMAND);
				menu.GetSubMenu(0)->CheckMenuItem(IDM_LB_LOD1,  MF_UNCHECKED | MF_BYCOMMAND);
				menu.GetSubMenu(0)->CheckMenuItem(IDM_LB_LOD2,  MF_UNCHECKED | MF_BYCOMMAND);
				// check the menu to tell which lod is used for this located bindable
				if (nt->Bind->getLOD() == NL3D::PSLod1n2) menu.GetSubMenu(0)->CheckMenuItem(IDM_LB_LOD1N2,  MF_CHECKED | MF_BYCOMMAND);
				if (nt->Bind->getLOD() == NL3D::PSLod1) menu.GetSubMenu(0)->CheckMenuItem(IDM_LB_LOD1,  MF_CHECKED | MF_BYCOMMAND);
				if (nt->Bind->getLOD() == NL3D::PSLod2) menu.GetSubMenu(0)->CheckMenuItem(IDM_LB_LOD2,  MF_CHECKED | MF_BYCOMMAND);

				 menu.EnableMenuItem(IDM_COPY_BINDABLE,   bIsRunning);
			break;
			case CNodeType::particleSystem:
				if (nt->PS->isLoaded())
				{
					 menu.LoadMenu(IDR_PARTICLE_SYSTEM_MENU);	
 					 menu.EnableMenuItem(IDM_PASTE_LOCATED,   bIsRunning);
				}
				else
				{
					menu.LoadMenu(IDR_PARTICLE_SYSTEM_NOT_LOADED_MENU);
				}
			break;
			case CNodeType::locatedInstance:
				menu.LoadMenu(IDR_LOCATED_INSTANCE_MENU);
			break;
			case CNodeType::workspace:
				menu.LoadMenu(IDR_PARTICLE_WORKSPACE_MENU);
			break;
			default:
				return;
			break;
		}		
		subMenu = menu.GetSubMenu(0);    
		nlassert(subMenu);
		subMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,  r.left + point.x,  r.top + point.y,  this);
	}	
}

//****************************************************************************************************************
void CParticleTreeCtrl::deleteSelection()
{
	if (!GetSelectedItem()) return;
	CNodeType *nt = (CNodeType *) GetItemData(GetSelectedItem());
	switch(nt->Type)
	{
		case CNodeType::located:
		{			
			_ParticleDlg->setRightPane(NULL);			
			CPSLocated *loc = nt->Loc;
			touchPSState(nt);
			CParticleWorkspace::CNode *ownerNode = getOwnerNode(nt);
			nlassert(ownerNode);
			ownerNode->setModified(true);
			// if the system is running,  we must destroy initial infos about the located, 
			// as they won't need to be restored when the stop button will be pressed			
			ownerNode->removeLocated(loc);
			_ParticleDlg->StartStopDlg->resetAutoCount(getOwnerNode(nt));
			nt->getOwnerPS()->remove(loc);	
			removeTreePart(GetSelectedItem());
		}
		break;
		case CNodeType::particleSystem:
		{		
			if (GetSelectedItem() == _LastActiveNode)
			{
				setActiveNode(NULL);
				_ParticleDlg->setActiveNode(NULL);
			}
			nt->PS->getWorkspace()->removeNode(nt->PS);
			removeTreePart(GetSelectedItem());
			_ParticleDlg->setRightPane(NULL);
		}
		break;
		case CNodeType::locatedBindable:
		{			
			CPSLocatedBindable *lb = nt->Bind;
			touchPSState(nt);
			CParticleWorkspace::CNode *ownerNode = getOwnerNode(nt);
			nlassert(ownerNode);
			// if the system is running,  we must destroy initial infos 
			// that what saved about the located bindable,  when the start button was pressed,  as they won't need
			// to be restored
			ownerNode->removeLocatedBindable(lb);
			ownerNode->setModified(true);
			_ParticleDlg->StartStopDlg->resetAutoCount(getOwnerNode(nt));
			lb->getOwner()->remove(lb);						
			removeTreePart(GetSelectedItem());
			_ParticleDlg->setRightPane(NULL);
		}
		break;
		case CNodeType::locatedInstance:
		{
			nlassert(nt->Type == CNodeType::locatedInstance);
			nlassert(getOwnerNode(nt));
			getOwnerNode(nt)->setModified(true);
			_ParticleDlg->StartStopDlg->resetAutoCount(getOwnerNode(nt));
			CParticleWorkspace::CNode *owner = getOwnerNode(nt);
			nlassert(owner);
			//suppressLocatedInstanceNbItem(*owner);
			NL3D::CPSEmitter::setBypassEmitOnDeath(true);
			nt->Loc->deleteElement(nt->LocatedInstanceIndex);					
			NL3D::CPSEmitter::setBypassEmitOnDeath(false);
			CParticleWorkspace::CNode *ownerNode = getOwnerNode(nt);
			nlassert(ownerNode);
			// Move selection to parent
			HTREEITEM currItem = GetSelectedItem();
			SelectItem(GetParentItem(GetSelectedItem()));
			removeTreePart(currItem);
			suppressLocatedInstanceNbItem(*ownerNode, 0);
			rebuildLocatedInstance(*ownerNode);			
		}
		break;
		case CNodeType::workspace:
			// no-op -> close the workpsace
		break;
		default:
			nlassert(0);
		break;
	}
}

//****************************************************************************************************************
BOOL CParticleTreeCtrl::OnCmdMsg(UINT nID,  int nCode,  void* pExtra,  AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	if (nCode != 0) return CTreeCtrl::OnCmdMsg(nID,  nCode,  pExtra,  pHandlerInfo);
	CPSLocatedBindable *toCreate = NULL;	
	CNodeType *nt = (CNodeType *) GetItemData(GetSelectedItem());
	bool	  createLocAndBindable = false; // when set to true,  must create a located and a simultaneously a located bindable		
	switch(nID)
	{
			///////////////
		// workspace //
		///////////////
		case IDM_INSERT_NEW_PS:
			nlassert(nt->Type == CNodeType::workspace);
			insertNewPS(*nt->WS);
		break;
		case IDM_CREATE_NEW_PS:
			nlassert(nt->Type == CNodeType::workspace);
			createNewPS(*nt->WS);
		break;
		case IDM_REMOVE_ALL_PS:
			removeAllPS(*nt->WS);
		break;
		case IDM_PS_SORT_BY_FILENAME:
		{
			nlassert(nt->Type == CNodeType::workspace);
			struct CSortByFilename : public CParticleWorkspace::ISort
			{
				bool less(const CParticleWorkspace::CNode &lhs, const CParticleWorkspace::CNode &rhs) const
				{					
					return lhs.getFilename() < rhs.getFilename();					
				}
			};
			sortWorkspace(*nt->WS, CSortByFilename());
		}			
		break;
		case IDM_PS_SORT_BY_NAME:
		{
			nlassert(nt->Type == CNodeType::workspace);
			struct CSortByName : public CParticleWorkspace::ISort
			{
				bool less(const CParticleWorkspace::CNode &lhs, const CParticleWorkspace::CNode &rhs) const
				{					
					if (!rhs.isLoaded())
					{
						if (lhs.isLoaded()) return true;
						return lhs.getFilename() < rhs.getFilename();
					}
					if (!lhs.isLoaded())
					{
						if (rhs.isLoaded()) return false;
					}
					return lhs.getPSPointer()->getName() < rhs.getPSPointer()->getName();
				}
			};
			sortWorkspace(*nt->WS, CSortByName());
		}
		break;
		case IDM_SAVE_PS_WORKSPACE:
		{
			_ParticleDlg->OnSavePsWorkspace();
		}
		break;
		case IDM_LOAD_PS_WORKSPACE:
		{
			_ParticleDlg->OnLoadPSWorkspace();
		}
		break;
		///////////////
		// particles //
		///////////////
		case IDM_DOT_LOC: createLocAndBindable = true;
		case IDM_DOT:
			toCreate = new NL3D::CPSDot;
		break;
		case IDM_LOOKAT_LOC:  createLocAndBindable = true;
		case IDM_LOOKAT:
			toCreate = new NL3D::CPSFaceLookAt; 
		break;
		case IDM_FANLIGHT_LOC:  createLocAndBindable = true;
		case IDM_FANLIGHT:
			toCreate = new NL3D::CPSFanLight;
		break;
		case IDM_RIBBON_LOC:  createLocAndBindable = true;
		case IDM_RIBBON:
			toCreate = new NL3D::CPSRibbon;
		break;
		case IDM_TAILDOT_LOC: createLocAndBindable = true;
		case IDM_TAILDOT:
			toCreate = new NL3D::CPSTailDot; 
		break;
		case IDM_MESH_LOC:  createLocAndBindable = true;
		case IDM_MESH:
			toCreate = new NL3D::CPSMesh;			
		break;
		case IDM_CONSTRAINT_MESH_LOC:  createLocAndBindable = true;
		case IDM_CONSTRAINT_MESH:
			toCreate = new NL3D::CPSConstraintMesh;
		break;
		case IDM_FACE_LOC:  createLocAndBindable = true;
		case IDM_FACE:
			toCreate = new NL3D::CPSFace;
		break;
		case IDM_SHOCKWAVE_LOC:  createLocAndBindable = true;
		case IDM_SHOCKWAVE:
			toCreate = new NL3D::CPSShockWave;
		break;
		case IDM_RIBBON_LOOK_AT_LOC:  createLocAndBindable = true;
		case IDM_RIBBON_LOOK_AT:
			toCreate = new NL3D::CPSRibbonLookAt;
		break;

		//////////////
		// emitters //
		//////////////

		case IDM_DIRECTIONNAL_EMITTER_LOC:  createLocAndBindable = true;
		case IDM_DIRECTIONNAL_EMITTER:
			toCreate = new NL3D::CPSEmitterDirectionnal;
		break;
		case IDM_OMNIDIRECTIONNAL_EMITTER_LOC:  createLocAndBindable = true;
		case IDM_OMNIDIRECTIONNAL_EMITTER:
			toCreate = new NL3D::CPSEmitterOmni;
		break;
		case IDM_CONIC_EMITTER_LOC:  createLocAndBindable = true;
		case IDM_CONIC_EMITTER:
			toCreate = new NL3D::CPSEmitterConic;
		break; 
		case IDM_RECTANGLE_EMITTER_LOC:  createLocAndBindable = true;
		case IDM_RECTANGLE_EMITTER:
			toCreate = new NL3D::CPSEmitterRectangle;
		break;
		case IDM_SPHERICAL_EMITTER_LOC:  createLocAndBindable = true;
		case IDM_SPHERICAL_EMITTER:
			toCreate = new NL3D::CPSSphericalEmitter;
		break;
		case IDM_RADIAL_EMITTER_LOC:  createLocAndBindable = true;
		case IDM_RADIAL_EMITTER:
			toCreate = new NL3D::CPSRadialEmitter;
		break;

		////////////////
		//   Zones    //
		////////////////
		
		case IDM_ZONE_PLANE_LOC:  createLocAndBindable = true;
		case IDM_ZONE_PLANE:
			toCreate = new NL3D::CPSZonePlane;
		break;
		case IDM_ZONE_SPHERE_LOC:  createLocAndBindable = true;
		case IDM_ZONE_SPHERE:
			toCreate = new NL3D::CPSZoneSphere;
		break;
		case IDM_ZONE_DISC_LOC:  createLocAndBindable = true;
		case IDM_ZONE_DISC:
			toCreate = new NL3D::CPSZoneDisc;
		break;
		case IDM_ZONE_RECTANGLE_LOC:  createLocAndBindable = true;
		case IDM_ZONE_RECTANGLE:
			toCreate = new NL3D::CPSZoneRectangle;
		break;
		case IDM_ZONE_CYLINDER_LOC:  createLocAndBindable = true;
		case IDM_ZONE_CYLINDER:
			toCreate = new NL3D::CPSZoneCylinder;
		break;

		///////////////
		//   forces  //
		///////////////
		case IDM_GRAVITY_FORCE_LOC:  createLocAndBindable = true;
		case IDM_GRAVITY_FORCE:
			toCreate = new NL3D::CPSGravity;
		break;
		case IDM_DIRECTIONNAL_FORCE_LOC:  createLocAndBindable = true;
		case IDM_DIRECTIONNAL_FORCE:
			toCreate = new NL3D::CPSDirectionnalForce;
		break;
		case IDM_SPRING_FORCE_LOC:  createLocAndBindable = true;
		case IDM_SPRING_FORCE:
			toCreate = new NL3D::CPSSpring;
		break;
		case IDM_FLUID_FRICTION_LOC:  createLocAndBindable = true;
		case IDM_FLUID_FRICTION:
			toCreate = new NL3D::CPSFluidFriction;
		break;
		case IDM_CENTRAL_GRAVITY_LOC:  createLocAndBindable = true;
		case IDM_CENTRAL_GRAVITY:
			toCreate = new NL3D::CPSCentralGravity;
		break;
		case IDM_CYLINDRIC_VORTEX_LOC:  createLocAndBindable = true;
		case IDM_CYLINDRIC_VORTEX:
			toCreate = new NL3D::CPSCylindricVortex;
		break;
		case IDM_BROWNIAN_MOVE_LOC:  createLocAndBindable = true;
		case IDM_BROWNIAN_MOVE:
			toCreate = new NL3D::CPSBrownianForce;
		break;
		case IDM_MAGNETIC_FORCE_LOC:  createLocAndBindable = true;
		case IDM_MAGNETIC_FORCE:
			toCreate = new NL3D::CPSMagneticForce;
		break;

		///////////////
		//    sound  //
		///////////////
		case IDM_SOUND_LOC:  createLocAndBindable = true;
		case IDM_SOUND:
			toCreate = new NL3D::CPSSound;
			if (!_ParticleDlg->StartStopDlg->isRunning())
			{
				(static_cast<NL3D::CPSSound *>(toCreate))->stopSound();
			}
		break;

		///////////////
		//    light  //
		///////////////
		case IDM_LIGHT_LOC:  createLocAndBindable = true;
		case IDM_LIGHT:
			toCreate = new NL3D::CPSLight;			
		break;

		//////////////
		// deletion //
		//////////////
		case IDM_DELETE_LOCATED:
		{
			deleteSelection();
			return TRUE;			
			//return CTreeCtrl::OnCmdMsg(nID,  nCode,  pExtra,  pHandlerInfo);
		}
		break;


		case IDM_DELETE_LOCATED_BINDABLE:
		{								
			deleteSelection();			
			return TRUE;			
		}
		break;

		case IDM_DELETE_LOCATED_INSTANCE:
		{
			deleteSelection();
			return TRUE;
		}
		break; 

		// instanciate an element
		case ID_INSTANCIATE_LOCATED:
		{						
			getOwnerNode(nt)->setModified(true);
			_ParticleDlg->StartStopDlg->resetAutoCount(getOwnerNode(nt));
			if (nt->Loc->getSize() == nt->Loc->getMaxSize())
			{
				nt->Loc->resize(nt->Loc->getMaxSize() + 1);
				// force a re-update of the left pane
				NM_TREEVIEW nmt;
				LRESULT pResult;
				nmt.itemNew.lParam = GetItemData(GetSelectedItem());
				OnSelchanged((NMHDR *) &nmt,  &pResult);
			}
			sint32 objIndex = nt->Loc->newElement(NLMISC::CVector::Null,  NLMISC::CVector::Null,  NULL,  0,  nt->Loc->getMatrixMode(),  0.f);
			nt = new CNodeType(nt->Loc,  objIndex);
			_NodeTypes.push_back(nt);
			// insert the element in the tree
			HTREEITEM root = InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT, _T("instance"),  PSIconLocatedInstance,  PSIconLocatedInstance,  0,  0,  (LPARAM) nt,  GetSelectedItem(),  TVI_LAST);
			SetItemData(root, (DWORD_PTR) nt);
			Invalidate();
		}
		break;

		////////////
		// LOD OP //
		////////////
		case IDM_LB_LOD1N2:
			nlassert(nt->Type = CNodeType::locatedBindable);
			nt->Bind->setLOD(NL3D::PSLod1n2);
			getOwnerNode(nt)->setModified(true);
		break;
		case IDM_LB_LOD1:
			nlassert(nt->Type = CNodeType::locatedBindable);
			nt->Bind->setLOD(NL3D::PSLod1);
			getOwnerNode(nt)->setModified(true);
		break;
		case IDM_LB_LOD2:
			nlassert(nt->Type = CNodeType::locatedBindable);
			nt->Bind->setLOD(NL3D::PSLod2);
			getOwnerNode(nt)->setModified(true);
		break;

		////////////////
		// extern ID  //
		////////////////
		case IDM_LB_EXTERN_ID:
		{
			nlassert(nt->Type = CNodeType::locatedBindable);
			nlassert(nt->Bind);
			CLBExternIDDlg 	dlg(nt->Bind->getExternID());
			INT_PTR res = dlg.DoModal();
			if ( res == IDOK )
			{
				nt->Bind->setExternID( dlg.getNewID() );
				getOwnerNode(nt)->setModified(true);
			}
		}
		break;
		////////////////////////
		//		COPY / PASTE  //
		////////////////////////
		case IDM_COPY_LOCATED:
			nlassert(nt->Type == CNodeType::located);
			nlassert(nt->Loc);			
			_LocatedCopy.reset(NLMISC::safe_cast<NL3D::CPSLocated *>(::DupPSLocated(nt->Loc)));
		break;
		case IDM_COPY_BINDABLE:
			nlassert(nt->Type == CNodeType::locatedBindable);
			nlassert(nt->Bind);			
			_LocatedBindableCopy.reset(::DupPSLocatedBindable(nt->Bind));
		break;
		case IDM_PASTE_LOCATED:
		{			
			nlassert(nt->Type== CNodeType::particleSystem);
			nlassert(nt->PS);
			getOwnerNode(nt)->setModified(true);
			_ParticleDlg->StartStopDlg->resetAutoCount(getOwnerNode(nt));
			CPSLocated *copy = dynamic_cast<CPSLocated *>(::DupPSLocated(_LocatedCopy.get()));
			if (!copy) break;
			if (nt->PS->getPSPointer()->attach(copy))
			{			
				createNodeFromLocated(copy, GetSelectedItem());
				Invalidate();
			}
			else
			{
				CString mess;
				mess.LoadString(IDS_PS_NO_FINITE_DURATION);
				CString errorStr;
				errorStr.LoadString(IDS_ERROR);
				MessageBox((LPCTSTR) mess,  (LPCTSTR) errorStr,  MB_ICONEXCLAMATION);
			}
		}
		break;
		case IDM_PASTE_BINDABLE:
		{
			nlassert(nt->Type == CNodeType::located);
			nlassert(nt->Loc);
			getOwnerNode(nt)->setModified(true);
			_ParticleDlg->StartStopDlg->resetAutoCount(getOwnerNode(nt));
			CPSLocatedBindable *copy = ::DupPSLocatedBindable(_LocatedBindableCopy.get());
			if (!copy) break;
			if (nt->Loc->bind(copy))
			{			
				createNodeFromLocatedBindable(copy,  GetSelectedItem());
				Invalidate();
			}
			else
			{
				delete copy;
				CString mess;
				mess.LoadString(IDS_PS_NO_FINITE_DURATION);
				CString errorStr;
				errorStr.LoadString(IDS_ERROR);
				MessageBox((LPCTSTR) mess,  (LPCTSTR) errorStr,  MB_ICONEXCLAMATION);
			}
		}
		break;


		////////////////////////
		// PARTICLE SYSTEM OP //
		////////////////////////
		case IDM_SET_ACTIVE_PARTICLE_SYSTEM:
			nlassert(nt->Type == CNodeType::particleSystem);
			setActiveNode(nt->PS);
			_ParticleDlg->setActiveNode(nt->PS);
		break;
		case ID_MENU_NEWLOCATED:
		{			
			getOwnerNode(nt)->setModified(true);
			createLocated(nt->PS->getPSPointer(),  GetSelectedItem());	
			Invalidate();
		}
		break;
		case IDM_RELOAD_PS:
		{			
			nlassert(!nt->PS->isLoaded());
			_ParticleDlg->loadPS(*this,  *nt->PS,  CParticleDlg::ReportError);
			if (nt->PS->isLoaded())
			{						
				// remove old icon
				HTREEITEM root = GetParentItem(GetSelectedItem());
				HTREEITEM previousSibling = GetPrevSiblingItem(GetSelectedItem());
				DeleteItem(GetSelectedItem());			
				// add newly loaded ps in the tree
				buildTreeFromPS(*nt->PS,  root,  previousSibling);
				updateRightPane(*nt);
			}
		}			
		break;
		case IDM_REMOVE_PS_FROM_WORKSPACE:
		{
			deleteSelection();			
			return TRUE;
		}
		break;		
		case IDM_MERGE_PS:
		{			
			_ParticleDlg->StartStopDlg->stop();
			static TCHAR BASED_CODE szFilter[] = _T("ps & shapes files(*.ps;*.shape)|*.ps; *.shape||");
			CFileDialog fd( TRUE,  _T(".ps"),  _T("*.ps;*.shape"),  0,  szFilter);
			
			if (fd.DoModal() == IDOK)
			{
				CParticleWorkspace::CNode *ownerNode = getOwnerNode(nt);
				nlassert(ownerNode);				

				// Add search path for the texture
			    NLMISC::CPath::addSearchPath(NLMISC::CFile::getPath(NLMISC::tStrToUtf8(fd.GetPathName())));

				CUniquePtr<NL3D::CShapeBank> sb(new NL3D::CShapeBank);
				CParticleSystemModel *psm  = NULL;
				try
				{					
					NL3D::CShapeStream ss;
					NLMISC::CIFile inputFile;
				    inputFile.open(NLMISC::tStrToUtf8(fd.GetPathName()));
					ss.serial(inputFile);
				    std::string shapeName = NLMISC::CFile::getFilename(NLMISC::tStrToUtf8(fd.GetPathName()));
					sb->add(shapeName, ss.getShapePointer());
					NL3D::CShapeBank *oldSB = CNELU::Scene->getShapeBank();
					CNELU::Scene->setShapeBank(sb.get());
					NL3D::CTransformShape *trs = CNELU::Scene->createInstance(shapeName);
					NL3D::CNELU::Scene->setShapeBank(oldSB);
					if (!trs)
					{
						localizedMessageBox(*this, IDS_COULDNT_INSTANCIATE_PS,  IDS_ERROR, MB_OK|MB_ICONEXCLAMATION);
						return false;
					}
					psm = dynamic_cast<CParticleSystemModel *>(trs);
					if (!psm)
					{
						localizedMessageBox(*this, IDS_COULDNT_INSTANCIATE_PS,  IDS_ERROR, MB_OK|MB_ICONEXCLAMATION);
						// Not a particle system
						NL3D::CShapeBank *oldSB = CNELU::Scene->getShapeBank();
						NL3D::CNELU::Scene->setShapeBank(sb.get());
						NL3D::CNELU::Scene->deleteInstance(trs);
						NL3D::CNELU::Scene->setShapeBank(oldSB);
						return false;
					}
				}
				catch(const NLMISC::EStream &e)
				{
				    MessageBox(nlUtf8ToTStr(e.what()), getStrRsc(IDS_ERROR), MB_OK | MB_ICONEXCLAMATION);
					return TRUE;
				}							
				ownerNode->setResetAutoCountFlag(false);								
				bool wasActiveNode = _LastActiveNode == GetSelectedItem();
				if (wasActiveNode)
				{
					_ParticleDlg->StartStopDlg->stop();
				}
				// link to the root for manipulation
				_ParticleDlg->_ObjView->getSceneRoot()->hrcLinkSon(psm);
				bool mergeOK = nt->PS->getPSPointer()->merge( NLMISC::safe_cast<NL3D::CParticleSystemShape *>((NL3D::IShape *) psm->Shape));
				NL3D::CShapeBank *oldSB = CNELU::Scene->getShapeBank();
				CNELU::Scene->setShapeBank(sb.get());
				CNELU::Scene->deleteInstance(psm);
				CNELU::Scene->setShapeBank(oldSB);
				if (!mergeOK)
				{					
					localizedMessageBox(*this, IDS_PS_NO_FINITE_DURATION,  IDS_ERROR,  MB_ICONEXCLAMATION);
					return TRUE;
				}				
				if (wasActiveNode)
				{					
					setActiveNode(NULL);					
					_ParticleDlg->setActiveNode(NULL);
				}
				HTREEITEM prevSibling = GetPrevSiblingItem(GetSelectedItem());
				HTREEITEM prevParent = GetParentItem(GetSelectedItem());
				removeTreePart(GetSelectedItem());				
				ownerNode->setModified(true);
				HTREEITEM newRoot = buildTreeFromPS(*ownerNode,  prevParent,  prevSibling);
				Select(newRoot, TVGN_CARET);
				if (wasActiveNode)
				{
					setActiveNode(ownerNode);
					_ParticleDlg->setActiveNode(ownerNode);
				}
				updateRightPane(*nt);				
				_LastClickedPS = NULL;				
			}								
		}
		break;
		case ID_MENU_SAVE_PS:
			nlassert(getOwnerNode(nt));
			_ParticleDlg->savePS(*this,  *getOwnerNode(nt),  false);	
		break;
		case IDM_SAVE_PS_AS:
		{
			if (nt->PS->getResetAutoCountFlag() && nt->PS->getPSPointer()->getAutoCountFlag())
			{		
				MessageBox(nt->PS->getFilename().c_str() + getStrRsc(IDS_AUTO_COUNT_ERROR), getStrRsc(IDS_WARNING), MB_ICONEXCLAMATION);				
			}			
			else
			{			
				_ParticleDlg->StartStopDlg->stop();
				std::string fileName = nt->PS->getFilename();
				static TCHAR BASED_CODE szFilter[] = _T("ps & shapes files(*.ps;*.shape)|*.ps; *.shape||");
			    CFileDialog fd(FALSE, _T(".ps"), nlUtf8ToTStr(fileName), OFN_OVERWRITEPROMPT, szFilter, this);
				if (fd.DoModal() == IDOK)
				{
				    _ParticleDlg->savePSAs(*this, *nt->PS, NLMISC::tStrToUtf8(fd.GetPathName()), false);
				}
			}
		}
		break;
		case IDM_CLEAR_PS_CONTENT:
		{
			if (localizedMessageBox(*this, IDS_CLEAR_CONTENT,  IDS_PARTICLE_EDITOR,  MB_YESNO) == IDYES)
			{				
				CParticleWorkspace::CNode *ownerNode = getOwnerNode(nt);
				nlassert(ownerNode);
				ownerNode->setResetAutoCountFlag(false);								
				bool wasActiveNode = _LastActiveNode == GetSelectedItem();
				if (wasActiveNode)
				{					
					setActiveNode(NULL);					
					_ParticleDlg->setActiveNode(NULL);
				}
				HTREEITEM prevSibling = GetPrevSiblingItem(GetSelectedItem());
				HTREEITEM prevParent = GetParentItem(GetSelectedItem());
				removeTreePart(GetSelectedItem());
				ownerNode->createEmptyPS();
				ownerNode->setModified(true);
				buildTreeFromPS(*ownerNode,  prevParent,  prevSibling);
				if (wasActiveNode)
				{
					setActiveNode(ownerNode);
					_ParticleDlg->setActiveNode(ownerNode);
				}
				updateRightPane(*nt);
				_LastClickedPS = NULL;
			}
		}
		break;
		//////////
		// MISC //
		//////////
		case  IDM_FORCE_ZBIAS:
			// querry zbias
			CSetValueDlg valueDlg;			
			// Set default value
			valueDlg.Value="0.00";
			valueDlg.Title.LoadString(IDS_FORCE_ZBIAS);			
			// Open dialog
			if (valueDlg.DoModal ()==IDOK)
			{
				float value;		
				int dummy; // to avoid non numeric characters at the end
				if (_stscanf((LPCTSTR)(valueDlg.Value + _T("\n0")), _T("%f\n%d"), &value, &dummy) == 2)
				{
					nlassert(getOwnerNode(nt)->getPSPointer());
					getOwnerNode(nt)->getPSPointer()->setZBias(-value);
					getOwnerNode(nt)->setModified(true);
				}
				else
				{
					CString caption;
					CString mess;
					caption.LoadString(IDS_CAPTION_ERROR);
					mess.LoadString(IDS_BAD_ZBIAS);								
					MessageBox((LPCTSTR) mess,  (LPCTSTR) caption,  MB_ICONERROR);
				}
			}			
		break;
	}


	if (toCreate)
	{
		HTREEITEM son,  lastSon,  father;
		if (createLocAndBindable)
		{
			
			std::pair<CParticleTreeCtrl::CNodeType *,  HTREEITEM> p = createLocated(nt->PS->getPSPointer(),  GetSelectedItem());
			nt = p.first;
			son = 0;
			father = p.second;
			lastSon = p.second;
			nlassert(getOwnerNode(nt) && getOwnerNode(nt)->getPSPointer());				
			if (getOwnerNode(nt)->getPSPointer()->getBypassMaxNumIntegrationSteps())
			{		
				if (toCreate->getType() == NL3D::PSParticle || toCreate->getType() == NL3D::PSEmitter)
				{				
					nt->Loc->setInitialLife(1.f);
				}
				// object must have finite duration with that flag				
			}
		}
		else
		{
			son = GetChildItem(GetSelectedItem());
			father = GetSelectedItem();
		}				
		if (!nt->Loc->bind(toCreate))
		{
			MessageBox(_T("The system is flagged with 'No max Nb steps',  or uses the preset 'Spell FX'. System must have finite duration. Can't add object. To solve this,  set a limited life time for the father."), _T("Error"),  MB_ICONEXCLAMATION);
			delete toCreate;
			if (createLocAndBindable)
			{			
				nlassert(0);
				/* ps->remove(nt->Loc);	
				DeleteItem(father);				
				_NodeTypes.erase(std::find(_NodeTypes.begin(),  _NodeTypes.end(),  nt));
				delete nt;*/
			}
			return CTreeCtrl::OnCmdMsg(nID,  nCode,  pExtra,  pHandlerInfo);
		}			
		// complete the name
		std::string name = toCreate->getName();
		char num[128];
		if (_PSElementIdentifiers.count(name))
		{			 
			sprintf(num,  "%d",  ++_PSElementIdentifiers[name]);
			toCreate->setName(name + num);
		}
		else
		{ 
			_PSElementIdentifiers[toCreate->getName()] = 0;
			toCreate->setName(name + "0");
		}						
		CNodeType *newNt = new CNodeType(toCreate);
		_NodeTypes.push_back(newNt);

		// insert the element in the tree
		// we want that the instance always appears in the last position				
		if (!createLocAndBindable)
		{
			if (!son)
			{
				lastSon = GetSelectedItem();
			}
			else
			{

				lastSon  = TVI_FIRST;

				
				while (son != NULL)
				{				
					if (((CNodeType *) GetItemData(son))->Type == CNodeType::locatedInstance)
					{
						break;
					}
					lastSon = son;
					son = GetChildItem(son);
				}
			}
		}
		getOwnerNode(nt)->setModified(true);
		// TODO : an enum for CPSLocatedBindable::getType would be better...
		InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT, nlUtf8ToTStr(toCreate->getName()), toCreate->getType(), toCreate->getType(), 0, 0, (LPARAM)newNt, father, lastSon);
		touchPSState(nt);		
		Invalidate();		
		_ParticleDlg->StartStopDlg->resetAutoCount(getOwnerNode(nt));
	}
	return CTreeCtrl::OnCmdMsg(nID,  nCode,  pExtra,  pHandlerInfo);
}

//****************************************************************************************************************
std::pair<CParticleTreeCtrl::CNodeType *,  HTREEITEM> CParticleTreeCtrl::createLocated(NL3D::CParticleSystem *ps,  HTREEITEM headItem)
{ 
	std::string name; 
	char num[128];
	if (_PSElementIdentifiers.count(std::string("located")))
	{
		sprintf(num,  "located %d",  ++_PSElementIdentifiers[std::string("located")]);
		name = num;
	}
	else
	{
		name = std::string("located 0");
		_PSElementIdentifiers["located"] = 0;
	}			
	CPSLocated *loc = new CPSLocated;
	loc->setName(name);
	loc->setMatrixMode(NL3D::PSFXWorldMatrix);
	ps->attach(loc);

	CNodeType *newNt = new CNodeType(loc);
	_NodeTypes.push_back(newNt);
	// insert item in tree
	HTREEITEM insertedItem = InsertItem(TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_TEXT, nlUtf8ToTStr(name), PSIconLocated, PSIconLocated, 0, 0, (LPARAM)newNt, headItem, TVI_LAST);
	touchPSState(newNt);	
	return std::make_pair(newNt,  insertedItem);
}


//****************************************************************************************************************
/// The user finished to edit a label in the tree
void CParticleTreeCtrl::OnEndlabeledit(NMHDR* pNMHDR,  LRESULT* pResult) 
{
	NMTVDISPINFO* info = (NMTVDISPINFO*)pNMHDR;
	*pResult = 0;
	if (info->item.pszText)
	{
		CNodeType *nt = (CNodeType *) info->item.lParam;
		switch (nt->Type)
		{
			case CNodeType::workspace:
			{
			    nt->WS->setName(NLMISC::tStrToUtf8(info->item.pszText));
				workspaceModifiedFlagChanged(*nt->WS); // change name (this may be called twice because of the modification callback, but this doesn't matter)
			}
			break;
			case CNodeType::particleSystem:
			{
				if (!nt->PS->isLoaded())
				{
					localizedMessageBox(*this, IDS_CANT_CHANGE_PS_NAME, IDS_ERROR, MB_OK|MB_ICONEXCLAMATION);
				}
				else
				{				
					nt->PS->getPSPointer()->setName(NLMISC::tStrToUtf8(info->item.pszText));
					nt->PS->setModified(true);
				}
			    this->SetItemText(info->item.hItem, nlUtf8ToTStr(computeCaption(*nt->PS)));
			}
			break;
			case CNodeType::located:
			{
				nlassert(getOwnerNode(nt));
				getOwnerNode(nt)->setModified(true);
				this->SetItemText(info->item.hItem,  info->item.pszText);
			    nt->Loc->setName(NLMISC::tStrToUtf8(info->item.pszText));
			}
			break;
			case CNodeType::locatedBindable:
			{
				nlassert(getOwnerNode(nt));
				getOwnerNode(nt)->setModified(true);
				this->SetItemText(info->item.hItem,  info->item.pszText);
			    nt->Bind->setName(NLMISC::tStrToUtf8(info->item.pszText));
			}
			break;
		}

	}
}


//****************************************************************************************************************
void CParticleTreeCtrl::moveElement(const NLMISC::CMatrix &m)
{
	NLMISC::CMatrix mat;

	// no == operator yet... did the matrix change ?
	if (m.getPos() == mat.getPos()
		&& m.getI() == mat.getI() 
		&& m.getJ() == mat.getJ() 
		&& m.getK() == mat.getK() 
	   ) return;

	nlassert(_ParticleDlg);	
	
	// the current element must be an instance
	if (::IsWindow(m_hWnd))
	{
		HTREEITEM currItem = GetSelectedItem();
		if (currItem)
		{			
			CNodeType *nt = (CNodeType *) GetItemData(currItem);
			if (nt->Type == CNodeType::locatedInstance)
			{
				if (nt->Loc->getMatrixMode() == NL3D::PSFXWorldMatrix)
				{
					mat = _ParticleDlg->getPSWorldMatrix().inverted() * m;	
				}
				else if (nt->Loc->getMatrixMode() == NL3D::PSIdentityMatrix)
				{
					mat = m;
				}
				else
				{
					return;
				}

				CWnd *rightPane = _ParticleDlg->getRightPane();
				NL3D::IPSMover *psm = NULL;
				if (dynamic_cast<CPSMoverDlg *>(rightPane ))
				{
					CPSMoverDlg *rp = (CPSMoverDlg *) rightPane;
					psm = ((CPSMoverDlg *) rightPane)->getMoverInterface();
													
					if (psm && !psm->onlyStoreNormal())
					{									
						psm->setMatrix(rp->getLocatedIndex(),  mat);										
					}
					else
					{
						rp->getLocated()->getPos()[rp->getLocatedIndex()] = mat.getPos();
					}

					// update the dialog				
					rp->updatePosition();
				}
			}
		}
	}
}


//****************************************************************************************************************
NLMISC::CMatrix CParticleTreeCtrl::getElementMatrix(void) const
{
	HTREEITEM currItem = GetSelectedItem();
	if (currItem)
	{
		CNodeType *nt = (CNodeType *) GetItemData(currItem);
		if (nt->Type == CNodeType::locatedInstance)
		{
			NLMISC::CVector pos = nt->Loc->getPos()[nt->LocatedInstanceIndex];
			NLMISC::CMatrix m;
			m.identity();
			m.setPos(pos);
			if (nt->Loc->getMatrixMode() == NL3D::PSFXWorldMatrix)
			{
				m = _ParticleDlg->getPSWorldMatrix() * m;
			}
			return m; 
		}
	}

	return NLMISC::CMatrix::Identity;
}


//****************************************************************************************************************
CParticleSystem *CParticleTreeCtrl::CNodeType::getOwnerPS() const
{
	switch(Type)
	{
		case located:			return Loc->getOwner();
		case particleSystem:	return PS->getPSPointer();
		case locatedBindable:	return Bind->getOwner()->getOwner();
		case locatedInstance:	return Loc->getOwner();
		case workspace:			return NULL;
		default:
			nlassert(0);
		break;
	}
	return NULL;
}

//****************************************************************************************************************
CParticleWorkspace::CNode *CParticleTreeCtrl::getOwnerNode(CNodeType *nt) const
{
	if (!nt) return NULL;
	HTREEITEM node = getTreeItem(nt);	
	while(node)
	{
		CNodeType *nt = (CNodeType *) GetItemData(node);
		if (!nt) return NULL;
		if (nt->Type == CNodeType::particleSystem)
		{
			return nt->PS;
		}
		node = GetParentItem(node);
	}
	return NULL;
}

//****************************************************************************************************************
void CParticleTreeCtrl::updateCaption(CParticleWorkspace::CNode &node)
{
	HTREEITEM item = getTreeItem(&node);
	if (!item) return;
	// update name of ps to dipslay a star in front of it (this tells that the ps has been modified)
	SetItemText(item, nlUtf8ToTStr(computeCaption(node)));
}

//****************************************************************************************************************
std::string CParticleTreeCtrl::computeCaption(const std::string &path, const std::string &userName, bool modified)
{	
	std::string name;
	if (modified)
	{
		name = "* ";
	}
	if (!userName.empty())
	{ 
		name += userName;
		if (_ViewFilenameFlag)
		{
			name += " - ";
		}
	}
	if (_ViewFilenameFlag)
	{	
		name += NLMISC::CFile::getFilename(path);
	}
	return name;
}

//****************************************************************************************************************
std::string CParticleTreeCtrl::computeCaption(CParticleWorkspace::CNode &node)
{
	std::string baseCaption;
	if (node.isLoaded())
	{	
		baseCaption = computeCaption(node.getRelativePath(), node.getPSPointer()->getName(), node.isModified());
	}
	else
	{
		baseCaption = computeCaption(node.getRelativePath(), "", false);
	}	
	if (!node.getTriggerAnim().empty())
	{
		baseCaption = "(" + node.getTriggerAnim() + ") " + baseCaption;
	}
	if (node.getParentSkel())
	{	
		baseCaption = "(L) " + baseCaption;
	}
	return baseCaption;
}

//****************************************************************************************************************
std::string CParticleTreeCtrl::computeCaption(CParticleWorkspace &workspace)
{
	return computeCaption(workspace.getFilename(), workspace.getName(), workspace.isModified());	
}

//****************************************************************************************************************
void CParticleTreeCtrl::insertNewPS(CParticleWorkspace &pws)
{
	static const TCHAR BASED_CODE szFilter[] = _T("NeL Particle systems (*.ps)|*.ps||");
	CFileDialog fd(TRUE,  _T(".ps"), _T("*.ps"),  OFN_ALLOWMULTISELECT|OFN_FILEMUSTEXIST,  szFilter,  this);	

	const uint MAX_NUM_CHAR = 65536;
	TCHAR filenamesBuf[MAX_NUM_CHAR];
	_tcscpy_s(filenamesBuf, MAX_NUM_CHAR, _T("*.ps"));

	fd.m_ofn.lpstrFile = filenamesBuf;
	fd.m_ofn.nMaxFile = MAX_NUM_CHAR - 1;
	if (fd.DoModal() == IDOK)
	{
		CParticleWorkspace::IModificationCallback *oldCallback = pws.getModificationCallback();
		pws.setModificationCallback(NULL);
		POSITION pos = fd.GetStartPosition();
		bool diplayLoadingError = true;
		bool diplayNodeAlreadyInserted = true;
		bool hasSelectedNode = _ParticleDlg->getActiveNode() != NULL;
		CParticleWorkspace::CNode *firstLoadedNode = NULL;
		while (pos)
		{
			CString path = fd.GetNextPathName(pos);						
			CParticleWorkspace::CNode *node = pws.addNode(NLMISC::tStrToUtf8(path));
			if (!node)
			{
				if (diplayNodeAlreadyInserted)
				{
					if (pos)
					{					
						CSkippableMessageBox smb(path + getStrRsc(IDS_PS_ALREADY_INSERTED), getStrRsc(IDS_ERROR), this);
						smb.DoModal();
						diplayNodeAlreadyInserted = !smb.getBypassFlag();
					}
					else
					{
						MessageBox(CString(nlUtf8ToTStr(NLMISC::CFile::getFilename(NLMISC::tStrToUtf8(path)))) + getStrRsc(IDS_PS_ALREADY_INSERTED), getStrRsc(IDS_ERROR), MB_OK | MB_ICONEXCLAMATION);
					}
				}								
				continue;				
			}
			if (diplayLoadingError)
			{
				diplayLoadingError = _ParticleDlg->loadPS(*this,  *node, pos ? CParticleDlg::ReportErrorSkippable : CParticleDlg::ReportError);
			}
			else
			{
				_ParticleDlg->loadPS(*this,  *node, CParticleDlg::Silent);
			}
			if (!node->isLoaded()) 
			{
				pws.removeNode(pws.getNumNode() - 1);				
			}
			else
			{		
				if (!firstLoadedNode) firstLoadedNode = node;			
				buildTreeFromPS(*node,  GetRootItem());
			}
		}
		pws.setModificationCallback(oldCallback);
		if (firstLoadedNode)
		{
			expandRoot();
			pws.touch();
			if (!hasSelectedNode)
			{
				_ParticleDlg->setActiveNode(firstLoadedNode);
				setActiveNode(firstLoadedNode);
			}
		}
		// update modified state
		SetItemText(GetRootItem(), nlUtf8ToTStr(computeCaption(pws)));
	}
	
}

//****************************************************************************************************************
void CParticleTreeCtrl::createNewPS(CParticleWorkspace &pws)
{
	CCreateFileDlg cfd(getStrRsc(IDS_CREATE_NEW_PS), NLMISC::CPath::standardizeDosPath(pws.getPath()), "ps", this);
	if (cfd.DoModal() == IDOK)
	{
		if (pws.containsFile(cfd.getFileName()))
		{
			MessageBox((LPCTSTR) (CString(NLMISC::CFile::getFilename(cfd.getFileName()).c_str()) + getStrRsc(IDS_PS_ALREADY_INSERTED)),  getStrRsc(IDS_ERROR),  MB_ICONEXCLAMATION);
		}
		if (cfd.touchFile())
		{
			CParticleWorkspace::CNode *node = pws.addNode(cfd.getFullPath());
			nlassert(node); // should always succeed because we tested if file already exists
			node->createEmptyPS();
			_ParticleDlg->savePS(*this, *node,   false); // write empty ps to disk
			// create an icon at the end of workspace
			buildTreeFromPS(*node,  GetRootItem());
		}
	}
}

//****************************************************************************************************************
void CParticleTreeCtrl::removeTreePart(HTREEITEM root)
{
	CNodeType *nt = (CNodeType *) GetItemData(root);
	_NodeTypes.erase(std::find(_NodeTypes.begin(),  _NodeTypes.end(),  nt));
	delete nt;
	HTREEITEM child = GetChildItem(root);
	while (child)
	{
		HTREEITEM tmpChild = child;
		child = GetNextSiblingItem(child);
		removeTreePart(tmpChild);
	}	
	DeleteItem(root);
}

//****************************************************************************************************************
HTREEITEM CParticleTreeCtrl::getTreeItem(CNodeType *nt) const
{
	if (!GetRootItem()) return NULL;
	std::stack<HTREEITEM> leftToTraverse;
	leftToTraverse.push(GetRootItem());
	while (!leftToTraverse.empty())
	{
		HTREEITEM curr = leftToTraverse.top();
		leftToTraverse.pop();
		if ((CNodeType *) GetItemData(curr) == nt)
		{
			return curr;
		}
		if (GetChildItem(curr)) leftToTraverse.push(GetChildItem(curr));
		if (GetNextSiblingItem(curr)) leftToTraverse.push(GetNextSiblingItem(curr));
	}
	return NULL;
}

//****************************************************************************************************************
HTREEITEM CParticleTreeCtrl::getTreeItem(CParticleWorkspace::CNode *node) const
{
	if (!GetRootItem()) return NULL;
	std::stack<HTREEITEM> leftToTraverse;
	leftToTraverse.push(GetRootItem());
	while (!leftToTraverse.empty())
	{
		HTREEITEM curr = leftToTraverse.top();
		leftToTraverse.pop();
		const CNodeType *nt = (CNodeType *) GetItemData(curr);
		if (nt && nt->Type == CNodeType::particleSystem && nt->PS == node)
		{
			return curr;
		}
		if (GetChildItem(curr)) leftToTraverse.push(GetChildItem(curr));
		if (GetNextSiblingItem(curr)) leftToTraverse.push(GetNextSiblingItem(curr));
	}
	return NULL;
}

//****************************************************************************************************************
void CParticleTreeCtrl::nodeModifiedFlagChanged(CParticleWorkspace::CNode &node)
{
	updateCaption(node);	
}

//****************************************************************************************************************
void CParticleTreeCtrl::workspaceModifiedFlagChanged(CParticleWorkspace &ws)
{
	// for now assume that workspace is the root
	HTREEITEM root = GetRootItem();
	if (!root) return;
	SetItemText(root, (LPCTSTR) computeCaption(ws).c_str());
}

//****************************************************************************************************************
void CParticleTreeCtrl::nodeSkelParentChanged(CParticleWorkspace::CNode &node)
{
	updateCaption(node);	
}

//****************************************************************************************************************
void CParticleTreeCtrl::setActiveNode(CParticleWorkspace::CNode *node)
{
	HTREEITEM newItem = getTreeItem(node);
	if (_LastActiveNode) SetItemState(_LastActiveNode,  0,  TVIS_BOLD);
	if (newItem)
	{	
		SetItemState(newItem,  TVIS_BOLD,  TVIS_BOLD);
	}
	_LastActiveNode = newItem;
}

//****************************************************************************************************************
void CParticleTreeCtrl::touchPSState(CNodeType *nt)
{
	if (!nt) return;
	CParticleWorkspace::CNode *ownerNode = getOwnerNode(nt);
	if (ownerNode && ownerNode->getPSModel())
	{
		ownerNode->getPSModel()->touchLightableState();
		ownerNode->getPSModel()->touchTransparencyState();
	}
}

//****************************************************************************************************************
void CParticleTreeCtrl::OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{	
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;	
	NMTVDISPINFO* info = (NMTVDISPINFO*)pNMHDR;
	*pResult = 0;
	CEdit* pEdit = GetEditControl();
	if (!pEdit) return;
	if (info->item.pszText)
	{
		CNodeType *nt = (CNodeType *) info->item.lParam;
		switch (nt->Type)
		{
			case CNodeType::workspace:
			pEdit->SetWindowText(nlUtf8ToTStr(nt->WS->getName()));
			break;
			case CNodeType::particleSystem:
			{
				if (!nt->PS->isLoaded())
				{
					pEdit->SetWindowText(_T(""));
					//localizedMessageBox(*this, IDS_CANT_CHANGE_PS_NAME, IDS_ERROR, MB_OK|MB_ICONEXCLAMATION);
				}
				else
				{				
					pEdit->SetWindowText(nlUtf8ToTStr(nt->PS->getPSPointer()->getName()));
				}				
			}
			break;
			case CNodeType::located:
			{				
				pEdit->SetWindowText(nlUtf8ToTStr(nt->Loc->getName()));
			}
			break;
			case CNodeType::locatedBindable:
			{
			    pEdit->SetWindowText(nlUtf8ToTStr(nt->Bind->getName()));
			}
			break;
		}
	}	
	*pResult = 0;
}

//****************************************************************************************************************
void CParticleTreeCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{	
	UINT flags;	
	HTREEITEM item  = this->HitTest(point,  &flags);
	if (item)
	{
		CNodeType *nt =  (CNodeType *) GetItemData(item);
		nlassert(nt);
		if (nt->Type == CNodeType::particleSystem && nt->PS->isLoaded())
		{						
			_ParticleDlg->setActiveNode(nt->PS);
			setActiveNode(nt->PS);
			return;
		}
	}
	CTreeCtrl::OnLButtonDblClk(nFlags, point);
}

//****************************************************************************************************************
void CParticleTreeCtrl::sortWorkspace(CParticleWorkspace &ws, CParticleWorkspace::ISort &sorter)
{
	// stop all fx
	nlassert(_ParticleDlg);
	_ParticleDlg->StartStopDlg->stop();
	CParticleWorkspace::CNode *activeNode = _ParticleDlg->getActiveNode();
	setActiveNode(NULL);
	reset();
	ws.sort(sorter);
	buildTreeFromWorkSpace(ws);
	Select(GetRootItem(), TVGN_FIRSTVISIBLE);
	setActiveNode(activeNode);
	expandRoot();
}

//****************************************************************************************************************
void CParticleTreeCtrl::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	/*
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;
	if (pTVKeyDown->wVKey == VK_DELETE)
	{
		deleteSelection();
	}	
	*pResult = 0;
	*/
}

//****************************************************************************************************************
void CParticleTreeCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	/*
	if (nChar == VK_DELETE)
	{
		deleteSelection();
	}		
	*/
	CTreeCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
}

//****************************************************************************************************************
void CParticleTreeCtrl::expandRoot()
{
	if (GetRootItem()) Expand(GetRootItem(), TVE_EXPAND);
}
	
//****************************************************************************************************************
void CParticleTreeCtrl::setViewFilenameFlag(bool enabled)
{
	if (enabled == _ViewFilenameFlag) return;
	_ViewFilenameFlag = enabled;
	updateAllCaptions();
}

//****************************************************************************************************************
void CParticleTreeCtrl::updateAllCaptions()
{
	if (!GetRootItem()) return;
	std::stack<HTREEITEM> leftToTraverse;
	leftToTraverse.push(GetRootItem());
	// TODO: factorize code to traverse all nodes
	while (!leftToTraverse.empty())
	{
		HTREEITEM curr = leftToTraverse.top();
		leftToTraverse.pop();
		CNodeType *nt = (CNodeType *) GetItemData(curr);
		switch(nt->Type)
		{
			case CNodeType::particleSystem:
				SetItemText(curr, nlUtf8ToTStr(computeCaption(*nt->PS)));
			break;
			case CNodeType::workspace:
			    SetItemText(curr, nlUtf8ToTStr(computeCaption(*nt->WS)));
			break;
			case CNodeType::located:			
			case CNodeType::locatedBindable:			
			case CNodeType::locatedInstance:
				// no-op
			break;
			default:
				nlassert(0);
			break;
		}
		if (GetChildItem(curr)) leftToTraverse.push(GetChildItem(curr));
		if (GetNextSiblingItem(curr)) leftToTraverse.push(GetNextSiblingItem(curr));
	}	
}

//****************************************************************************************************************
void CParticleTreeCtrl::removeAllPS(CParticleWorkspace &ws)
{
	if (localizedMessageBox(*this, IDS_REMOVE_ALL_PS, IDS_PARTICLE_SYSTEM_EDITOR, MB_OKCANCEL|MB_ICONQUESTION) != IDOK) return;
	setActiveNode(NULL);
	_ParticleDlg->setActiveNode(NULL);	
	uint numNodes = ws.getNumNode();
	for(uint k = 0; k < numNodes; ++k)
	{
		ws.removeNode((uint) 0);
	}
	reset();
	buildTreeFromWorkSpace(ws);
}



