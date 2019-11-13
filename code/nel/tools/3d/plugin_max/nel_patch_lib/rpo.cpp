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

#include "stdafx.h"
#include "rpo.h"

using namespace NL3D;

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif max

#define PBLOCK_REF	0

#ifdef USE_CACHE
#ifndef NDEBUG
#define DEBUG_PIPELINE
#endif // NDEBUG
#endif // USE_CACHE


class RPOClassDesc:public ClassDesc 
{
	public:
	int 			IsPublic() 
	{
		return 0;
	}
	void *			Create(BOOL loading = FALSE) 
	{
		return new RPO;
	}
	const MCHAR *	ClassName() 
	{
		return _M("RklPatch");
	}
	SClass_ID		SuperClassID() 
	{
		return GEOMOBJECT_CLASS_ID;
	}
	Class_ID		ClassID() 
	{
		return RYKOLPATCHOBJ_CLASS_ID;
	}
	const MCHAR* 	Category() 
	{
		return _M("Rykol Tools");
	}
};


enum { RPO_params };

//TODO: Add enums for various parameters
enum { pb_spin };

IObjParam *RPO::ip			= NULL;

BOOL PatchObject::attachMat = FALSE;
BOOL PatchObject::condenseMat = FALSE;

static RPOClassDesc RPODesc;

ClassDesc* GetRPODesc()
{
	return &RPODesc;
}

// RPO ------------------------------------------------------------------------------------------------------------------------------------------------

RPO::RPO()
{
	RPODesc.MakeAutoParamBlocks(this);
	bBigHack=false;

	rpatch = NULL;

	// Infinite
	topoValid.SetInfinite();
	geomValid.SetInfinite();
	selectValid.SetInfinite();
	texmapValid.SetInfinite();
	validBits = 0xffffffff;
}

RPO::RPO(PatchObject& pPO) : PatchObject(pPO)
{
	RPODesc.MakeAutoParamBlocks(this);

	rpatch = new RPatchMesh (&pPO.patch);

	//Mode=ModeRykolPatchMesh;
	bBigHack=false;

	// Infinite
	topoValid.SetInfinite();
	geomValid.SetInfinite();
	selectValid.SetInfinite();
	texmapValid.SetInfinite();
	validBits = 0xffffffff;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

RPO::~RPO()
{
	if ( (~((PartID)GetChannelLocks())) & PART_TOPO )
	{
		delete rpatch;
		rpatch=NULL;
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

void RPO::BeginEditParams(IObjParam *ip,ULONG flags,Animatable *prev)
{
	this->ip = ip;
	//RPODesc.BeginEditParams(ip, this, flags, prev);
	//PatchObject::BeginEditParams(ip, flags, prev);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

void RPO::EndEditParams( IObjParam *ip, ULONG flags,Animatable *next )
{
	//TODO: Save plugin parameter values into class variables, if they are not hosted in ParamBlocks. 
	

	//RPODesc.EndEditParams(ip, this, flags, next);
	this->ip = NULL;
	//PatchObject::EndEditParams(ip, flags, next);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

//From Object
BOOL RPO::HasUVW() 
{ 
	//TODO: Return whether the object has UVW coordinates or not
	return TRUE; 
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

void RPO::SetGenUVW(BOOL sw) 
{  
	if (sw==HasUVW()) return;
	//TODO: Set the plugin's internal value to sw				
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

//Class for interactive creation of the object using the mouse
class RPOCreateCallBack : public CreateMouseCallBack 
{
	IPoint2 sp0;		//First point in screen coordinates
	RPO *ob;		//Pointer to the object 
	Point3 p0;			//First point in world coordinates
public:	
	int proc( ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat);
	void SetObj(RPO *obj) 
	{
		ob = obj;
	}
};

// ------------------------------------------------------------------------------------------------------------------------------------------------

int RPOCreateCallBack::proc(ViewExp *vpt,int msg, int point, int flags, IPoint2 m, Matrix3& mat )
{
	//TODO: Implement the mouse creation code here
	if (msg==MOUSE_POINT||msg==MOUSE_MOVE) 
	{
		switch(point) 
		{
		case 0: // only happens with MOUSE_POINT msg
			ob->suspendSnap = TRUE;
			sp0 = m;
			p0 = vpt->SnapPoint(m,m,NULL,SNAP_IN_PLANE);
			mat.SetTrans(p0);
			break;
		//TODO: Add for the rest of points
		}
	} 
	else 
	{
		if (msg == MOUSE_ABORT) 
		{
			return CREATE_ABORT;
		}
	}
	return TRUE;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

static RPOCreateCallBack RPOCreateCB;

//From BaseObject
CreateMouseCallBack* RPO::GetCreateMouseCallBack() 
{
	RPOCreateCB.SetObj(this);
	return(&RPOCreateCB);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

int RPO::Display(TimeValue t, INode* inode, ViewExp *vpt, int flags)
{
	//TODO: Implement the displaying of the object here
	if (!rpatch->rTess.ModeTile)
	{
		int ret= PatchObject::Display(t, inode, vpt, flags);

		// Point binded
		if (inode->Selected())
		{
			GraphicsWindow *gw = vpt->getGW();
			gw->setColor(LINE_COLOR, 0, 0, 0);
			for (int i=0; i<patch.numVerts; i++)
			{
				if (rpatch->getUIVertex (i).Binding.bBinded)
				{
					// draw the point
					gw->marker (&patch.verts[i].p, DOT_MRKR);
				}
			}
		}

		return ret;

	}
	else	
	{
		try
		{
			return rpatch->Display (t, inode, vpt, flags, patch);
		}
		catch (std::exception e)
		{
			nlwarning("RPO::Display failed");
			return 0;
		}
	}
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

int RPO::HitTest(TimeValue t, INode* inode, int type, int crossing, 
	int flags, IPoint2 *p, ViewExp *vpt)
{
	//TODO: Implement the hit testing here
	return PatchObject::HitTest(t, inode, type, crossing, flags, p, vpt);
	//return 0;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

void RPO::Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt)
{
	//TODO: Check the point passed for a snap and update the SnapInfo structure
	PatchObject::Snap(t, inode, snap, p, vpt);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

void RPO::GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box )
{
	//TODO: Return the world space bounding box of the object
	PatchObject::GetWorldBoundBox(t, mat, vpt, box);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

void RPO::GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box )
{
	//TODO: Return the local space bounding box of the object
	PatchObject::GetLocalBoundBox(t, mat, vpt, box);
}	

// ------------------------------------------------------------------------------------------------------------------------------------------------

void RPO::GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel )
{
	//TODO: Compute the bounding box in the objects local coordinates 
	//		or the optional space defined by tm.
	PatchObject::GetDeformBBox(t, box, tm, useSel);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

//From ReferenceMaker
RefResult RPO::NotifyRefChanged(NOTIFY_REF_PARAMS)
{
	//TODO: Implement, if the object makes references to other things
	//return PatchObject::NotifyRefChanged( changeInt, hTarget, partID, message, propagate);
	return(REF_SUCCEED);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

Mesh* RPO::GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete)
{
	//TODO: Return the mesh representation of the object used by the renderer
	needDelete=TRUE;
	Mesh *pMesh=new Mesh();
	rpatch->BuildMesh(t, patch, pMesh);
	return pMesh;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

/*
  Lire doc intervals
  Ici : conversion d'un RPM en n'importe kwa : triObj, PO
  If this returns a pointer other than itself, it must be deleted by whatever called it.
*/
Object* RPO::ConvertToType(TimeValue t, Class_ID cid)
{
	if (cid == RYKOLPATCHOBJ_CLASS_ID) return this;
	//if (cid == patchObjectClassID)
	//{
	//  --- uncomment this if you want to do conversion from rkl back to max patch (you will lose rkl data in the zone)
	//	PatchObject *o = new PatchObject();
	//	o->patch = patch;
	//	return o;
	//}
	if (cid == patchObjectClassID) return this;
	if (cid == defObjectClassID) return this;
	if (cid == triObjectClassID) 
	{		
		TriObject *pTriObj=CreateNewTriObject();
		rpatch->InvalidateChannels (PART_TOPO|PART_GEOM|PART_TEXMAP|PART_SELECT|PART_DISPLAY);
		rpatch->BuildMesh(t, patch, &pTriObj->mesh);
		pTriObj->SetChannelValidity(TOPO_CHAN_NUM,ConvertValidity(t));
		pTriObj->SetChannelValidity(GEOM_CHAN_NUM,ConvertValidity(t));
		return pTriObj;
	}
	return PatchObject::ConvertToType(t, cid);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

int RPO::CanConvertToType(Class_ID cid)
{
	if (cid == RYKOLPATCHOBJ_CLASS_ID) return 1;
	if (cid == patchObjectClassID) return 1;
	if (cid == defObjectClassID) return 1;
	if (cid == triObjectClassID) return 1;
	return PatchObject::CanConvertToType(cid);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

// From Object
int RPO::IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm)
{
	//TODO: Return TRUE after you implement this method
	return PatchObject::IntersectRay( t, ray, at, norm);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

void RPO::GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist)
{
	Object::GetCollapseTypes(clist, nlist);
	//TODO: Append any any other collapse type the plugin supports
	
    Class_ID id = RYKOLPATCHOBJ_CLASS_ID;
    TSTR *name = new TSTR(_T("Rykol Patch Mesh"));
    clist.Append(1,&id);
    nlist.Append(1,&name);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

// From ReferenceTarget
RefTargetHandle RPO::Clone(RemapDir& remap) 
{
	RPO* newob = new RPO();
	newob->rpatch=new RPatchMesh ();
	*newob->rpatch=*rpatch;
	newob->rpatch->selLevel=EP_OBJECT;
	newob->rpatch->InvalidateChannels(ALL_CHANNELS);
	newob->patch.DeepCopy(&patch, ALL_CHANNELS);
	newob->patch.SetShowInterior(patch.GetShowInterior());

	return(newob);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

#define CHK_TEST	0x0001
#define VALIDITY_CHUNK 	2301
static int counter;
IOResult RPO::Save(ISave *isave)
{
	ULONG nb;

	//PatchObject::Save(isave);

	// Version
	isave->BeginChunk(VALIDITY_CHUNK);

	unsigned int nVersion=RPO_SERIALIZE_VERSION;
	isave->Write(&nVersion, sizeof (nVersion), &nb);

	// RPatch
	rpatch->Save (isave);
	
	isave->EndChunk();
	patch.Save (isave);
	rpatch->mesh.Save (isave);

	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

IOResult RPO::Load(ILoad *iload)
{
	ULONG nb;

	if (rpatch==NULL)
		rpatch=new RPatchMesh ();

	if (IO_OK==(iload->OpenChunk()))
	{
		if (iload->CurChunkID()==VALIDITY_CHUNK)
		{
			// Version
			unsigned int nVersion;
			iload->Read(&nVersion, sizeof (nVersion), &nb);

			switch (nVersion)
			{
			case RPO_SERIALIZE_VERSION:
				// RPatch
				rpatch->Load (iload);
				break;
			}

		}
		iload->CloseChunk();
	}
	patch.Load (iload);
	rpatch->mesh.Load (iload);

	return IO_OK;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

Object *RPO::MakeShallowCopy(ChannelMask channels)
{
	RPO* newob=(RPO*)RPODesc.Create();

	if (channels&PART_TOPO)
		newob->rpatch=rpatch;

	// Copy patch mesh
	newob->patch.ShallowCopy(&patch, channels);

	// Copy validity
	newob->CopyValidity(this,channels);

	return newob;
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

void RPO::ShallowCopy(Object* fromOb, ChannelMask channels)
{
	RPO* fromRPO=(RPO*)fromOb;

	// Copy rpatch mesh
	if (channels&PART_TOPO)
		rpatch=fromRPO->rpatch;

	// Copy patch mesh
	patch.ShallowCopy(&fromRPO->patch, channels);

	// Copy validity
	CopyValidity(fromRPO, channels);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

void RPO::NewAndCopyChannels(ChannelMask channels)
{
	if (channels&PART_TOPO)
	{
		RPatchMesh* old=rpatch;
		rpatch=new RPatchMesh ();
		*rpatch=*old;
	}
	PatchObject::NewAndCopyChannels(channels);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

void RPO::FreeChannels(ChannelMask channels)
{
	if ((channels&(~GetChannelLocks()))&PART_TOPO)
	{
		delete rpatch;
		rpatch=NULL;
	}
	PatchObject::FreeChannels(channels);
}

// ------------------------------------------------------------------------------------------------------------------------------------------------

void RPO::InvalidateChannels(ChannelMask channels)
{
	//PatchObject::InvalidateChannels(channels);
	if (rpatch)
		rpatch->InvalidateChannels(channels);

	for (int i=0; i<NUM_OBJ_CHANS; i++) 
	{
		if (channels&chMask[i]) 
		{
			switch(i) 
			{
				case GEOM_CHAN_NUM: geomValid.SetEmpty(); break;
				//case VERT_COLOR_CHAN_NUM: vcolorValid.SetEmpty(); break;
				case TOPO_CHAN_NUM: topoValid.SetEmpty(); break;
				case TEXMAP_CHAN_NUM: texmapValid.SetEmpty(); break;
				case SELECT_CHAN_NUM: selectValid.SetEmpty(); break;
				//case GFX_DATA_CHAN_NUM: gfxdataValid.SetEmpty(); break;
				default: validBits &= ~chMask[i]; break;
			}
		}
	}
}

// --------------------------------------------------

void RPO::PointsWereChanged()
{
	PatchObject::PointsWereChanged();
	rpatch->InvalidateBindingPos ();
}

// --------------------------------------------------

bool RPO::isZone (INode& node, TimeValue time)
{
	// Result false by default
	bool bRet=false;

	// Eval the object a time
	ObjectState os = node.EvalWorldState(time);

	// Object exist ?
	if (os.obj)
	{
		// Object can convert itself to NeL patchmesh ?
		if (os.obj->CanConvertToType(RYKOLPATCHOBJ_CLASS_ID))
			bRet=true;
	}

	// Return result
	return bRet;
}

// --------------------------------------------------

