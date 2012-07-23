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

#ifndef NL_EXTERIOR_MESH_H
#define NL_EXTERIOR_MESH_H

#include <vector>

#include "nel/misc/types_nl.h"
#include "nel/misc/vector.h"
#include "nel/misc/file.h"

#include "nel/misc/aabbox.h"

#include "vector_2s.h"
#include "surface_quad.h"
#include "chain.h"
#include "retrievable_surface.h"
#include "chain_quad.h"

#include "nel/pacs/u_global_position.h"



namespace NLPACS
{

/**
 * The external mesh of a interior local retriever.
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CExteriorMesh
{
public:
	/// An edge of the edge list
	struct CEdge
	{
		NLMISC::CVector					Start;
		sint32							Link;

		CEdge() {}
		CEdge(const CVector &start, sint32 link) : Start(start), Link(link) {}
		void	serial(NLMISC::IStream &f) { f.serial(Start, Link); }
	};

	/// A list of edges that are sorted
	struct COrderedEdges
	{
		uint32							Start, End;
		bool							Forward;
		void	serial(NLMISC::IStream &f) { f.serial(Start, End, Forward); }
	};

	/// A neighbor link, on an interior surface
	class CLink
	{
	public:
		uint16				BorderChainId;
		uint16				ChainId;
		uint16				SurfaceId;
		CLink() : BorderChainId(0xFFFF), ChainId(0xFFFF), SurfaceId(0xFFFF) {}
		void	serial(NLMISC::IStream &f) { f.serial(BorderChainId, ChainId, SurfaceId); }
	};

protected:
	std::vector<CEdge>					_Edges;
	std::vector<COrderedEdges>			_OrderedEdges;

	std::vector<CLink>					_Links;

	NLMISC::CAABBox						_BBox;


public:
	/// @name Constructors
	// @{

	CExteriorMesh();

	// @}

	void	clear()
	{
		NLMISC::contReset(_Edges);
		NLMISC::contReset(_OrderedEdges);
		NLMISC::contReset(_Links);
	}

	/// @name Selectors
	// @{

	/// Get the set of edges that forms the whole mesh
	const std::vector<CEdge>			&getEdges() const { return _Edges; }

	/// Get the nth edge of the mesh
	const CEdge							getEdge(uint n) const { return _Edges[n]; }


	/// Get the ordered edges
	const std::vector<COrderedEdges>	&getOrderedEdges() const { return _OrderedEdges; }

	/// Get the nth set of ordered edges
	const COrderedEdges					&getOrderedEdges(uint n) const { return _OrderedEdges[n]; }


	/// Get the links
	const std::vector<CLink>			&getLinks() const { return _Links; }
	/// Get a specific link
	const CLink							&getLink(uint n) const { return _Links[n]; }

	/// Get the link on a specific edge
	CLink								getLinkFromEdge(uint edge) const { return (_Edges[edge].Link != -1) ? _Links[_Edges[edge].Link] : CLink(); }


	/// Get the bbox of the mesh
	const NLMISC::CAABBox				&getBBox() const { return _BBox; }

	// @}


	/// @name Mutators/initialisation
	// @{

	/// Set the edges
	void								setEdges(const std::vector<CEdge> &edges);

	/// Set the links
	void								setLinks(const std::vector<CLink> &links) { _Links = links; }

	// @}

	/// Serializes the mesh
	void								serial(NLMISC::IStream &f);
};

}; // NLPACS

#endif // NL_EXTERIOR_MESH_H

/* End of exterior_mesh.h */
