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

#ifndef NL_CHAIN_H
#define NL_CHAIN_H

#include <vector>
#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "vector_2s.h"
#include "nel/misc/file.h"

namespace NLPACS
{
class COrderedChain;

/**
 * A list of ordered vertices, partially delimiting 2 different surfaces.
 * In the vertex list, we consider the following order
 *    v1 < v2 iff  v1.x < v2.x  ||  v1.x == v2.x && v1.y < v2.y  ||  v1.x == v2.x && v1.y == v2.y && v1.z < v2.z
 * The vertices composing the chain are actual CVector (12 bytes per vertex.)
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class COrderedChain3f
{
protected:
	friend class CChain;
	friend class CRetrievableSurface;

	/// The vertices of the chain, ordered following x growth.
	std::vector<NLMISC::CVector>		_Vertices;

	/// Set if the chain should be read forward within the parent CChain (for sequential access to vertices.)
	bool								_Forward;

	/// The parent chain Id.
	uint16								_ParentId;

	/// The index of the ochain within the parent chain
	uint16								_IndexInParent;

public:
	/// Returns the vertices of the chain
	const std::vector<NLMISC::CVector>	&getVertices() const { return _Vertices; }

	/// Returns true if the chain should be accessed forward within the parent CChain (see _Forward.)
	bool								isForward() const { return _Forward; }

	/// Returns the parent chain Id of this ordered chain.
	uint16								getParentId() const { return _ParentId; }

	/// Returns the index of the ochain within the parent chain.
	uint16								getIndexInParent() const { return _IndexInParent; }

	///
	const NLMISC::CVector				&operator[] (uint n) const { return _Vertices[n]; }

	///
	void								unpack(const COrderedChain &ochain);

	///
	void								translate(const NLMISC::CVector &translation)
	{
		uint	i;
		for (i=0; i<_Vertices.size(); ++i)
			_Vertices[i] += translation;
	}

	void								serial(NLMISC::IStream &f);
};

/**
 * A list of ordered vertices, partially delimiting 2 different surfaces.
 * In the vertex list, we consider the following order
 *    v1 < v2 iff  v1.x < v2.x  ||  v1.x == v2.x && v1.y < v2.y
 * The vertices composing the chain are only 2 coordinates (x, y) wide, packed on 16 bits each
 * (which is 4 bytes per vertex.) This is the packed form of the COrderedChain3f.
 * (4 bytes per vertex.)
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class COrderedChain
{
protected:
	friend class CChain;
	friend class CRetrievableSurface;

	/// The vertices of the chain, ordered following x growth.
	std::vector<CVector2s>				_Vertices;

	/// Set if the chain should be read forward within the parent CChain (for sequential access to vertices.)
	bool								_Forward;

	/// The parent chain Id.
	uint16								_ParentId;

	/// The index of the ochain within the parent chain
	uint16								_IndexInParent;

	/// The length of the chain.
	float								_Length;

	/// The min and max vertices of the chain
	CVector2s							_Min, _Max;

public:
	/// Returns the vertices of the chain
	const std::vector<CVector2s>		&getVertices() const { return _Vertices; }

	/// Returns true if the chain should be accessed forward within the parent CChain (see _Forward.)
	bool								isForward() const { return _Forward; }

	/// Returns the parent chain Id of this ordered chain.
	uint16								getParentId() const { return _ParentId; }

	/// Returns the index of the ochain within the parent chain.
	uint16								getIndexInParent() const { return _IndexInParent; }

	/// Returns the length of the chain
	float								getLength() const { return _Length; }

	///
	const CVector2s						&operator[] (uint n) const { return _Vertices[n]; }

	/// Returns the min vector of the chain
	const CVector2s						&getMin() const { return _Min; };

	/// Returns the max vector of the chain
	const CVector2s						&getMax() const { return _Max; };


	///
	void								translate(const NLMISC::CVector &translation);

	///
	void								pack(const COrderedChain3f &chain)
	{
		uint	i;
		const std::vector<NLMISC::CVector>	&vertices = chain.getVertices();
		_Vertices.resize(vertices.size());
		_Forward = chain.isForward();
		_ParentId = chain.getParentId();
		_IndexInParent = chain.getIndexInParent();
		for (i=0; i<vertices.size(); ++i)
		{
			_Vertices[i] = CVector2s(vertices[i]);
			_Min.minof(_Min, _Vertices[i]);
			_Max.maxof(_Max, _Vertices[i]);
		}
	}

	///
	void								computeMinMax()
	{
		_Min = _Max = CVector2s(0, 0);

		if (_Vertices.empty())
			return;

		_Min = _Max = _Vertices[0];
		uint	i;
		for (i=1; i<_Vertices.size(); ++i)
		{
			_Min.minof(_Min, _Vertices[i]);
			_Max.maxof(_Max, _Vertices[i]);
		}
	}

	///
	void								traverse(sint from, sint to, bool forward, std::vector<CVector2s> &path) const;

	///
	float								distance(const NLMISC::CVector &position) const;

	///
	void								serial(NLMISC::IStream &f);
};

/**
 * A list of ordered chains of vertices, delimiting 2 surfaces.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CChain
{
protected:
	friend class CRetrievableSurface;
	friend class CLocalRetriever;

	/// The list of ordered chains that compose the chain.
	std::vector<uint16>					_SubChains;

	/// The surface on the left of the chain.
	sint32								_Left;

	/// The surface on the right of the chain.
	sint32								_Right;

	/// The tips indexes in the retriever object.
	uint16								_StartTip;
	uint16								_StopTip;

	/// The length of the whole chain.
	float								_Length;

	uint8								_LeftLoop, _LeftLoopIndex;
	uint8								_RightLoop, _RightLoopIndex;

protected:
	/// Build the whole surface from a vector of CVector and the left and right surfaces.
	void								make(const std::vector<NLMISC::CVector> &vertices, sint32 left, sint32 right, std::vector<COrderedChain> &chains, uint16 thisId,
											 std::vector<COrderedChain3f> &fullChains, std::vector<uint> &useOChainId);

	void								setLoopIndexes(sint32 surface, uint loop, uint loopIndex)
	{
		if (_Left == surface)
		{
			_LeftLoop = uint8(loop);
			_LeftLoopIndex = uint8(loopIndex);
		}
		else
		{
			_RightLoop = uint8(loop);
			_RightLoopIndex = uint8(loopIndex);
		}
	}

	void								setBorderChainIndex(sint32 id)	{ _Right = convertBorderChainId(id); }

public:

	/// Constructor.
	CChain() :	_Left(-1), _Right(-1), _StartTip(0xffff), _StopTip(0xffff), _Length(0.0f),
				_LeftLoop(0), _LeftLoopIndex(0), _RightLoop(0), _RightLoopIndex(0) {}

	/// Returns a vector of ordered chain ids that compose the entire chain.
	const std::vector<uint16>			&getSubChains() const { return _SubChains; }

	/// Returns the id of the nth ordered chain that composes the chain.
	uint16								getSubChain(uint n) const { return _SubChains[n]; }

	/// Returns the left surface id.
	sint32								getLeft() const { return _Left; }
	uint8								getLeftLoop() const { return _LeftLoop; }
	uint8								getLeftLoopIndex() const { return _LeftLoopIndex; }

	/// Returns the right surface id.
	sint32								getRight() const { return _Right; }
	uint8								getRightLoop() const { return _RightLoop; }
	uint8								getRightLoopIndex() const { return _RightLoopIndex; }

	/// returns the legnth of the whole chain.
	float								getLength() const { return _Length; }

	/// Gets the index of the chain on border (in the local retriever object.)
	sint32								getBorderChainIndex() const
	{
		return (isBorderChainId(_Right)) ? convertBorderChainId(_Right) : -1;
	}

	/// Returns true iff the given id corresponds to a link on the border.
	static bool							isBorderChainId(sint32 id) { return id <= -256; }

	/// Converts the surf id into the real index to the link (in the BorderChainLinks of the CRetrieverInstance.)
	static sint32						convertBorderChainId(sint32 id) { return -(id+256); }

	/// Returns true iff the chaion is a border chain
	bool								isBorderChain() const { return isBorderChainId(_Right); }

	/// Returns true iff the chaion is a border chain
	static sint32						getDummyBorderChainId() { return convertBorderChainId(0); }

	/// Returns the id of the start tip of the chain.
	uint16								getStartTip() const { return _StartTip; }

	/// Returns the id of the end tip of the chain.
	uint16								getStopTip() const { return _StopTip; }

	/// Serialises the CChain object.
	void								serial(NLMISC::IStream &f);

protected:
	///
	void								unify(std::vector<COrderedChain> &ochains);

	///
	void								setStartVector(const CVector2s &v, std::vector<COrderedChain> &ochains);

	///
	void								setStopVector(const CVector2s &v, std::vector<COrderedChain> &ochains);

	///
	CVector2s							getStartVector(std::vector<COrderedChain> &ochains);

	//
	CVector2s							getStopVector(std::vector<COrderedChain> &ochains);
};





//
inline void								COrderedChain3f::unpack(const COrderedChain &chain)
{
	uint	i, mx;
	const std::vector<CVector2s>	&vertices = chain.getVertices();
	mx = (uint)_Vertices.size();
	_Vertices.resize(vertices.size());
	_Forward = chain.isForward();
	_ParentId = chain.getParentId();
	_IndexInParent = chain.getIndexInParent();
	for (i=0; i<vertices.size(); ++i)
	{
		_Vertices[i] = vertices[i].unpack3f(i >= mx ? 0.0f : _Vertices[i].z);
	}
}

}; // NLPACS

#endif // NL_CHAIN_H

/* End of chain.h */
