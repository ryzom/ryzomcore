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

#ifndef NL_MESHVP_WIND_TREE_H
#define NL_MESHVP_WIND_TREE_H

#include "nel/misc/types_nl.h"
#include "nel/3d/mesh_vertex_program.h"
#include "nel/3d/vertex_program.h"


namespace NL3D {

class CVertexProgramWindTree;

// ***************************************************************************
/**
 * VertexProgram for an effect of Wind on Tree meshes.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2002
 */
class CMeshVPWindTree : public IMeshVertexProgram
{
public:
	friend class CVertexProgramWindTree;

	enum	{HrcDepth= 3};

	/// \name WindTree Parameters;
	// @{

	/// Frequency of the wind for 3 Hierachy levels
	float		Frequency[HrcDepth];
	/// Additional frequency, multiplied by the globalWindPower
	float		FrequencyWindFactor[HrcDepth];
	/// Power of the wind on XY. Mul by globalWindPower
	float		PowerXY[HrcDepth];
	/// Power of the wind on Z. Mul by globalWindPower
	float		PowerZ[HrcDepth];
	/// Bias result of the cosinus: f= cos(time)+bias.
	float		Bias[HrcDepth];

	/// true if want Specular Lighting.
	bool		SpecularLighting;

	// @}

public:

	/// Constructor
	CMeshVPWindTree();
	virtual ~CMeshVPWindTree();


	/// \name IMeshVertexProgram implementation
	// @{

	/// Setup a rand phase for wind in mbi
	virtual	void	initInstance(CMeshBaseInstance *mbi);
	/// Setup Wind constants, Light constants, and activate the VP.
	virtual	bool	begin(IDriver *drv,
						  CScene *scene,
						  CMeshBaseInstance *mbi,
						  const NLMISC::CMatrix &invertedModelMat,
						  const NLMISC::CVector & /*viewerPos*/);
	/// disable the VertexProgram.
	virtual	void	end(IDriver *drv);

	// Setup this shader for the given material.
	virtual void	setupForMaterial(const CMaterial &mat,
									 IDriver *drv,
									 CScene *scene,
									 CVertexBuffer *vb);

	// Max VP Distance movement
	virtual float	getMaxVertexMove();

	// Serial.
	virtual void	serial(NLMISC::IStream &f) throw(NLMISC::EStream);
	NLMISC_DECLARE_CLASS(CMeshVPWindTree);

	// @}

	/** \name MBR support For WindTree
	*/
	// @{
	virtual	bool	supportMeshBlockRendering() const;
	virtual	bool	isMBRVpOk(IDriver *drv) const;
	virtual	void	beginMBRMesh(IDriver *drv, CScene *scene);
	virtual	void	beginMBRInstance(IDriver *drv, CScene *scene, CMeshBaseInstance *mbi, const NLMISC::CMatrix &invertedModelMat);
	virtual	void	endMBRMesh(IDriver *drv);
	// @}

private:
	void	setupLighting(CScene *scene, CMeshBaseInstance *mbi, const NLMISC::CMatrix &invertedModelMat);
private:

	enum { NumVp = 16};

	/** The 16 versions: Specular or not (0 or 2), + normalize normal or not (0 or 1).
	 *	All multiplied by 4, because support from 0 to 3 pointLights activated. (0.., 4.., 8.., 12..)
	 */
	static	NLMISC::CSmartPtr<CVertexProgramWindTree> _VertexProgram[NumVp];

	NLMISC::CRefPtr<CVertexProgramWindTree> _ActiveVertexProgram;

	// WindTree Time for this mesh param setup. Stored in mesh because same for all instances.
	float		_CurrentTime[HrcDepth];
	double		_LastSceneTime;

	// maximum amplitude vector for each level. Stored in mesh because same for all instances.
	CVector		_MaxDeltaPos[HrcDepth];
	float		_MaxVertexMove;

	// MBR Cache
	uint		_LastMBRIdVP;

	// Compute a cosinus with an angle given in 0-1 <=> 0-2Pi. Actual values goes from 0 to 2.
	static float	speedCos(float angle);


	void		setupPerMesh(IDriver *driver, CScene *scene);
	void		setupPerInstanceConstants(IDriver *driver, CScene *scene, CMeshBaseInstance *mbi, const NLMISC::CMatrix &invertedModelMat);

};


} // NL3D


#endif // NL_MESHVP_WIND_TREE_H

/* End of meshvp_wind_tree.h */
