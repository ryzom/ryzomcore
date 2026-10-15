// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019-2022  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef __RYKOL_PATCH_OBJ_H
#define __RYKOL_PATCH_OBJ_H

#pragma warning (disable : 4786)
#include "nel/misc/debug.h"
#include <tchar.h>
#include <vector>
#include <set>
#include <string>
#include "nel/3d/tile_bank.h"
#include "nel/misc/file.h"
#include "nel_patch_mesh.h"

//#define USE_CACHE

typedef unsigned int uint;

#define RYKOLPATCHOBJ_CLASS_ID	Class_ID(0x368c679f, 0x711c22ee)

extern TCHAR *GetString(int id);

extern HINSTANCE hInstance;

#define RPO_SERIALIZE_VERSION 0

#define EP_OBJECT	0
#define EP_VERTEX	1
#define EP_EDGE		2
#define EP_PATCH	3
#define EP_TILE		4

#define PO_TILE		4

#define PATCH_HIT_TILE (PATCH_HIT_INTERIOR+1)

#define MAX_TILE_IN_PATCH 16
#define NUM_TILE_SEL (MAX_TILE_IN_PATCH*MAX_TILE_IN_PATCH)

#pragma warning (disable : 4786)

// ------------------------------------------------------------------------------------------------------------------------------------------------

class RPO : public PatchObject
{
	public:
//		PatchObject			PO;
		RPatchMesh			*rpatch;			// User info for this RykolPatchObject

		// bug hack
		bool bBigHack;

		// Validity
		Interval topoValid;
		Interval geomValid;
		Interval selectValid;
		Interval texmapValid;
		/*Interval vcolorValid;
		Interval gfxdataValid;*/
		DWORD validBits; // for the remaining constant channels

		//Class vars
		static IObjParam *ip;			//Access to the interface
		BOOL suspendSnap;				//A flag for setting snapping on/off
		// From BaseObject
		virtual CreateMouseCallBack* GetCreateMouseCallBack() NL_OVERRIDE;
		virtual int Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) NL_OVERRIDE;
		virtual int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) NL_OVERRIDE;
		virtual void Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) NL_OVERRIDE;
		//TODO: Return the name that will appear in the history browser (modifier stack)
		virtual GET_OBJECT_NAME_CONST MCHAR *GetObjectName(NL_GET_OBJECT_NAME_PARAMS) NL_GET_CLASS_NAME_CONST NL_OVERRIDE { return _M("Rykol Patch Object");}
		
		virtual void GetWorldBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box ) NL_OVERRIDE;
		virtual void GetLocalBoundBox(TimeValue t, INode *mat, ViewExp *vpt, Box3& box ) NL_OVERRIDE;

		virtual void GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel ) NL_OVERRIDE;
		//TODO: Return the default name of the node when it is created.
		virtual void InitNodeName(TSTR& s) NL_OVERRIDE { s = _M("Rykol Patch Object"); }
		
		// From Object
		virtual BOOL HasUVW() NL_OVERRIDE;
		virtual void SetGenUVW(BOOL sw) NL_OVERRIDE;
		virtual int CanConvertToType(Class_ID obtype) NL_OVERRIDE;
		virtual Object* ConvertToType(TimeValue t, Class_ID obtype) NL_OVERRIDE;
		virtual void GetCollapseTypes(Tab<Class_ID> &clist,Tab<TSTR*> &nlist) NL_OVERRIDE;
		virtual int IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm) NL_OVERRIDE;
		virtual void PointsWereChanged() NL_OVERRIDE;
		
		//TODO: Evaluate the object and return the ObjectState
		virtual ObjectState Eval(TimeValue t) NL_OVERRIDE
		{
			return ObjectState(this); 
		};		
		
		//TODO: Return the validity interval of the object as a whole
		virtual Interval ObjectValidity(TimeValue t) NL_OVERRIDE
		{ 
			Interval iv;
			iv.SetInfinite();
			iv &= geomValid;
			//iv &= vcolorValid;
			iv &= topoValid;
			iv &= texmapValid;
			iv &= selectValid;
			//iv &= gfxdataValid;
			
			if (!(validBits&chMask[SUBSEL_TYPE_CHAN_NUM])) iv.SetEmpty();
			if (!(validBits&chMask[DISP_ATTRIB_CHAN_NUM])) iv.SetEmpty();
			return iv;
		}
		
		virtual Interval ChannelValidity(TimeValue t, int nchan) NL_OVERRIDE
		{
			switch(nchan) 
			{
				case GEOM_CHAN_NUM: return geomValid; break;
				case TOPO_CHAN_NUM: return topoValid; break;
				case TEXMAP_CHAN_NUM: return texmapValid; break;
				case SELECT_CHAN_NUM: return selectValid; break;
				default:
					return((chMask[nchan]&validBits) ? FOREVER: NEVER);
			}
		}
		
		virtual void SetChannelValidity(int nchan, Interval v) NL_OVERRIDE
		{
			switch(nchan) 
			{
				case GEOM_CHAN_NUM: geomValid = v; break;
				//case VERT_COLOR_CHAN_NUM: vcolorValid = v; break;
				case TOPO_CHAN_NUM: topoValid = v; break;
				case TEXMAP_CHAN_NUM: texmapValid = v; break;
				case SELECT_CHAN_NUM: selectValid = v; break;
				//case GFX_DATA_CHAN_NUM: gfxdataValid = v; break;
				default :
					//if (v.InInterval(0)) validBits|= chMask[nchan];
					if (!(v==NEVER)) validBits|= chMask[nchan];
					else validBits &= ~chMask[nchan];
					break;
			}	
		}
		
		Interval ConvertValidity(TimeValue t)
		{
			Interval iv = FOREVER;	
			if (geomValid.InInterval(t)) iv &= geomValid;
			//if (vcolorValid.InInterval(t)) iv &= vcolorValid;
			if (topoValid.InInterval(t)) iv &= topoValid;	
			if (texmapValid.InInterval(t)) iv &= texmapValid;	
			if (selectValid.InInterval(t)) iv &= selectValid;	
			return iv;
		}

#define copyFlags(dest, source, mask) dest =  ((dest&(~mask))|(source&mask))
		
		void CopyValidity(RPO *fromOb, ChannelMask channels) 
		{
			if (channels&GEOM_CHANNEL) geomValid = fromOb->geomValid;
			//if (channels&VERTCOLOR_CHANNEL) vcolorValid = fromOb->vcolorValid;
			if (channels&TOPO_CHANNEL) topoValid = fromOb->topoValid;
			if (channels&TEXMAP_CHANNEL) texmapValid = fromOb->texmapValid;
			if (channels&SELECT_CHANNEL) selectValid = fromOb->selectValid;
			//if (channels&GFX_DATA_CHANNEL) gfxdataValid = fromOb->gfxdataValid;
			copyFlags(validBits, fromOb->validBits,channels);
		}

		virtual void InvalidateChannels(ChannelMask channels) NL_OVERRIDE;

		// From Animatable
		virtual void BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev) NL_OVERRIDE;
		virtual void EndEditParams( IObjParam *ip, ULONG flags,Animatable *next) NL_OVERRIDE;

		// From GeomObject
		virtual Mesh* GetRenderMesh(TimeValue t, INode *inode, View& view, BOOL& needDelete) NL_OVERRIDE;
		
		// Loading/Saving
		virtual IOResult Load(ILoad *iload) NL_OVERRIDE;
		virtual IOResult Save(ISave *isave) NL_OVERRIDE;

		//From Animatable
		virtual Class_ID ClassID() NL_OVERRIDE
		{
			if (bBigHack)
				return Class_ID(PATCHOBJ_CLASS_ID,0);
			else
				return RYKOLPATCHOBJ_CLASS_ID;
		}
		virtual BOOL IsSubClassOf(Class_ID classID) NL_OVERRIDE
		{
			return classID == ClassID() 
				? true : PatchObject::IsSubClassOf(classID);
		}
		virtual SClass_ID SuperClassID() NL_OVERRIDE { return GEOMOBJECT_CLASS_ID; }
		virtual void GetClassName(NL_GET_CLASS_NAME_PARAMS) NL_GET_CLASS_NAME_CONST NL_OVERRIDE { s = _T("Rykol Patch Object");}
		
		virtual RefTargetHandle Clone ( RemapDir &remap ) NL_OVERRIDE;
		virtual RefResult NotifyRefChanged (NOTIFY_REF_PARAMS) NL_OVERRIDE;

		virtual int NumSubs() NL_OVERRIDE
		{ 
			return PatchObject::NumSubs ();
		}
		//TSTR SubAnimName(int i) { return NULL; }
		virtual Animatable* SubAnim(int i) NL_OVERRIDE 
		{ 
			return PatchObject::SubAnim(i);
			// return pblock; 
		}
		virtual int NumRefs() NL_OVERRIDE
		{ 
			return PatchObject::NumRefs();
		}
		virtual RefTargetHandle GetReference(int i) NL_OVERRIDE
		{ 
			return PatchObject::GetReference(i);
			//return pblock; 
		}
		virtual void SetReference(int i, RefTargetHandle rtarg) NL_OVERRIDE
		{ 
			PatchObject::SetReference(i, rtarg);
			//pblock=(IParamBlock2*)rtarg; 
		}

		/*int	NumParamBlocks() 
		{ 
			//return PatchObject::NumParamBlocks();
			return 1; 
		}					// return number of ParamBlocks in this instance
		IParamBlock2* GetParamBlock(int i) 
		{ 
			//return PatchObject::GetParamBlock(i);
			return pblock; 
		} // return i'th ParamBlock
		IParamBlock2* GetParamBlockByID(BlockID id) 
		{ 
			return PatchObject::GetParamBlockByID(id);
			//return (pblock->ID() == id) ? pblock : NULL; 
		} // return id'd ParamBlock*/
		virtual void DeleteThis() NL_OVERRIDE { delete this; }
		virtual Object *MakeShallowCopy(ChannelMask channels) NL_OVERRIDE;
		virtual void ShallowCopy(Object* fromOb, ChannelMask channels) NL_OVERRIDE;
		virtual void NewAndCopyChannels(ChannelMask channels) NL_OVERRIDE;
		virtual void FreeChannels(ChannelMask channels) NL_OVERRIDE;


		//Constructor/Destructor
		RPO();
		RPO(PatchObject& pPO);
		virtual ~RPO();

		virtual void SetPoint(int i, const Point3& p) NL_OVERRIDE
		{
			PatchObject::SetPoint(i, p);
		}

		/**
		  * Return true if it is a zone.
		  *
		  * skeletonShape must be NULL if no bones.
		  */
		static bool						isZone (INode& node, TimeValue time);

#if MAX_RELEASE >= 4000
		// ace: These functions are needed in max 4
		// NS: New SubObjType API
		int NumSubObjTypes() { return 0; }
		ISubObjType *GetSubObjType(int i) { return NULL; }
#endif

};

#endif // __RYKOL_PATCH_OBJ_H
