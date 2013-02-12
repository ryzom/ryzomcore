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

#include "stdpacs.h"
#include "nel/pacs/surface_quad.h"

#include "nel/misc/file.h"

using namespace NLMISC;
using namespace std;

NLPACS::CSurfaceQuadTree::CSurfaceQuadTree()
{
	_Root = NULL;
	_MaxThickness = FLT_MAX;
	_MaxLevel = 1;
	_BBox.setCenter(CVector::Null);
	_BBox.setSize(CVector::Null);
}

NLPACS::CSurfaceQuadTree::CSurfaceQuadTree(const NLPACS::CSurfaceQuadTree &quad)
{
	*this = quad;
}

NLPACS::CSurfaceQuadTree	&NLPACS::CSurfaceQuadTree::operator = (const NLPACS::CSurfaceQuadTree &quad)
{
	if (&quad == this)
		return *this;

	_MaxThickness = quad._MaxThickness;
	_MaxLevel = quad._MaxLevel;
	_BBox = quad._BBox;

	_Root = NULL;
	if (quad._Root != NULL)
	{
		if (quad._Root->isLeaf())
		{
			CQuadLeaf	*newLeaf = new CQuadLeaf();
			*newLeaf = *((CQuadLeaf *)(quad._Root));
			_Root = newLeaf;
		}
		else
		{
			CQuadBranch	*newBranch = new CQuadBranch();
			*newBranch = *((CQuadBranch *)(quad._Root));
			_Root = newBranch;
		}
	}

	return *this;
}

void	NLPACS::CSurfaceQuadTree::clear()
{
	delete _Root;
	_Root = NULL;
}

void	NLPACS::CSurfaceQuadTree::init(float maxThickness, uint maxLevel, const CVector &center, float halfSize)
{
	nlassert(maxLevel > 0);
	clear();
	_MaxThickness = maxThickness;
	_MaxLevel = uint8(maxLevel);
	_BBox.setCenter(center);
	_BBox.setHalfSize(CVector(halfSize, halfSize, 10000.0f));
}

void	NLPACS::CSurfaceQuadTree::addVertex(const CVector &v)
{
	if (!_BBox.include(v))
		return;

	if (_Root == NULL)
	{
		if (_MaxLevel == 1)
		{
			_Root = new CQuadLeaf(_MaxLevel);
		}
		else
		{
			_Root = new CQuadBranch(_MaxLevel);
		}

		_Root->_MaxThickness = _MaxThickness;
		_Root->_HalfSize = _BBox.getHalfSize().x;
		_Root->_MinHeight = FLT_MAX;
		_Root->_MaxHeight = -FLT_MAX;
		_Root->_XCenter = _BBox.getCenter().x;
		_Root->_YCenter = _BBox.getCenter().y;
	}

	_Root->addVertex(v);
}

void	NLPACS::CSurfaceQuadTree::compile()
{
	if (_Root != NULL &&
		!_Root->isLeaf() &&
		_Root->getMaxHeight()-_Root->getMinHeight() <= _MaxThickness)
	{
		CQuadLeaf	*leaf = new CQuadLeaf();
		*((IQuadNode *)leaf) = *_Root;
		delete _Root;
		_Root = leaf;
	}
	else if (_Root != NULL &&
			 !_Root->isLeaf())
	{
		((CQuadBranch *)_Root)->reduceChildren();
	}
}


NLPACS::CQuadBranch::CQuadBranch(const NLPACS::CQuadBranch &branch) : NLPACS::IQuadNode(branch)
{
	*this = branch;
}

NLPACS::CQuadBranch	&NLPACS::CQuadBranch::operator = (const NLPACS::CQuadBranch &branch)
{
	IQuadNode::operator=(branch);

	uint	child;
	for (child=0; child<4; ++child)
	{
		_Children[child] = NULL;
		if (branch._Children[child] != NULL)
		{
			if (branch._Children[child]->isLeaf())
			{
				CQuadLeaf	*newLeaf = new CQuadLeaf();
				*newLeaf = *((CQuadLeaf *)(branch._Children[child]));
				_Children[child] = newLeaf;
			}
			else
			{
				CQuadBranch	*newBranch = new CQuadBranch();
				*newBranch = *((CQuadBranch *)(branch._Children[child]));
				_Children[child] = newBranch;
			}
		}
	}
	return *this;
}

void	NLPACS::CQuadBranch::reduceChildren()
{
	uint	i;

	for (i=0; i<4; ++i)
	{
		if (_Children[i] != NULL &&
			!_Children[i]->isLeaf() &&
			_Children[i]->getMaxHeight()-_Children[i]->getMinHeight() <= _MaxThickness)
		{
			CQuadLeaf	*leaf = new CQuadLeaf();
			*((IQuadNode *)leaf) = *_Children[i];
			delete _Children[i];
			_Children[i] = leaf;
		}
		else if (_Children[i] != NULL &&
				 !_Children[i]->isLeaf())
		{
			((CQuadBranch *)_Children[i])->reduceChildren();
		}
	}
}

void	NLPACS::CQuadBranch::addVertex(const CVector &v)
{
	IQuadNode::addVertex(v);
	uint	child;
	if (v.x > _XCenter)
		child = (v.y > _YCenter) ? 2 : 1;
	else
		child = (v.y > _YCenter) ? 3 : 0;

	if (_Children[child] == NULL)
	{
		if (_Level == 2)
		{
			_Children[child] = new CQuadLeaf(_Level-1);
		}
		else
		{
			_Children[child] = new CQuadBranch(_Level-1);
		}

		_Children[child]->_MaxThickness = _MaxThickness;
		_Children[child]->_HalfSize = _HalfSize/2.0f;
		_Children[child]->_MinHeight = FLT_MAX;
		_Children[child]->_MaxHeight = -FLT_MAX;
		_Children[child]->_XCenter = _XCenter+_Children[child]->_HalfSize*((child == 1 || child == 2) ? 1.0f : -1.0f);
		_Children[child]->_YCenter = _YCenter+_Children[child]->_HalfSize*((child == 2 || child == 3) ? 1.0f : -1.0f);
	}

	_Children[child]->addVertex(v);
}

bool	NLPACS::CQuadBranch::check() const
{
	if (!IQuadNode::check())
		return false;

	uint	child;
	for (child=0; child<4; ++child)
		if (_Children[child] != NULL && !_Children[child]->check())
			return false;
	return true;
}


/*
 * Serialization methods...
 */


void	NLPACS::CQuadBranch::serial(NLMISC::IStream &f)
{
	IQuadNode::serial(f);

	uint	child;
	for (child=0; child<4; ++child)
	{
		uint8	childType = 0;

		if (f.isReading())
		{
			CQuadLeaf	*leaf;
			CQuadBranch	*branch;
			f.serial(childType);
			switch (childType)
			{
			case NoChild:
				_Children[child] = NULL;
				break;
			case LeafChild:
				leaf = new CQuadLeaf();
				_Children[child] = leaf;
				leaf->serial(f);
				break;
			case BranchChild:
				branch = new CQuadBranch();;
				_Children[child] = branch;
				branch->serial(f);
				break;
			default:
				nlerror("While serializing (read) CQuadBranch: unknown child type");
				break;
			}
		}
		else
		{
			if (_Children[child] == NULL)
			{
				childType = NoChild;
				f.serial(childType);
			}
			else
			{
				childType = uint8(_Children[child]->isLeaf() ? LeafChild : BranchChild);
				f.serial(childType);
				_Children[child]->serial(f);
			}
		}
	}
}

bool	NLPACS::CSurfaceQuadTree::check() const
{
	if (_Root != NULL)
		return _Root->check();
	return true;
}

const NLPACS::CQuadLeaf	*NLPACS::CSurfaceQuadTree::getLeaf(const CVector &v) const
{
	CVector	pos = CVector(v.x, v.y, 0.0f);
	if (_Root == NULL || !_BBox.include(pos))
		return NULL;

	const IQuadNode	*node = _Root;

	while (node != NULL && !node->isLeaf())
	{
		nlassert(node->getBBox().include(pos));
		uint	child;

		if (pos.x > node->_XCenter)
			child = ((pos.y > node->_YCenter) ? 2 : 1);
		else
			child = ((pos.y > node->_YCenter) ? 3 : 0);

		node = node->getChild(child);
	}

	return (const CQuadLeaf *)node;
}


void	NLPACS::CSurfaceQuadTree::serial(NLMISC::IStream &f)
{
	/*
	Version 0:
		- base version.
	*/
	(void)f.serialVersion(0);

	uint8	childType = 0;
	f.serial(_MaxThickness);
	f.serial(_MaxLevel);
	f.serial(_BBox);
	if (f.isReading())
	{
		CQuadLeaf	*leaf;
		CQuadBranch	*branch;

		f.serial(childType);
		switch (childType)
		{
		case CQuadBranch::NoChild:
			_Root = NULL;
			break;
		case CQuadBranch::LeafChild:
			leaf = new CQuadLeaf();
			_Root = leaf;
			leaf->serial(f);
			break;
		case CQuadBranch::BranchChild:
			branch = new CQuadBranch();
			_Root = branch;
			branch->serial(f);
			break;
		default:
			nlerror("While serializing (read) CQuadBranch: unknown child type");
			break;
		}
	}
	else
	{
		if (_Root == NULL)
		{
			childType = CQuadBranch::NoChild;
			f.serial(childType);
		}
		else
		{
			childType = uint8(_Root->isLeaf() ? CQuadBranch::LeafChild : CQuadBranch::BranchChild);
			f.serial(childType);
			_Root->serial(f);
		}
	}
}

//
float	NLPACS::CSurfaceQuadTree::getInterpZ(const CVector &v) const
{
	// first get final leaf for position
	CVector	pos = CVector(v.x, v.y, 0.0f);
	if (_Root == NULL || !_BBox.include(pos))
		return v.z;	// return unmodified z

	const IQuadNode				*node = _Root;
	vector<uint>				children;
	vector<const IQuadNode*>	nodes;

	while (node != NULL && !node->isLeaf())
	{
		nodes.push_back(node);

		nlassert(node->getBBox().include(pos));
		uint	child;

		if (pos.x > node->_XCenter)
			child = ((pos.y > node->_YCenter) ? 2 : 1);
		else
			child = ((pos.y > node->_YCenter) ? 3 : 0);

		children.push_back(child);

		node = node->getChild(child);
	}

	if (node == NULL)
		return v.z;	// return unmodified z

	nodes.push_back(node);

	vector<const CQuadLeaf*>	leaves;
	vector<const IQuadNode*>	explore;

	leaves.push_back((const CQuadLeaf*)node);

	// for each side of the leaf, find neighbor leaves
	uint	side;
	for (side=0; side<4; ++side)
	{
		static const sint	ct[4][4] = { {-1, 1, 3,-1}, {-1,-1, 2, 0}, { 1,-1,-1, 3}, { 0, 2,-1,-1} };	// child table
		static const sint	nt[4][4] = { { 3, 1, 3, 1}, { 2, 0, 2, 0}, { 1, 3, 1, 3}, { 0, 2, 0, 2} };	// neighbor table

		sint	nlev = (sint)nodes.size()-1;
		sint	child = -1;

		while (nlev > 0)
		{
			child = ct[children[nlev-1]][side];

			if (child >= 0)
				break;

			--nlev;
		}

		// neighbor not found in root, leave
		if (nlev == 0)
			continue;

		// get father
		node = nodes[nlev-1];

		while (nlev < (sint)nodes.size() && node!=NULL && !node->isLeaf())
		{
			child = nt[children[nlev-1]][side];
			node = node->getChild(child);

			++nlev;
		}

		if (node == NULL)
			continue;

		if (node->isLeaf())
		{
			leaves.push_back((const CQuadLeaf*)node);
		}
		else
		{
			explore.push_back(node);

			while (!explore.empty())
			{
				node = explore.back();
				explore.pop_back();

				if (node == NULL)
					continue;

				if (node->isLeaf())
					leaves.push_back((const CQuadLeaf*)node);
				else
				{
					explore.push_back(node->getChild((side+2)&3));
					explore.push_back(node->getChild((side+3)&3));
				}
			}
		}
	}

	uint			i;
	float			di, wi;
	float			sum = 0.0;
	float			interpZ = 0.0;

	for (i=0; i<leaves.size(); ++i)
	{
		di = (float)sqrt(sqr(v.x-leaves[i]->_XCenter)+sqr(v.y-leaves[i]->_YCenter))*(float)pow(1.5, leaves[i]->_Level);
		wi = 1.0f/di;
		sum += wi;
		interpZ += (leaves[i]->getMinHeight()+leaves[i]->getMaxHeight())*0.5f*wi;
	}

	return interpZ / sum;
}
