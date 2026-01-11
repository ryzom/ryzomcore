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

#ifndef NL_MRM_MESH_H
#define NL_MRM_MESH_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/vector_h.h"
#include "nel/misc/vector.h"
#include "nel/3d/mesh.h"
#include <vector>


namespace NL3D
{


using NLMISC::CVector;
using NLMISC::CVectorH;


// ***************************************************************************
#define	NL3D_MRM_MAX_ATTRIB		12


// ***************************************************************************
/**
 * An internal mesh corner Index representation for MRM.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CMRMCorner
{
	// The id of the vertex.
	sint	Vertex;
	// The ids of the wedges. Points on Attributes of the mesh.
	sint	Attributes[NL3D_MRM_MAX_ATTRIB];

	// For CMRMMeshFinal computing, wedge ids.
	sint	WedgeStartId;
	sint	WedgeEndId;
	sint	WedgeGeomId;
};


// ***************************************************************************
/**
 * An internal mesh face representation for MRM.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct	CMRMFace
{
public:
	// The 3 corner indices of the face.
	CMRMCorner		Corner[3];
	// The id of the material. Used for material boundaries.
	sint			MaterialId;
};


// ***************************************************************************
/**
 * An internal mesh representation for MRM. USER DO NOT USE IT!!
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 * \see CMRMBuilder
 */
class	CMRMBlendShape
{
public:
	// The vertices of the MRMMesh.
	std::vector<CVector>		Vertices;
	// The attributes of the MRMMesh.
	std::vector<CVectorH>		Attributes[NL3D_MRM_MAX_ATTRIB];
	// The number of used attributes of the MRMMesh.
	sint						NumAttributes;
};


// ***************************************************************************
/**
 * An internal mesh representation for MRM. USER DO NOT USE IT!!
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 * \see CMRMBuilder
 */
class CMRMMesh
{
public:
	// The vertices of the MRMMesh.
	std::vector<CVector>				Vertices;
	// The skinWeights of the MRMMesh. same size than Vertices.
	std::vector<CMesh::CSkinWeight>		SkinWeights;
	// The link to the original meshInterface vertex. same size than Vertices.
	std::vector<CMesh::CInterfaceLink>	InterfaceLinks;
	// The attributes of the MRMMesh.
	std::vector<CVectorH>		Attributes[NL3D_MRM_MAX_ATTRIB];
	// The number of used attributes of the MRMMesh.
	sint						NumAttributes;
	// The faces of the MRMMesh.
	std::vector<CMRMFace>		Faces;


	/// List of BlendShapes.
	std::vector<CMRMBlendShape>		BlendShapes;


public:
	/// Constructor
	CMRMMesh();

};


// ***************************************************************************
/**
 * An internal mesh representation for MRM, with geomoprh information. USER DO NOT USE IT.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 * \see CMRMBuilder
 */
class CMRMMeshGeom : public CMRMMesh
{
public:
	/// Same size than Faces, but points onto coarser Mesh verices. NB: MaterialId means nothing here.
	std::vector<CMRMFace>		CoarserFaces;


	CMRMMeshGeom	&operator=(const CMRMMesh &o)
	{
		(CMRMMesh&)(*this)= o;
		// copy faces into CoarserFaces.
		CoarserFaces= Faces;
		return *this;
	}

public:
	/// Constructor
	CMRMMeshGeom() {}
};


// ***************************************************************************
/**
 * A geomoprh information.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 * \see CMRMBuilder
 */
struct	CMRMWedgeGeom
{
	/// The start wedge index of the geomorph.
	uint32	Start;
	/// The end wedge index of the geomorph.
	uint32	End;


	void	serial(NLMISC::IStream &f)
	{
		f.serial(Start, End);
	}
};


// ***************************************************************************
/**
 * An internal MRM mesh representation for MRM, with All lods information. USER DO NOT USE IT.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 * \see CMRMBuilder
 */
class	CMRMMeshFinal
{
public:

	// An wedge value (vertex + all attribs).
	struct	CWedge
	{
		CWedge();
		CMesh::CSkinWeight		VertexSkin;
		// Number of matrix this wedge use.
		uint		NSkinMatUsed;
		CVector		Vertex;
		CVectorH	Attributes[NL3D_MRM_MAX_ATTRIB];
		static		uint	NumAttributesToCompare;
		static		bool	CompareSkinning;

		bool	operator<(const CWedge &o) const
		{
			if(Vertex!=o.Vertex)
				return Vertex<o.Vertex;
			else
			{
				nlassert(NumAttributesToCompare<=NL3D_MRM_MAX_ATTRIB);
				for(uint i=0; i<NumAttributesToCompare; i++)
				{
					if(Attributes[i]!=o.Attributes[i])
						return Attributes[i]<o.Attributes[i];
				}
			}

			// They may be different by their skin Weight.
			if(CompareSkinning)
			{
				for(uint i=0; i<NL3D_MESH_SKINNING_MAX_MATRIX; i++)
				{
					if( VertexSkin.MatrixId[i] != o.VertexSkin.MatrixId[i] )
						return VertexSkin.MatrixId[i] < o.VertexSkin.MatrixId[i];
					if( VertexSkin.Weights[i] != o.VertexSkin.Weights[i] )
						return VertexSkin.Weights[i] < o.VertexSkin.Weights[i];
				}
			}

			// else they are equal.
			return false;
		}
	};

	// a face.
	struct	CFace
	{
		/// Three index on the wedegs.
		sint		WedgeId[3];
		/// the material id.
		sint		MaterialId;
	};


	// A LOD information for the final MRM representation.
	struct	CLod
	{
		/// this tells how many wedges in the Wedges array this lod requires. this is useful for partial loading.
		sint						NWedges;
		/// This is the face list for this LOD.
		std::vector<CFace>			Faces;
		/// List of geomorphs.
		std::vector<CMRMWedgeGeom>	Geomorphs;
	};


	struct	CMRMBlendShapeFinal
	{
		std::vector<CWedge>			Wedges;
	};

public:
	/** The wedges of the final mesh. Contains all Wedges for all lods, sorted from LOD0 to LODN,
	 * with additional empty wedges, for geomorph.
	 */
	std::vector<CWedge>			Wedges;
	/// This tells the number of empty wedges, for geomorph.
	sint						NGeomSpace;
	/// The number of used attributes of the MRMMesh.
	sint						NumAttributes;
	/// If the Mesh is skinned.
	bool						Skinned;
	/// the finals Lods of the MRM.
	std::vector<CLod>			Lods;


	std::vector<CMRMBlendShapeFinal>			MRMBlendShapesFinals;


	CMRMMeshFinal()
	{
		NumAttributes= 0;
	}


	/// add a wedge to this mesh, or return id if exist yet.
	sint	findInsertWedge(const CWedge &w);


	void	reset()
	{
		Wedges.clear();
		_WedgeMap.clear();
		Lods.clear();
		NumAttributes= 0;
	}


private:
	// The map of wedges to wedges index.
	typedef std::map<CWedge, sint>		TWedgeMap;
	TWedgeMap			_WedgeMap;

};


} // NL3D


#endif // NL_MRM_MESH_H

/* End of mrm_mesh.h */
