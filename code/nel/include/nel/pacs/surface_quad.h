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

#ifndef NL_SURFACE_QUAD_H
#define NL_SURFACE_QUAD_H

#include "nel/misc/types_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/vector.h"
#include "nel/misc/aabbox.h"


namespace NLPACS
{

class IQuadNode
{
protected:
	friend class CSurfaceQuadTree;
	friend class CQuadLeaf;
	friend class CQuadBranch;
	float					_MinHeight,
							_MaxHeight;
	float					_MaxThickness;
	uint8					_Level;
	float					_XCenter, _YCenter, _HalfSize;
	IQuadNode(uint8 level = 0) : _Level(level) {}
public:
	virtual ~IQuadNode()	{}
	virtual bool			isLeaf() const = 0;
	virtual float			getMinHeight() const { return _MinHeight; }
	virtual float			getMaxHeight() const { return _MaxHeight; }
	virtual uint8			getLevel() const { return _Level; }
	virtual const IQuadNode	*getChild(uint child) const = 0;
	virtual void			addVertex(const NLMISC::CVector &v)
	{
		if (v.z < _MinHeight)	_MinHeight = v.z;
		if (v.z > _MaxHeight)	_MaxHeight = v.z;
	}
	virtual NLMISC::CAABBox	getBBox() const
	{
		NLMISC::CAABBox	bbox;
		bbox.setCenter(NLMISC::CVector(_XCenter, _YCenter, 0.0f));
		bbox.setHalfSize(NLMISC::CVector(_HalfSize, _HalfSize, 10000.0f));
		return bbox;
	}

	virtual void			translate(const NLMISC::CVector &translation)
	{
		_XCenter += translation.x;
		_YCenter += translation.y;
		_MinHeight += translation.z;
		_MaxHeight += translation.z;
	}

	virtual bool			check() const	{ return (_MaxHeight >= _MinHeight); }

	virtual void			serial(NLMISC::IStream &f)
	{
		f.serial(_MinHeight, _MaxHeight, _Level, _MaxThickness);
		f.serial(_XCenter, _YCenter, _HalfSize);
	}
};

//
class CQuadLeaf : public IQuadNode
{
public:
	CQuadLeaf(uint8 level = 0) : IQuadNode(level) {}
	bool					isLeaf() const { return true; }
	const IQuadNode			*getChild(uint child) const { nlerror("Can't access child %d on the leaf!", child); return NULL; }
	bool			check() const
	{
		if (!IQuadNode::check())
			return false;
		return (_Level == 1 || _MaxHeight-_MinHeight <= _MaxThickness);
	}

	void					serial(NLMISC::IStream &f)	{ IQuadNode::serial(f); }
};

//
class CQuadBranch : public IQuadNode
{
public:
	enum
	{
		NoChild = 0,
		LeafChild,
		BranchChild
	};

protected:
	IQuadNode				*_Children[4];

protected:
	friend class CSurfaceQuadTree;

	void					reduceChildren();

public:
	CQuadBranch(const CQuadBranch &branch);
	CQuadBranch(uint8 level = 0) : IQuadNode(level) { uint i; for (i=0; i<4; ++i) _Children[i] = NULL; }
	~CQuadBranch() { uint i; for (i=0; i<4; ++i) delete _Children[i]; }
	CQuadBranch				&operator = (const CQuadBranch &branch);
	bool					isLeaf() const { return false; }
	const IQuadNode			*getChild(uint child) const
	{
		if (child > 3)	nlerror("Can't access child %d on the branch", child);
		return _Children[child];
	}
	void					setChild(uint child, IQuadNode *node)
	{
		if (child > 3)	nlerror("Can't set child %d on the branch", child);
		_Children[child] = node;
	}
	void					addVertex(const NLMISC::CVector &v);
	bool					check() const;

	void					translate(const NLMISC::CVector &translation)
	{
		IQuadNode::translate(translation);
		uint	i;
		for (i=0; i<4; ++i)
			if (_Children[i] != NULL)
				_Children[i]->translate(translation);
	}

	void					serial(NLMISC::IStream &f);
};

class CSurfaceQuadTree
{
protected:

	IQuadNode					*_Root;
	float						_MaxThickness;
	uint8						_MaxLevel;
	NLMISC::CAABBox				_BBox;

public:

	CSurfaceQuadTree();
	CSurfaceQuadTree(const CSurfaceQuadTree &quad);
	CSurfaceQuadTree			&operator = (const CSurfaceQuadTree &quad);

	const IQuadNode				*getRoot() const { return _Root; }
	float						getMaxThickness() const { return _MaxThickness; }
	uint8						getMaxLevel() const { return _MaxLevel; }
	const NLMISC::CAABBox		&getBBox() const { return _BBox; }

	void						clear();
	void						init(float maxThickness, uint maxLevel, const NLMISC::CVector &center, float halfSize=80.0f);
	void						addVertex(const NLMISC::CVector &v);
	void						compile();

	bool						check() const;
	const CQuadLeaf				*getLeaf(const NLMISC::CVector &v) const;

	float						getInterpZ(const NLMISC::CVector &v) const;

	void						translate(const NLMISC::CVector &translation)
	{
		_BBox.setCenter(_BBox.getCenter()+translation);
		if (_Root != NULL)
			_Root->translate(translation);
	}

	void						serial(NLMISC::IStream &f);
};

}; // NLPACS

#endif // NL_SURFACE_QUAD_H

/* End of surface_quad.h */
