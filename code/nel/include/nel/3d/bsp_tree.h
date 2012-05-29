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


#ifndef	NL_BSP_TREE_H
#define	NL_BSP_TREE_H

#include "nel/misc/debug.h"
#include "nel/misc/vector.h"
#include "nel/misc/plane.h"
#include "nel/misc/matrix.h"
#include "nel/misc/triangle.h"
#include <list>
#include <vector>


namespace	NL3D
{



/**
  * class:	CBSPTree.
  *
  *
  * A template CBSPTree.
  *
  */
template<class T>	class	CBSPTree
{

public:

	/// Default constructor, use axes XZ
	CBSPTree();

	/// dtor.
	~CBSPTree();

public:

	void insert( NLMISC::CTriangle &tri, T &value );
	sint32 select( CVector &v1, CVector &v2 );
	T getSelection( sint32 i );
	sint32 getNbNode();

private:

	class CBSPNode;
	std::vector<CBSPNode*> _Selection;

private:

	class CBSPNode
	{
		CBSPNode *pBack, *pFront;
		CPlane p;

	public:

		T Value;

	public:

		CBSPNode( NLMISC::CTriangle &tri, T &val ) : Value(val), pBack(NULL), pFront(NULL)
		{
			p.make( tri.V0, tri.V1, tri.V2 );
			p.normalize();
		}

		~CBSPNode()
		{
			if( pBack != NULL )
				delete pBack;
			if( pFront != NULL )
				delete pFront;
		}

		void insert( NLMISC::CTriangle &tri, T &val )
		{
			float	f[3];
			CBSPNode *pCurrent = this;

			while( true )
			{
				f[0] = pCurrent->p*tri.V0;
				f[1] = pCurrent->p*tri.V1,
				f[2] = pCurrent->p*tri.V2;
				if( fabs( f[0] ) < 0.00001 ) f[0] = 0.0f;
				if( fabs( f[1] ) < 0.00001 ) f[1] = 0.0f;
				if( fabs( f[2] ) < 0.00001 ) f[2] = 0.0f;
				if( ( f[0] >= 0.0f ) && ( f[1] >= 0.0f ) && ( f[2] >= 0.0f ) )
				{	// All front
					if( pCurrent->pFront == NULL )
					{
						pCurrent->pFront = new CBSPNode( tri, val );
						return;
					}
					else
					{
						pCurrent = pCurrent->pFront;
					}
				}
				else
				if( ( f[0] <= 0.0f ) && ( f[1] <= 0.0f ) && ( f[2] <= 0.0f ) )
				{	// All back
					if( pCurrent->pBack == NULL )
					{
						pCurrent->pBack = new CBSPNode( tri, val );
						return;
					}
					else
					{
						pCurrent = pCurrent->pBack;
					}
				}
				else
				{
					if( pCurrent->pFront == NULL )
					{
						pCurrent->pFront = new CBSPNode( tri, val );
					}
					else
					{
						pCurrent->pFront->insert( tri, val );
					}
					if( pCurrent->pBack == NULL )
					{
						pCurrent->pBack = new CBSPNode( tri, val );
					}
					else
					{
						pCurrent->pBack->insert( tri, val );
					}
					return;
				}
			}
		}


		sint32 getNbNode()
		{
			sint32 nBack = 0, nFront= 0;
			if( pBack != NULL )
				nBack = pBack->getNbNode();
			if( pFront != NULL )
				nFront = pFront->getNbNode();
			return 1+nBack+nFront;
		}


		void select( std::vector<CBSPNode*> &sel, CVector &v1, CVector &v2 )
		{
			float	f[2];
			CBSPNode *pCurrent = this;

			while( true )
			{
				f[0] = pCurrent->p*v1;
				f[1] = pCurrent->p*v2;
				if( fabs( f[0] ) < 0.00001 ) f[0] = 0.0;
				if( fabs( f[1] ) < 0.00001 ) f[1] = 0.0;
				if( ( f[0] >= 0.0 ) && ( f[1] >= 0.0 ) )
				{	// All front
					if( pCurrent->pFront == NULL )
					{
						return;
					}
					else
					{
						pCurrent = pCurrent->pFront;
					}
				}
				else
				if( ( f[0] <= 0.0 ) && ( f[1] <= 0.0 ) )
				{	// All back
					if( pCurrent->pBack == NULL )
					{
						return;
					}
					else
					{
						pCurrent = pCurrent->pBack;
					}
				}
				else
				{
					if( sel.size() == sel.capacity() )
						sel.reserve( sel.size() + 64 );
					sel.push_back( this );
					if( pCurrent->pFront == NULL )
					{
					}
					else
					{
						//CVector newV1 = v1;
						//CVector newV2 = v2;
						//pCurrent->p.clipSegmentFront( newV1, newV2 );
						//pCurrent->pFront->select( sel, newV1, newV2 );
						pCurrent->pFront->select( sel, v1, v2 );
					}
					if( pCurrent->pBack == NULL )
					{
					}
					else
					{
						//CVector newV1 = v1;
						//CVector newV2 = v2;
						//pCurrent->p.clipSegmentBack( newV1, newV2 );
						//pCurrent->pBack->select( sel, newV1, newV2 );
						pCurrent->pBack->select( sel, v1, v2 );
					}
					return;
				}
			}
		}
	};

private:

	CBSPNode *_Root;

};

// ============================================================================================
// ============================================================================================
// Template CBSPTree implementation. Construction/Destruction.
// ============================================================================================
// ============================================================================================

template<class T> CBSPTree<T>::CBSPTree() : _Root(NULL)
{
	_Selection.reserve( 64 );
}

template<class T> CBSPTree<T>::~CBSPTree()
{
	if( _Root != NULL )
		delete _Root;
}

// ============================================================================================
// ============================================================================================
// Template CBSPTree implementation.
// ============================================================================================
// ============================================================================================

template<class T> void CBSPTree<T>::insert( NLMISC::CTriangle &tri, T &val )
{
	if( _Root == NULL )
		_Root = new CBSPNode( tri, val );
	else
		_Root->insert( tri, val );
}

template<class T> sint32 CBSPTree<T>::select( CVector &v1, CVector &v2 )
{
	_Selection.clear();
	if( _Root != NULL )
	{
		_Root->select( _Selection, v1, v2 );
		return _Selection.size();
	}
	else
		return 0;
}

template<class T> T CBSPTree<T>::getSelection( sint32 i )
{
	return _Selection[i]->Value;
}

template<class T> sint32 CBSPTree<T>::getNbNode()
{
	return _Root->getNbNode();
}

}

#endif // NL_BSP_TREE_H
