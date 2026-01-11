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

#include "std3d.h"

#include "nel/3d/mini_col.h"
#include "nel/misc/aabbox.h"
#include "nel/3d/quad_grid.h"

using namespace NLMISC;
using namespace std;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{

static const	sint	QuadDepth= 10;


// Element for grid lookup.
static const sint	GridSize=512;
static const float	GridEltSize=2;


// ***************************************************************************
CMiniCol::CMiniCol()
{
	_RadMin= 100;
	_RadMax= 125;
	_Grid.create(GridSize, GridEltSize);
}


// ***************************************************************************
void			CMiniCol::addFaces(const std::vector<CTriangle> &faces, uint16 zoneId, uint16 patchId)
{
	for(sint i=0;i<(sint)faces.size();i++)
	{
		const CTriangle	&f= faces[i];
		CAABBox	box;
		box.setCenter(f.V0);
		box.extend(f.V1);
		box.extend(f.V2);
		CFace	node;
		node.Face= f;
		node.Plane.make(f.V0, f.V1, f.V2);
		node.ZoneId= zoneId;
		node.PatchId=patchId;
		_Grid.insert(box.getMin(), box.getMax(), node);
	}
}


// ***************************************************************************
void			CMiniCol::addLandscapePart(uint16 zoneId, uint16 patchId)
{
	vector<CTriangle> faces;
	_Landscape->buildCollideFaces(zoneId, patchId, faces);
	addFaces(faces, zoneId, patchId);
}


// ***************************************************************************
void			CMiniCol::removeLandScapePart(uint16 zoneId, uint16 patchId, const CBSphere &sphere)
{
	// Build the AAbox which englobe the bsphere of the patch.
	CAABBox		bb;
	bb.setCenter(sphere.Center);
	float	l= sphere.Radius;
	bb.setHalfSize(CVector(l,l,l));

	// For optimisation, select only faces which are IN the bbox of the patch.
	_Grid.select(bb.getMin(), bb.getMax());
	CQuadGrid<CFace>::CIterator	iFace;
	for(iFace= _Grid.begin();iFace!=_Grid.end();)
	{
		if((*iFace).isFromPatch(zoneId, patchId))
			iFace= _Grid.erase(iFace);
		else
			iFace++;
	}
}


// ***************************************************************************
void			CMiniCol::init(CLandscape *land, float radMin, float radDelta)
{
	_Landscape= land;
	_RadMin= radMin;
	_RadMax= radMin+radDelta;
}


// ***************************************************************************
void			CMiniCol::addZone(uint16 zoneId)
{
	CZoneIdent	newZone;

	// landscape must have been inited.
	nlassert(_Landscape);
	const CZone	*zone= _Landscape->getZone(zoneId);
	// zone must be loaded into landscape.
	nlassert(zone);

	// Fill the newzone.
	newZone.ZoneId= zoneId;
	newZone.Sphere.Center= zone->getZoneBB().getCenter();
	newZone.Sphere.Radius= zone->getZoneBB().getRadius();
	newZone.Patchs.resize(zone->getNumPatchs());
	for(sint i=0;i<zone->getNumPatchs();i++)
	{
		newZone.Patchs[i].Sphere= zone->getPatchBSphere(i);
	}

	// Add it to the set (if not already done...).
	_Zones.insert(newZone);
}


// ***************************************************************************
void			CMiniCol::removeZone(uint16 zoneId)
{
	CZoneIdent	delZone;


	// First, delete all patch from the grid.
	//=======================================
	// Fill the key part only.
	delZone.ZoneId= zoneId;
	// Find the zone (or quit).
	TZoneSet::iterator	itZone;
	itZone= _Zones.find(delZone);
	if(itZone==_Zones.end())
		return;

	CZoneIdent	&zone= const_cast<CZoneIdent&>(*itZone);
	for(sint i=0;i<(sint)zone.Patchs.size();i++)
	{
		CPatchIdent	&pa= zone.Patchs[i];
		if(pa.Inserted)
		{
			// Reject the patch.
			removeLandScapePart(uint16(zone.ZoneId), uint16(i), pa.Sphere);
			pa.Inserted= false;
			zone.NPatchInserted--;
		}
	}

	// Then, delete it.
	//=================
	_Zones.erase(delZone);

}


// ***************************************************************************
void			CMiniCol::setCenter(const CVector& center)
{
	CBSphere	BMin(center, _RadMin), BMax(center, _RadMax);

	// For all zones, test if must insert patchs..
	TZoneSet::iterator	itZone;
	for(itZone= _Zones.begin();itZone!=_Zones.end();itZone++)
	{
		CZoneIdent	&zone= const_cast<CZoneIdent&>(*itZone);

		// Tests must be done in 2D...
		BMin.Center.z= zone.Sphere.Center.z;
		BMax.Center.z= zone.Sphere.Center.z;

		// Must test first if the zone is IN the area.
		//=============================================
		bool	zoneIn= false;
		if(zone.NPatchInserted==0)
		{
			if(BMin.intersect(zone.Sphere))
				zoneIn= true;
		}
		else
			zoneIn= true;

		// Then for all patchs, must test if the patch must be inserted, or rejected.
		//=============================================
		if(zoneIn)
		{
			for(sint i=0;i<(sint)zone.Patchs.size();i++)
			{
				CPatchIdent	&pa= zone.Patchs[i];

				// Tests must be done in 2D...
				BMin.Center.z= pa.Sphere.Center.z;
				BMax.Center.z= pa.Sphere.Center.z;

				if(pa.Inserted)
				{
					// Reject the patch, if entirely OUT the max radius.
					if(!BMax.intersect(pa.Sphere))
					{
						removeLandScapePart(uint16(zone.ZoneId), uint16(i), pa.Sphere);
						pa.Inserted= false;
						zone.NPatchInserted--;
					}
				}
				else
				{
					// Insert the pacth, if only partially IN the min radius.
					if(BMin.intersect(pa.Sphere))
					{
						addLandscapePart(uint16(zone.ZoneId), uint16(i));
						pa.Inserted= true;
						zone.NPatchInserted++;
					}
				}
			}
		}
	}
}


// ***************************************************************************
bool			CMiniCol::snapToGround(CVector &pos, float hup, float hbot)
{
	CVector	b1,b2;
	bool	found=false;
	float	height=0.f;


	// Select quad nodes which contains pos.
	b1=b2=pos;
	b1.z-= hbot;
	b2.z+= hup;
	// Select.
	_Grid.select(b1,b2);

	// For each face, test if it is under pos, then test if height is correct.
	CQuadGrid<CFace>::CIterator	iFace;
	for(iFace= _Grid.begin();iFace!=_Grid.end();iFace++)
	{
		CTriangle	&pFace= (*iFace).Face;
		CPlane		&pPlane= (*iFace).Plane;
		// Order is important.
		CVector		&p0= pFace.V0;
		CVector		&p1= pFace.V1;
		CVector		&p2= pFace.V2;

		// TOIMP: This is VERY SLOW!!! (hope that the quadtree will help, but it still very slow...).

		// Yoyo Debug, test, if the point may be IN the bbox.
		/*CAABBox		bbFace;
		bbFace.setCenter(p0);
		bbFace.extend(p1);
		bbFace.extend(p2);
		CVector		bext=p0;
		bext.z= maxof(p0.z, p1.z, p2.z)+hbot;
		bbFace.extend(bext);
		bext.z= minof(p0.z, p1.z, p2.z)-hup;
		bbFace.extend(bext);
		if(!bbFace.include(pos))
			continue;*/

		// Test if the face enclose the pos in X/Y plane.
		// NB: compute and using a BBox to do a rapid test is not a very good idea, since it will
		// add an overhead which is NOT negligeable compared to the following test.
		float		a,b,c;		// 2D cartesian coefficients of line in plane X/Y.
		// Line p0-p1.
		a= -(p1.y-p0.y);
		b= (p1.x-p0.x);
		c= -(p0.x*a + p0.y*b);
		if( (a*pos.x + b*pos.y + c) < 0)	continue;
		// Line p1-p2.
		a= -(p2.y-p1.y);
		b= (p2.x-p1.x);
		c= -(p1.x*a + p1.y*b);
		if( (a*pos.x + b*pos.y + c) < 0)	continue;
		// Line p2-p0.
		a= -(p0.y-p2.y);
		b= (p0.x-p2.x);
		c= -(p2.x*a + p2.y*b);
		if( (a*pos.x + b*pos.y + c) < 0)	continue;


		// Compute the possible height.
		CVector		tmp;
		// intersect the vertical line with the plane.
		tmp= pPlane.intersect(pos, pos-CVector(0,0,100));

		/*
		// CTriangle intersect() method.
		CVector	tmp;
		if(pFace.intersect(b1, b2, tmp, pPlane))
		*/
		{
			float		h= tmp.z;
			// Test if it would fit in the wanted field.
			if(h>pos.z+hup)	continue;
			if(h<pos.z-hbot)	continue;

			// OK!!
			if(!found)
			{
				found=true;
				height=h;
			}
			else
			{
				height= max(height,h);
			}
		}
	}

	if(found)
		pos.z= height;

	return found;
}



// ***************************************************************************
bool			CMiniCol::getGroundNormal(const CVector &pos, CVector &normal, float hup, float hbot)
{
	CVector	b1,b2;
	bool	found=false;
	float	height=0.0;


	// Select quad nodes which contains pos.
	b1=b2=pos;
	b1.z-= hbot;
	b2.z+= hup;
	// Select.
	_Grid.select(b1,b2);

	// For each face, test if it is under pos, then test if height is correct.
	CQuadGrid<CFace>::CIterator	iFace;
	for(iFace= _Grid.begin();iFace!=_Grid.end();iFace++)
	{
		CTriangle	&pFace= (*iFace).Face;
		CPlane		&pPlane= (*iFace).Plane;
		// Order is important.
		CVector		&p0= pFace.V0;
		CVector		&p1= pFace.V1;
		CVector		&p2= pFace.V2;

		// TOIMP: This is VERY SLOW!!! (hope that the quadtree will help, but it still very slow...).

		// Yoyo Debug, test, if the point may be IN the bbox.
		CAABBox		bbFace;
		bbFace.setCenter(p0);
		bbFace.extend(p1);
		bbFace.extend(p2);
		CVector		bext=p0;
		bext.z= maxof(p0.z, p1.z, p2.z)+hbot;
		bbFace.extend(bext);
		bext.z= minof(p0.z, p1.z, p2.z)-hup;
		bbFace.extend(bext);
		if(!bbFace.include(pos))
			continue;

		// Test if the face enclose the pos in X/Y plane.
		// NB: compute and using a BBox to do a rapid test is not a very good idea, since it will
		// add an overhead which is NOT negligeable compared to the following test.
		float		a,b,c;		// 2D cartesian coefficients of line in plane X/Y.
		// Line p0-p1.
		a= -(p1.y-p0.y);
		b= (p1.x-p0.x);
		c= -(p0.x*a + p0.y*b);
		if( (a*pos.x + b*pos.y + c) < 0)	continue;
		// Line p1-p2.
		a= -(p2.y-p1.y);
		b= (p2.x-p1.x);
		c= -(p1.x*a + p1.y*b);
		if( (a*pos.x + b*pos.y + c) < 0)	continue;
		// Line p2-p0.
		a= -(p0.y-p2.y);
		b= (p0.x-p2.x);
		c= -(p2.x*a + p2.y*b);
		if( (a*pos.x + b*pos.y + c) < 0)	continue;


		// Compute the possible height.
		CVector		tmp;
		// intersect the vertical line with the plane.
		tmp= pPlane.intersect(pos, pos-CVector(0,0,100));
		float		h= tmp.z;
		// Test if it would fit in the wanted field.
		if(h>pos.z+hup)	continue;
		if(h<pos.z-hbot)	continue;

		// OK!!
		if(!found)
		{
			found=true;
			height=h;
			normal= pPlane.getNormal();
		}
		else
		{
			if(h>height)
			{
				normal= pPlane.getNormal();
				height= h;
			}
		}
	}

	return found;
}


// ***************************************************************************
bool			CMiniCol::testMove(const CVector &prec, CVector &cur)
{
	CVector	dir= cur-prec;
	dir.normalize();

	// Angle max.
	float	anglemax= 65;	// 65 degrees.
	anglemax= (float)tan( anglemax*Pi/180);

	// Must not go to near of a wall.
	CVector	test= cur+dir*0.5;
	float	norm= (test-prec).norm();
	norm*=anglemax;
	if(!snapToGround(test, norm, norm))
	{
		cur= prec;
		return false;
	}
	else
	{
		// Must test and snap the current position.
		norm= (cur-prec).norm();
		norm*=anglemax;
		if(!snapToGround(cur, norm, norm))
		{
			cur= prec;
			return false;
		}
	}
	return true;
}


// ***************************************************************************
void			CMiniCol::getFaces(std::vector<CTriangle>	&triresult, const CAABBox &bbox)
{
	triresult.clear();

	// Select.
	_Grid.select(bbox.getMin(),bbox.getMax());

	// For each face, test if it is under pos, then test if height is correct.
	CQuadGrid<CFace>::CIterator	iFace;
	for(iFace= _Grid.begin();iFace!=_Grid.end();iFace++)
	{
		CTriangle	&face= (*iFace).Face;

		if(bbox.intersect(face.V0, face.V1, face.V2))
		{
			triresult.push_back(face);
		}
	}

}


} // NL3D
