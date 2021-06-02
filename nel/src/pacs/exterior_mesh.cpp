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

#include "nel/pacs/exterior_mesh.h"
#include "nel/pacs/local_retriever.h"
#include "nel/pacs/collision_desc.h"


using namespace std;
using namespace NLMISC;

namespace NLPACS
{
	// Functions for vertices comparison.
	// total order relation
	static inline bool	isStrictlyLess(const CVector &a, const CVector &b)
	{
		if (a.x < b.x)	return true;
		if (a.x > b.x)	return false;
		if (a.y < b.y)	return true;
		if (a.y > b.y)	return false;
		if (a.z < b.y)	return true;
		return false;
	}

	static inline bool	isStrictlyGreater(const CVector &a, const CVector &b)
	{
		if (a.x > b.x)	return true;
		if (a.x < b.x)	return false;
		if (a.y > b.y)	return true;
		if (a.y < b.y)	return false;
		if (a.z > b.y)	return true;
		return false;
	}

	CExteriorMesh::CExteriorMesh() { }

	void	CExteriorMesh::setEdges(const vector<CExteriorMesh::CEdge> &edges)
	{
		_Edges = edges;
		_OrderedEdges.clear();

		uint	i;
		for (i=0; i+1<_Edges.size(); )
		{
			_OrderedEdges.resize(_OrderedEdges.size()+1);
			COrderedEdges	&edges = _OrderedEdges.back();
			edges.Start = i;
			if (isStrictlyLess(_Edges[i].Start, _Edges[i+1].Start))
			{
				edges.Forward = true;
				do
				{
					++i;
				}
				while (i+1<_Edges.size() && _Edges[i].Link != -2 && isStrictlyLess(_Edges[i].Start, _Edges[i+1].Start));
			}
			else
			{
				edges.Forward = false;
				do
				{
					++i;
				}
				while (i+1<_Edges.size() && _Edges[i].Link != -2 && isStrictlyGreater(_Edges[i].Start, _Edges[i+1].Start));
			}
			edges.End = i;

			if (_Edges[i].Link == -2)
				++i;
		}
	}

	void	CExteriorMesh::serial(NLMISC::IStream &f)
	{
		/*
		Version 0:
			- base version.
		*/
		(void)f.serialVersion(0);

		f.serialCont(_Edges);
		f.serialCont(_OrderedEdges);
		f.serialCont(_Links);
		f.serial(_BBox);
	}
};
