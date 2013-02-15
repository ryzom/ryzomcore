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

#ifndef NL_CUBE_GRID_H
#define NL_CUBE_GRID_H

#include "nel/misc/types_nl.h"
#include "nel/misc/triangle.h"
#include "nel/misc/plane.h"
#include "nel/3d/quad_grid.h"
#include "nel/3d/static_quad_grid.h"


namespace NL3D {


// ***************************************************************************
/**
 * Class used to classify elements in space around a point (good for pointlight raytracing)
 *	Elements are copied at insertion, and are duplicated at compilation along the grids.
 *	Therefore TCell should be small (ie a pointer). This constraint help because memory is much more
 *	reduced when compile is called.
 *
 * \author Matthieu Besson
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
template <class TCell>
class CCubeGrid
{
public:

	CCubeGrid ();
	~CCubeGrid ();

	/** Create the cubeGrid, specifying center of this one, and number of Cells per side.
	 *
	 */
	void		create (const CVector &center, int nSize);
	void		insert (const NLMISC::CTriangle &tri, const TCell &cell);
	/// compile the container, storing in CStaticQuadGrid.
	void		compile();

	// Selection. CubeGrid must be compilated. NB: it clear old selection.
	void		select (const NLMISC::CVector &v);
	TCell		getSel ();
	void		nextSel ();
	bool		isEndSel ();

// ************************
private:

	NLMISC::CVector		_Center;

	enum gridPos { kUp = 0, kDown, kLeft, kRight, kFront, kBack };
	// This is temp used at element insertion. freed at compile() time
	NL3D::CQuadGrid<TCell> _Grids[6];
	// For fast selection, and minimum memory overhead.
	NL3D::CStaticQuadGrid<TCell> _StaticGrids[6];


	bool					_Compiled;

	// Selection.
	const TCell				*_Selection;
	uint					_CurSel;
	uint					_NumSels;

private:

	void project	(const NLMISC::CTriangle &tri, NLMISC::CPlane pyr[4], NLMISC::CPlane &gridPlane,
					sint32 nGridNb, const TCell &cell);

};


// ***************************************************************************
// ***************************************************************************
// Implementation
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
template<class TCell>
CCubeGrid<TCell>::CCubeGrid ()
{
	NLMISC::CMatrix	tmp;
	NLMISC::CVector	I, J, K;

	// grids[kUp].changeBase(  );
	I = NLMISC::CVector(  1,  0,  0 );
	J = NLMISC::CVector(  0, -1,  0 );
	K = NLMISC::CVector(  0,  0, -1 );
	tmp.identity(); tmp.setRot( I, J, K, true );
	_Grids[kDown].changeBase( tmp );

	I = NLMISC::CVector(  0,  0,  1 );
	J = NLMISC::CVector(  0,  1,  0 );
	K = NLMISC::CVector( -1,  0,  0 );
	tmp.identity(); tmp.setRot( I, J, K, true);
	_Grids[kLeft].changeBase( tmp );

	I = NLMISC::CVector(  0,  0, -1 );
	J = NLMISC::CVector(  0,  1,  0 );
	K = NLMISC::CVector(  1,  0,  0 );
	tmp.identity(); tmp.setRot( I, J, K, true);
	_Grids[kRight].changeBase( tmp );

	I = NLMISC::CVector(  1,  0,  0 );
	J = NLMISC::CVector(  0,  0,  1 );
	K = NLMISC::CVector(  0, -1,  0 );
	tmp.identity(); tmp.setRot( I, J, K, true);
	_Grids[kFront].changeBase( tmp );

	I = NLMISC::CVector(  1,  0,  0 );
	J = NLMISC::CVector(  0,  0, -1 );
	K = NLMISC::CVector(  0,  1,  0 );
	tmp.identity(); tmp.setRot( I, J, K, true);
	_Grids[kBack].changeBase( tmp );


	_Compiled= false;
}

// ***************************************************************************
template<class TCell>
CCubeGrid<TCell>::~CCubeGrid ()
{
}

// ***************************************************************************
template<class TCell>
void CCubeGrid<TCell>::create (const CVector &center, int nSize)
{
	nlassert(!_Compiled);
	_Center= center;

	_Grids[kUp].create		( nSize, 1.0f / ((float)nSize) );
	_Grids[kDown].create	( nSize, 1.0f / ((float)nSize) );
	_Grids[kLeft].create	( nSize, 1.0f / ((float)nSize) );
	_Grids[kRight].create	( nSize, 1.0f / ((float)nSize) );
	_Grids[kFront].create	( nSize, 1.0f / ((float)nSize) );
	_Grids[kBack].create	( nSize, 1.0f / ((float)nSize) );
}

// ***************************************************************************
template<class TCell>
void CCubeGrid<TCell>::insert (const NLMISC::CTriangle &triIn, const TCell &cell)
{
	nlassert(!_Compiled);
	// Center triangle on _Center.
	NLMISC::CTriangle tri= triIn;
	tri.V0-= _Center;
	tri.V1-= _Center;
	tri.V2-= _Center;

	NLMISC::CPlane p[4], gp;
	// Construct clip pyramid for grid : UP
	p[0].make( NLMISC::CVector(0,0,0), NLMISC::CVector( -1,-1,+1 ), NLMISC::CVector( +1,-1,+1 ) );
	p[1].make( NLMISC::CVector(0,0,0), NLMISC::CVector( +1,-1,+1 ), NLMISC::CVector( +1,+1,+1 ) );
	p[2].make( NLMISC::CVector(0,0,0), NLMISC::CVector( +1,+1,+1 ), NLMISC::CVector( -1,+1,+1 ) );
	p[3].make( NLMISC::CVector(0,0,0), NLMISC::CVector( -1,+1,+1 ), NLMISC::CVector( -1,-1,+1 ) );
	gp.make( NLMISC::CVector(0,0,1), NLMISC::CVector(0,0,0.5) );
	project( tri, p, gp, kUp, cell );
	// Construct clip pyramid for grid : DOWN
	p[0].make( NLMISC::CVector(0,0,0), NLMISC::CVector( +1,-1,-1 ), NLMISC::CVector( -1,-1,-1 ) );
	p[1].make( NLMISC::CVector(0,0,0), NLMISC::CVector( -1,-1,-1 ), NLMISC::CVector( -1,+1,-1 ) );
	p[2].make( NLMISC::CVector(0,0,0), NLMISC::CVector( -1,+1,-1 ), NLMISC::CVector( +1,+1,-1 ) );
	p[3].make( NLMISC::CVector(0,0,0), NLMISC::CVector( +1,+1,-1 ), NLMISC::CVector( +1,-1,-1 ) );
	gp.make( NLMISC::CVector(0,0,-1), NLMISC::CVector(0,0,-0.5) );
	project( tri, p, gp, kDown, cell );
	// Construct clip pyramid for grid : LEFT
	p[0].make( NLMISC::CVector(0,0,0), NLMISC::CVector( -1,-1,-1 ), NLMISC::CVector( -1,-1,+1 ) );
	p[1].make( NLMISC::CVector(0,0,0), NLMISC::CVector( -1,-1,+1 ), NLMISC::CVector( -1,+1,+1 ) );
	p[2].make( NLMISC::CVector(0,0,0), NLMISC::CVector( -1,+1,+1 ), NLMISC::CVector( -1,+1,-1 ) );
	p[3].make( NLMISC::CVector(0,0,0), NLMISC::CVector( -1,+1,-1 ), NLMISC::CVector( -1,-1,-1 ) );
	gp.make( NLMISC::CVector(-1,0,0), NLMISC::CVector(-0.5,0,0) );
	project( tri, p, gp, kLeft, cell );
	// Construct clip pyramid for grid : RIGHT
	p[0].make( NLMISC::CVector(0,0,0), NLMISC::CVector( +1,-1,+1 ), NLMISC::CVector( +1,-1,-1 ) );
	p[1].make( NLMISC::CVector(0,0,0), NLMISC::CVector( +1,-1,-1 ), NLMISC::CVector( +1,+1,-1 ) );
	p[2].make( NLMISC::CVector(0,0,0), NLMISC::CVector( +1,+1,-1 ), NLMISC::CVector( +1,+1,+1 ) );
	p[3].make( NLMISC::CVector(0,0,0), NLMISC::CVector( +1,+1,+1 ), NLMISC::CVector( +1,-1,+1 ) );
	gp.make( NLMISC::CVector(1,0,0), NLMISC::CVector(0.5,0,0) );
	project( tri, p, gp, kRight, cell );
	// Construct clip pyramid for grid : FRONT
	p[0].make( NLMISC::CVector(0,0,0), NLMISC::CVector( -1,-1,-1 ), NLMISC::CVector( +1,-1,-1 ) );
	p[1].make( NLMISC::CVector(0,0,0), NLMISC::CVector( +1,-1,-1 ), NLMISC::CVector( +1,-1,+1 ) );
	p[2].make( NLMISC::CVector(0,0,0), NLMISC::CVector( +1,-1,+1 ), NLMISC::CVector( -1,-1,+1 ) );
	p[3].make( NLMISC::CVector(0,0,0), NLMISC::CVector( -1,-1,+1 ), NLMISC::CVector( -1,-1,-1 ) );
	gp.make( NLMISC::CVector(0,-1,0), NLMISC::CVector(0,-0.5,0) );
	project( tri, p, gp, kFront, cell );
	// Construct clip pyramid for grid : BACK
	p[0].make( NLMISC::CVector(0,0,0), NLMISC::CVector( +1,+1,+1 ), NLMISC::CVector( +1,+1,-1 ) );
	p[1].make( NLMISC::CVector(0,0,0), NLMISC::CVector( +1,+1,-1 ), NLMISC::CVector( -1,+1,-1 ) );
	p[2].make( NLMISC::CVector(0,0,0), NLMISC::CVector( -1,+1,-1 ), NLMISC::CVector( -1,+1,+1 ) );
	p[3].make( NLMISC::CVector(0,0,0), NLMISC::CVector( -1,+1,+1 ), NLMISC::CVector( +1,+1,+1 ) );
	gp.make( NLMISC::CVector(0,1,0), NLMISC::CVector(0,0.5,0) );
	project( tri, p, gp, kBack, cell );
}


// ***************************************************************************
template<class TCell>
void CCubeGrid<TCell>::compile()
{
	nlassert(!_Compiled);
	// For all Grids.
	uint	i;
	for(i=0; i<6; i++)
	{
		// build the _StaticGrid
		_StaticGrids[i].build(_Grids[i]);
		// And reset the grid. contReset is necessary to clean the CBlockMemory.
		NLMISC::contReset(_Grids[i]);
	}

	// done
	_Compiled= true;

	// Clear the Selection
	_Selection= NULL;
	_CurSel= 0;
	_NumSels= 0;
}

// ***************************************************************************
template<class TCell>
void CCubeGrid<TCell>::select (const NLMISC::CVector &vIn)
{
	nlassert(_Compiled);
	// Center triangle on _Center.
	NLMISC::CVector v= vIn-_Center;


	sint	nSelGrid= -1;
	NLMISC::CPlane gp;
	// Get the plane
	if( ( -v.z <= v.x ) && ( v.x <= v.z ) &&
		( -v.z <= v.y ) && ( v.y <= v.z ) &&
		( 0.0f <= v.z ) )
	{
		nSelGrid = kUp;
		gp.make( NLMISC::CVector(0,0,1), NLMISC::CVector(0,0,0.5) );
	}
	if( ( v.z <= v.x ) && ( v.x <= -v.z ) &&
		( v.z <= v.y ) && ( v.y <= -v.z ) &&
		( v.z <= 0.0f ) )
	{
		nSelGrid = kDown;
		gp.make( NLMISC::CVector(0,0,-1), NLMISC::CVector(0,0,-0.5) );
	}
	if( ( v.x <= 0.0f ) &&
		( v.x <= v.y ) && ( v.y <= -v.x ) &&
		( v.x <= v.z ) && ( v.z <= -v.x ) )
	{
		nSelGrid = kLeft;
		gp.make( NLMISC::CVector(-1,0,0), NLMISC::CVector(-0.5,0,0) );
	}
	if( ( 0.0f <= v.x ) &&
		( -v.x <= v.y ) && ( v.y <= v.x ) &&
		( -v.x <= v.z ) && ( v.z <= v.x ) )
	{
		nSelGrid = kRight;
		gp.make( NLMISC::CVector(1,0,0), NLMISC::CVector(0.5,0,0) );
	}
	if( ( v.y <= v.x ) && ( v.x <= -v.y ) &&
		( v.y <= 0.0f ) &&
		( v.y <= v.z ) && ( v.z <= -v.y ) )
	{
		nSelGrid = kFront;
		gp.make( NLMISC::CVector(0,-1,0), NLMISC::CVector(0,-0.5,0) );
	}
	if( ( -v.y <= v.x ) && ( v.x <= v.y ) &&
		( 0.0f <= v.y ) &&
		( -v.y <= v.z ) && ( v.z <= v.y ) )
	{
		nSelGrid = kBack;
		gp.make( NLMISC::CVector(0,1,0), NLMISC::CVector(0,0.5,0) );
	}
	nlassert(nSelGrid!=-1);
	NLMISC::CVector newV = gp.intersect( NLMISC::CVector(0,0,0), v );
	_Selection= _StaticGrids[nSelGrid].select(newV, _NumSels);
	_CurSel = 0;
}

// ***************************************************************************
template<class TCell>
TCell CCubeGrid<TCell>::getSel ()
{
	nlassert(_Compiled);
	return _Selection[_CurSel];
}

// ***************************************************************************
template<class TCell>
void CCubeGrid<TCell>::nextSel ()
{
	nlassert(_Compiled);
	++_CurSel;
}

// ***************************************************************************
template<class TCell>
bool CCubeGrid<TCell>::isEndSel ()
{
	nlassert(_Compiled);
	return (_CurSel == _NumSels);
}

// ***************************************************************************
template<class TCell>
void CCubeGrid<TCell>::project (const NLMISC::CTriangle &tri, NLMISC::CPlane pyr[4], NLMISC::CPlane &gridPlane, sint32 nGridNb, const TCell &cell)
{
	NLMISC::CVector vIn[7], vOut[7];
	sint32 i, nOut;
	vIn[0] = tri.V0; vIn[1] = tri.V1; vIn[2] = tri.V2;
	nOut = pyr[0].clipPolygonFront( vIn, vOut, 3 );
	if( nOut == 0 ) return;
	for( i = 0; i < nOut; ++i ) vIn[i] = vOut[i];
	nOut = pyr[1].clipPolygonFront( vIn, vOut, nOut );
	if( nOut == 0 ) return;
	for( i = 0; i < nOut; ++i ) vIn[i] = vOut[i];
	nOut = pyr[2].clipPolygonFront( vIn, vOut, nOut );
	if( nOut == 0 ) return;
	for( i = 0; i < nOut; ++i ) vIn[i] = vOut[i];
	nOut = pyr[3].clipPolygonFront( vIn, vOut, nOut );
	if( nOut >= 3 )
	{
		NLMISC::CVector vMin(1, 1, 1), vMax(-1, -1, -1);
		for( i = 0; i < nOut; ++i )
		{
			vOut[i] = gridPlane.intersect( NLMISC::CVector(0, 0, 0), vOut[i] );
			if( vMin.x > vOut[i].x ) vMin.x = vOut[i].x;
			if( vMin.y > vOut[i].y ) vMin.y = vOut[i].y;
			if( vMin.z > vOut[i].z ) vMin.z = vOut[i].z;
			if( vMax.x < vOut[i].x ) vMax.x = vOut[i].x;
			if( vMax.y < vOut[i].y ) vMax.y = vOut[i].y;
			if( vMax.z < vOut[i].z ) vMax.z = vOut[i].z;
		}
		// Create the bbox
		_Grids[nGridNb].insert( vMin, vMax, cell );
	}
}



} // NL3D


#endif // NL_CUBE_GRID_H

/* End of cube_grid.h */
