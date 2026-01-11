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

#ifndef NL_VEGETABLE_SHAPE_H
#define NL_VEGETABLE_SHAPE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"


namespace NL3D
{



// ***************************************************************************
/**
 * A vegetable shape. Ie not deformed by instanciation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVegetableShapeBuild
{
public:

	/** Bend center controls how each vertex is bended.
	 *	BendCenterNull: For each Center of bend is made on (0,0,0).
	 *	BendCenterZ: Center of bend is made on (vertx, verty, 0), perfect for "comb-like" vegetation.
	 */
	enum	TBendCenterMode	{BendCenterNull=0, BendCenterZ, BendCenterModeCount};

	/** The standard input vertexBuffer
	 *	If it has vertexColor, then BendWeight is read from color.R, and scale to take MaxBendWeight at max.
	 *		else, BendWeight is read from pos.z (clamped to (0, maxZ), and scaled to take MaxBendWeight at max.
	 *	If it has normal, and Lighted==true, then it is lighted.
	 *	Must have TexCoord0.
	 */
	CVertexBuffer		VB;
	/// only triangles of PB are used.
	CIndexBuffer		PB;
	/// If this shape must be lighted
	bool				Lighted;
	/// if Lighted && PreComputeLighting, lighting is precomputed per instance.
	bool				PreComputeLighting;
	/// If this shape must be 2Sided
	bool				DoubleSided;
	/** If this shape must be AlphaBlended (and hence ZSorted).
	 *	NB: valid ONLY if (!Lighted || PreComputeLighting) && DoubleSided.
	 */
	bool				AlphaBlend;
	/** true If this shape must take one sided lighting. ie max(normal, -normal) is computed
	*	during ligthing compute
	 *	NB: valid ONLY if (PreComputeLighting)
	 */
	bool				BestSidedPreComputeLighting;
	/// The maximum BendWeight to apply.
	float				MaxBendWeight;
	/// The BendCenter mode
	TBendCenterMode		BendCenterMode;

public:
	CVegetableShapeBuild()
	{
		Lighted= false;
		DoubleSided= false;
		PreComputeLighting= false;
		AlphaBlend= false;
		BestSidedPreComputeLighting= false;
		MaxBendWeight= 0;
		BendCenterMode= BendCenterNull;
		NL_SET_IB_NAME(PB, "CVegetableShapeBuild");
	}

};



// ***************************************************************************
/**
 * A vegetable shape. Ie not deformed by instanciation.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2001
 */
class CVegetableShape
{
public:

	/// Constructor
	CVegetableShape();

	/**	build a vegetable shape, with a standard shape.
	 *
	 */
	void		build(CVegetableShapeBuild &vbuild);


	/** load a vegetable shape (.veget), lookup into CPath, and serial
	 */
	bool		loadShape(const std::string &shape);

	/// \name Shape def
	// @{

	/// Type of this shape.
	bool					Lighted;
	bool					DoubleSided;
	bool					PreComputeLighting;
	bool					AlphaBlend;
	bool					BestSidedPreComputeLighting;
	CVegetableShapeBuild::TBendCenterMode			BendCenterMode;
	/** VertexBuffer of this Shape, ready to be transformed and copied into vegetable manager
	 *	Format is Pos/Normal/Tex0/Tex1 (no Normal if !Lighted). where Tex1.U==BendWeigth
	 */
	CVertexBuffer			VB;
	/// list of triangles index
	std::vector<uint32>		TriangleIndices;
	// @}


	/// \name Temporary for easy Instanciation (correct size)
	// @{
	/// For each vertex of the shape, gives the index in the VegetableManager
	std::vector<uint32>		InstanceVertices;
	// @}


	/// serial
	void		serial(NLMISC::IStream &f);

private:
};


} // NL3D


#endif // NL_VEGETABLE_SHAPE_H

/* End of vegetable_shape.h */
