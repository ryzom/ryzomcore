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

#include "nel/3d/portal.h"
#include "nel/misc/string_mapper.h"
#include "nel/misc/matrix.h"
#include "nel/misc/stream.h"
#include "nel/misc/polygon.h"
#include "nel/misc/triangle.h"
#include "nel/3d/scene.h"
#include "nel/3d/transform_shape.h"
#include "nel/3d/mesh_instance.h"

using namespace NLMISC;
using namespace std;

namespace NL3D
{

// 0.5 cm of precision
#define PORTALPRECISION 0.005

// ***************************************************************************
CPortal::CPortal()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	_Clusters[0] = _Clusters[1] = NULL;
	_Opened = true;
	_OcclusionModelId = CStringMapper::map("no occlusion");
	_OpenOcclusionModelId = CStringMapper::map("no occlusion");
}

void CPortal::setOcclusionModel(const std::string &occlusionModel)
{
	_OcclusionModelId = CStringMapper::map(occlusionModel);
}
const std::string	&CPortal::getOcclusionModel()
{
	return CStringMapper::unmap(_OcclusionModelId);
}
NLMISC::TStringId CPortal::getOcclusionModelId()
{
	return _OcclusionModelId;
}
void CPortal::setOpenOcclusionModel(const std::string &occlusionModel)
{
	_OpenOcclusionModelId = CStringMapper::map(occlusionModel);
}
const std::string	&CPortal::getOpenOcclusionModel()
{
	return CStringMapper::unmap(_OpenOcclusionModelId);
}
NLMISC::TStringId CPortal::getOpenOcclusionModelId()
{
	return _OpenOcclusionModelId;
}


// ***************************************************************************
bool CPortal::clipPyramid (CVector &observer, std::vector<CPlane> &pyramid)
{
	if (!_Opened || _Poly.size()<3)
		return false;
	// Clip portal with pyramid
	CPolygon p;
	p.Vertices = _Poly;
	p.clip( &pyramid[1], (uint)pyramid.size()-1 );

	// Construct pyramid with clipped portal
	if( p.Vertices.size() > 2 )
	{
		uint i;
		// Found the right orientation
		CVector n = (p.Vertices[1]-p.Vertices[0])^(p.Vertices[2]-p.Vertices[0]);
		if( ((observer-p.Vertices[0])*n) < 0.0f )
		{
			// Invert vertices
			for( i = 0; i < (p.Vertices.size()/2); ++i )
			{
				CVector tmp = p.Vertices[i];
				p.Vertices[i] = p.Vertices[p.Vertices.size()-1-i];
				p.Vertices[p.Vertices.size()-1-i] = tmp;
			}
		}
		// Make pyramid : preserve 0 and 1 plane which are near and far plane
		pyramid.resize (p.Vertices.size()+2);
		for( i = 0; i < (p.Vertices.size()); ++i )
		{
			pyramid[i+2].make( observer, p.Vertices[i], p.Vertices[(i+1)%p.Vertices.size()] );
		}
		return true;
	}

	return false;
}

// ***************************************************************************
bool CPortal::isInFront (CVector &v)
{
	if( _Poly.size()<3 )
		return false;
	CVector v1 = _Poly[1] - _Poly[0];
	CVector v2 = _Poly[2] - _Poly[0];
	CVector n = v1^v2;
	CVector pv = v - _Poly[0];
	return ((n*pv) > 0.0f);
}


// ***************************************************************************
void CPortal::resetClusterLinks()
{
	_Clusters[0] = _Clusters[1] = NULL;
}

// ***************************************************************************
bool CPortal::setCluster(CCluster *cluster)
{
	if( _Clusters[0] == NULL )
	{
		_Clusters[0] = cluster;
		return true;
	}
	if( _Clusters[1] == NULL )
	{
		_Clusters[1] = cluster;
		return true;
	}
	return false;
}

// ***************************************************************************
uint8 CPortal::getNbCluster()
{
	uint8 nRet = 0;
	if( _Clusters[0] != NULL )
		nRet++;
	if( _Clusters[1] != NULL )
		nRet++;
	return nRet;
}

// ***************************************************************************
bool CPortal::setPoly(const std::vector<CVector> &poly)
{
	uint i;

	if( poly.size() < 3 )
		return false;

	// Check if the polygon is a plane
	CPlane p;
	p.make( poly[0], poly[1], poly[2] );
	p.normalize();
	float dist;
	for( i = 0; i < (poly.size()-3); ++i )
	{
		dist = fabsf(p*poly[i+3]);
		if( dist > PORTALPRECISION )
			return false;
	}

	// Check if the polygon is convex
	/// \todo check if the polygon has the good orientation
	/*
	CPlane p2;
	for( i = 0; i < (poly.size()-1); ++i )
	{
		p2.make( poly[i], poly[i+1], poly[i]+p.getNormal() );
		for( j = 0; j < poly.size(); ++j )
		if( (j != i) && (j != i+1) )
		{
			if( p2*poly[j] < 0.0f )
				return false;
		}
	}*/

	// Set the value
	_LocalPoly = poly;
	_Poly = poly;

	return true;
}

// ***************************************************************************
void CPortal::getPoly(std::vector<NLMISC::CVector> &dest) const
{
	dest = _LocalPoly;
}


// ***************************************************************************
void CPortal::serial (NLMISC::IStream& f)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	int version = f.serialVersion (1);

	f.serialCont (_LocalPoly);
	if (f.isReading())
		_Poly = _LocalPoly;
	f.serial (_Name);

	if (version >= 1)
	{
		if (f.isReading())
		{
			std::string occName;
			f.serial(occName);
			if (occName.empty())
				occName = "no occlusion";
			_OcclusionModelId = CStringMapper::map(occName);

			f.serial(occName);
			if (occName.empty())
				occName = "no occlusion";
			_OpenOcclusionModelId = CStringMapper::map(occName);
		}
		else
		{
			std::string occName = CStringMapper::unmap(_OcclusionModelId);
			if (occName == "no occlusion")
				occName = "";
			f.serial(occName);
			occName = CStringMapper::unmap(_OpenOcclusionModelId);
			if (occName == "no occlusion")
				occName = "";
			f.serial(occName);
		}
	}
}

// ***************************************************************************
void CPortal::setWorldMatrix (const CMatrix &WM)
{
	for (uint32 i = 0; i < _LocalPoly.size(); ++i)
		_Poly[i] = WM.mulPoint(_LocalPoly[i]);
}

// ***************************************************************************
bool CPortal::clipRay(const NLMISC::CVector &startWorld, const NLMISC::CVector &endWorld)
{
	if(_Poly.size()<3)
		return false;

	// Avoid precision problem, make local to poly
	const	CVector		&refVert= _Poly[0];
	CVector		start= startWorld - refVert;
	CVector		end= endWorld - refVert;

	// compute the plane of this poly, local to polygon
	CPlane	plane;
	plane.make(CVector::Null, _Poly[1] - refVert, _Poly[2] - refVert);
	CVector	normal = plane.getNormal();

	float	np1 = normal*end;
	float	np2 = np1-normal*start;

	if (np2 == 0.0f)
		return false;

	float	lambda = (plane.d+np1)/np2;

	// Checks the intersection belongs to the segment
	if (lambda < 0 || lambda > 1.0f)
		return false;

	// The intersection on the plane
	CVector	hit = start*lambda+end*(1.0f-lambda);

	// Do convex test on each border
	sint	sign= 0;
	uint	polySize= (uint)_Poly.size();
	for(uint i=0;i<polySize;i++)
	{
		const	CVector	v0= _Poly[i] - refVert;
		const	CVector	v1= _Poly[(i+1)%polySize] - refVert;
		float	d = ((v1-v0)^normal)*(hit-v0);
		if(d<0)
		{
			if(sign==1)
				return false;
			else
				sign=-1;
		}
		else if(d>0)
		{
			if(sign==-1)
				return false;
			else
				sign=1;
		}
		else
			return false;
	}

	// all on same side, ok!
	return true;
}


} // NL3D
