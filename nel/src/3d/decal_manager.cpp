/** \file decal_manager.cpp
 * 
 *
 * $Id$
 */

/* Copyright, 2007 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#include "std3d.h"
#include "nel/3d/decal_manager.h"
#include "nel/3d/scene.h"

#include <iostream>


using namespace std;
using namespace NLMISC;
using namespace NL3D;


// ***************************************************************************
CDecalManager::CDecalManager() :	
		_Touched(true),
		_UsedVertices(0)
{
	_VB.setPreferredMemory( CVertexBuffer::AGPVolatile, true );
	_VB.setVertexFormat( CVertexBuffer::PositionFlag );
}

// ***************************************************************************
CDecalManager::~CDecalManager()
{

}

// ***************************************************************************
void CDecalManager::clearAllDecals()
{
	_Decals.clear();
	_UsedVertices = 0;
	_Touched = true;
}

// ***************************************************************************
void CDecalManager::addDecal(CDecal *decal, const string &texName)
{
	TDecalMap::iterator itEnd = _Decals.begin();
	TDecalMap::iterator it    = _Decals.find(texName);
	if (it!=itEnd)///this key exist, add decal to vector
	{
		it->second.push_back(decal);
	}
	else///Not exist, create new vector
	{
		vector<CDecal*> &vec = _Decals[texName];
		vec.push_back(decal);
	}

	///get number of vertices and compute prerender
	//_UsedVertices += decal->getVertices(_UseVertexProgram).size();
	decal->getVertices(_UseVertexProgram);

	_Touched = true;
}

// ***************************************************************************
void CDecalManager::computeBatch()
{
	//uint vertSize = _VB.getVertexSize();
	//_VB.setNumVertices( _UsedVertices );//tmp

	//CVertexBufferReadWrite vba;
	//_VB.lock(vba);

	//static const size = sizeof(CVector);

	TDecalMap::iterator it    = _Decals.begin();
	TDecalMap::iterator itEnd = _Decals.end();
	
	//uint count=0;
	vector<CVector> accum;
	for(;it!=itEnd;++it)
	{
		vector<CDecal*>::iterator d    = it->second.begin();
		vector<CDecal*>::iterator dEnd = it->second.end();
		for(;d!=dEnd;++d)
		{
			vector<CVector> &vec = (*d)->getVertices(_UseVertexProgram);
			//static const uint length = vec.size();
			//if (length == 0)
			//	continue;
			if(vec.size() == 0)
				continue;


			vector<CVector>::iterator v    = vec.begin();
			vector<CVector>::iterator vEnd = vec.end();
			for(; v!=vEnd; ++v)
			{
				accum.push_back( *v );
			}
			//memcpy( vba.getVertexCoordPointer(count), &vec[0], length*size );
			//count += length;
		}
	}

	//_VB.setNumVertices( count );
	_VB.setNumVertices( accum.size() );
	if (accum.size())
	{
		CVertexBufferReadWrite vba;
		_VB.lock(vba);

		memcpy(vba.getVertexCoordPointer(0), &accum[0], accum.size() * sizeof(CVector));

		// nlassert( count == _UsedVertices );
		nlassert(accum.size() % 3 == 0);

		vba.unlock();
	}
	
	_Touched = false;
}

// ***************************************************************************
///main call for rendering
void CDecalManager::flush(CScene *sc)
{
	if (_Touched)
	{
		computeBatch();
	}

	IDriver	*drv= sc->getRenderTrav().getDriver();
	drv->activeVertexProgram(NULL);///tmp

	drv->activeVertexBuffer(_VB);
	
	CMaterial mat;//tmp
	mat.initUnlit();
	mat.setDoubleSided(true);
	mat.setZBias(-50.0f);

	vector<CDecal*> jean = _Decals["Jean"];//tmp
	

	drv->setupModelMatrix(CMatrix::Identity);///->OK
	drv->renderRawTriangles( mat, 0, _VB.getNumVertices()/3 );

}

// ***************************************************************************
void CDecalManager::renderStatic()
{

}

// ***************************************************************************
void CDecalManager::renderDynamic()
{

}