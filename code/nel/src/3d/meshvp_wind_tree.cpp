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

#include "std3d.h"

#include "nel/3d/meshvp_wind_tree.h"
#include "nel/3d/mesh_base_instance.h"
#include "nel/3d/scene.h"
#include "nel/3d/driver.h"
#include <cmath>
#include "nel/misc/common.h"
#include "nel/3d/render_trav.h"


using namespace NLMISC;
using namespace std;


namespace NL3D
{


// ***************************************************************************
// Light VP fragment constants start at 24
static	const uint	VPLightConstantStart= 24;


// ***************************************************************************
std::auto_ptr<CVertexProgram>	CMeshVPWindTree::_VertexProgram[CMeshVPWindTree::NumVp];

static const char*	WindTreeVPCodeWave=
"!!VP1.0																				\n\
  # extract from color.R the 3 factors into R0.xyz									\n\
	MAD	R0, v[3].x, c[9].x, c[9].yzww;	# col.R*3										\n\
	MIN	R0, R0, c[8].yyyy;				# clamp each to 0,1								\n\
	MAX	R0, R0, c[8].xxxx;																\n\
																						\n\
	# Add influence of Bone Level1														\n\
	MAD	R5, c[15], R0.x, v[0];															\n\
																						\n\
	# Sample LevelPhase into R7.yz: 0 to 3.												\n\
	MUL	R7, v[3].xyzw, c[10].x;															\n\
																						\n\
	# Add influence of Bone Level2														\n\
	ARL	A0.x, R7.y;																		\n\
	MAD	R5, c[A0.x+16], R0.y, R5;														\n\
																						\n\
	# Add influence of Bone Level3														\n\
	ARL	A0.x, R7.z;																		\n\
	MAD	R5, c[A0.x+20], R0.z, R5;														\n\
																						\n\
	# Get normal in R6 for lighting.													\n\
	MOV	R6, v[2];																		\n\
";

static const char*	WindTreeVPCodeEnd=
"	# compute in Projection space														\n\
	DP4 o[HPOS].x, c[0], R5;															\n\
	DP4 o[HPOS].y, c[1], R5;															\n\
	DP4 o[HPOS].z, c[2], R5;															\n\
	DP4 o[HPOS].w, c[3], R5;															\n\
	MOV o[TEX0], v[8];																\n\
	# hulud : remove this line for the moment because it doesn't work under d3d, if it is needed, we will have to create 2 CVertexProgram objects.\n\
	#MOV o[TEX1], v[9];																\n\
	DP4	o[FOGC].x, c[6], R5;															\n\
	END																					\n\
";

// ***************************************************************************
float	CMeshVPWindTree::speedCos(float angle)
{
	// \todo yoyo TODO_OPTIM
	return cosf(angle * 2*(float)Pi);
}


// ***************************************************************************
CMeshVPWindTree::CMeshVPWindTree()
{
	for(uint i=0; i<HrcDepth; i++)
	{
		Frequency[i]= 1;
		FrequencyWindFactor[i]= 0;
		PowerXY[i]= 0;
		PowerZ[i]= 0;
		Bias[i]= 0;
		// Init currentTime.
		_CurrentTime[i]= 0;
	}
	SpecularLighting= false;

	_LastSceneTime= 0;
	_MaxVertexMove= 0;
}


// ***************************************************************************
CMeshVPWindTree::~CMeshVPWindTree()
{
}


// ***************************************************************************
void	CMeshVPWindTree::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	(void)f.serialVersion(0);

	nlassert(HrcDepth==3);
	for(uint i=0; i<HrcDepth; i++)
	{
		f.serial(Frequency[i]);
		f.serial(FrequencyWindFactor[i]);
		f.serial(PowerXY[i]);
		f.serial(PowerZ[i]);
		f.serial(Bias[i]);
	}
	f.serial(SpecularLighting);
}


// ***************************************************************************
void	CMeshVPWindTree::initInstance(CMeshBaseInstance *mbi)
{
	// init the vertexProgram code.
	static	bool	vpCreated= false;
	if(!vpCreated)
	{
		vpCreated= true;
		// All vpcode and begin() written for HrcDepth==3
		nlassert(HrcDepth==3);

		// combine fragments.
		string	vpCode;

		// For all possible VP.
		for(uint i=0;i<NumVp;i++)
		{
			// setup of the VPLight fragment
			uint	numPls= i/4;
			bool	normalize= (i&1)!=0;
			bool	specular= (i&2)!=0;

			// combine fragments
			vpCode= string(WindTreeVPCodeWave)
					+ CRenderTrav::getLightVPFragment(numPls, VPLightConstantStart, specular, normalize)
					+ WindTreeVPCodeEnd;
			_VertexProgram[i]= std::auto_ptr<CVertexProgram>(new CVertexProgram(vpCode.c_str()));
			// TODO_VP_GLSL
		}
	}

	// init a random phase.
	mbi->_VPWindTreePhase= frand(1);
}

// ***************************************************************************
inline void			CMeshVPWindTree::setupPerMesh(IDriver *driver, CScene *scene)
{
	// process current times and current power. Only one time per render() and per CMeshVPWindTree.
	if(scene->getCurrentTime() != _LastSceneTime)
	{
		// Get info from scene
		float	windPower= scene->getGlobalWindPower();

		float	dt= (float)(scene->getCurrentTime() - _LastSceneTime);
		_LastSceneTime= scene->getCurrentTime();

		// Update each boneLevel time according to frequency.
		uint i;
		for(i=0; i<HrcDepth; i++)
		{
			_CurrentTime[i]+= dt*(Frequency[i] + FrequencyWindFactor[i]*windPower);
			// get it between 0 and 1. Important for float precision problems.
			_CurrentTime[i]= (float)fmod(_CurrentTime[i], 1);
		}

		// Update each boneLevel maximum amplitude vector.
		for(i=0; i<HrcDepth; i++)
		{
			_MaxDeltaPos[i]= scene->getGlobalWindDirection() * PowerXY[i] * windPower;
			_MaxDeltaPos[i].z= PowerZ[i] * windPower;
		}

		/* Update the Max amplitude distance
			in world space, since maxdeltaPos are applied in world space, see setupPerInstanceConstants()
		*/
		_MaxVertexMove= 0;
		for(i=0; i<HrcDepth; i++)
		{
			_MaxVertexMove+= _MaxDeltaPos[i].norm();
		}
	}

	// Setup common constants for each instances.
	// c[8] take useful constants.
	static	float	ct8[4]= {0, 1, 0.5f, 2};
	driver->setConstant(8, 1, ct8);
	// c[9] take other useful constants.
	static	float	ct9[4]= {3.f, 0.f, -1.f, -2.f};
	driver->setConstant(9, 1, ct9);
	// c[10] take Number of phase (4) for level2 and 3. -0.01 to avoid int value == 4.
	static	float	ct10[4]= {4-0.01f, 0, 0, 0};
	driver->setConstant(10, 1, ct10);
}

// ***************************************************************************
inline	void		CMeshVPWindTree::setupPerInstanceConstants(IDriver *driver, CScene *scene, CMeshBaseInstance *mbi, const NLMISC::CMatrix &invertedModelMat)
{
	// get instance info
	float	instancePhase= mbi->_VPWindTreePhase;


	// maxDeltaPos in ObjectSpace. So same world Wind direction is applied to all objects
	static	CMatrix		invWorldMatrix;
	// Keep only rotation part. (just need it and faster invert)
	invWorldMatrix.setRot(mbi->getWorldMatrix());
	invWorldMatrix.invert();
	static	CVector		maxDeltaPosOS[HrcDepth];
	for(uint i=0; i<HrcDepth; i++)
	{
		maxDeltaPosOS[i]= invWorldMatrix.mulVector(_MaxDeltaPos[i]);
	}


	// Setup lighting and lighting constants
	setupLighting(scene, mbi, invertedModelMat);

	// c[0..3] take the ModelViewProjection Matrix. After setupModelMatrix();
	driver->setConstantMatrix(0, IDriver::ModelViewProjection, IDriver::Identity);
	// c[4..7] take the ModelView Matrix. After setupModelMatrix();00
	driver->setConstantFog(6);


	// c[15] take Wind of level 0.
	float	f;
	f= _CurrentTime[0] + instancePhase;
	f= speedCos(f) + Bias[0];
	driver->setConstant(15, maxDeltaPosOS[0]*f );


	// c[16-19] take Wind of level 1.
	// Unrolled.
	float	instTime1= _CurrentTime[1] + instancePhase;
	// phase 0.
	f= speedCos( instTime1+0 ) + Bias[1];
	driver->setConstant(16+0, maxDeltaPosOS[1]*f);
	// phase 1.
	f= speedCos( instTime1+0.25f ) + Bias[1];
	driver->setConstant(16+1, maxDeltaPosOS[1]*f);
	// phase 2.
	f= speedCos( instTime1+0.50f ) + Bias[1];
	driver->setConstant(16+2, maxDeltaPosOS[1]*f);
	// phase 3.
	f= speedCos( instTime1+0.75f ) + Bias[1];
	driver->setConstant(16+3, maxDeltaPosOS[1]*f);


	// c[20, 23] take Wind of level 2.
	// Unrolled.
	float	instTime2= _CurrentTime[2] + instancePhase;
	// phase 0.
	f= speedCos( instTime2+0 ) + Bias[2];
	driver->setConstant(20+0, maxDeltaPosOS[2]*f);
	// phase 1.
	f= speedCos( instTime2+0.25f ) + Bias[2];
	driver->setConstant(20+1, maxDeltaPosOS[2]*f);
	// phase 2.
	f= speedCos( instTime2+0.50f ) + Bias[2];
	driver->setConstant(20+2, maxDeltaPosOS[2]*f);
	// phase 3.
	f= speedCos( instTime2+0.75f ) + Bias[2];
	driver->setConstant(20+3, maxDeltaPosOS[2]*f);
}

// ***************************************************************************
bool	CMeshVPWindTree::begin(IDriver *driver, CScene *scene, CMeshBaseInstance *mbi, const NLMISC::CMatrix &invertedModelMat, const NLMISC::CVector & /*viewerPos*/)
{
	if (!(driver->supportVertexProgram() && !driver->isVertexProgramEmulated())) return false;


	// precompute mesh
	setupPerMesh(driver, scene);

	// Setup instance constants
	setupPerInstanceConstants(driver, scene, mbi, invertedModelMat);

	// Activate the good VertexProgram
	//===============

	// Get how many pointLights are setuped now.
	nlassert(scene != NULL);
	CRenderTrav		*renderTrav= &scene->getRenderTrav();
	sint	numPls= renderTrav->getNumVPLights()-1;
	clamp(numPls, 0, CRenderTrav::MaxVPLight-1);

	// Enable normalize only if requested by user. Because lighting don't manage correct "scale lighting"
	uint	idVP= (SpecularLighting?2:0) + (driver->isForceNormalize()?1:0) ;
	// correct VP id for correct unmber of pls.
	idVP= numPls*4 + idVP;

	// activate VP.
	driver->activeVertexProgram(_VertexProgram[idVP].get());


	return true;
}

// ***************************************************************************
void	CMeshVPWindTree::end(IDriver *driver)
{
	// Disable the VertexProgram
	driver->activeVertexProgram(NULL);
}

// ***************************************************************************
// tool fct
static inline void SetupForMaterial(const CMaterial &mat, CScene *scene)
{
	CRenderTrav		*renderTrav= &scene->getRenderTrav();
	renderTrav->changeVPLightSetupMaterial(mat, false /* don't exclude strongest */);
}

// ***************************************************************************
void	CMeshVPWindTree::setupForMaterial(const CMaterial &mat,
										  IDriver *drv,
									      CScene *scene,
										  CVertexBuffer *)
{
	SetupForMaterial(mat, scene);
}

// ***************************************************************************
void	CMeshVPWindTree::setupLighting(CScene *scene, CMeshBaseInstance *mbi, const NLMISC::CMatrix &invertedModelMat)
{
	nlassert(scene != NULL);
	CRenderTrav		*renderTrav= &scene->getRenderTrav();
	// setup cte for lighting
	renderTrav->beginVPLightSetup(VPLightConstantStart, SpecularLighting, invertedModelMat);
}


// ***************************************************************************
// ***************************************************************************
// MBR interface
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool	CMeshVPWindTree::supportMeshBlockRendering() const
{
	return true;
}

// ***************************************************************************
bool	CMeshVPWindTree::isMBRVpOk(IDriver *driver) const
{
	return driver->supportVertexProgram() && !driver->isVertexProgramEmulated();
}

// ***************************************************************************
void	CMeshVPWindTree::beginMBRMesh(IDriver *driver, CScene *scene)
{
	// precompute mesh
	setupPerMesh(driver, scene);

	/* Since need a VertexProgram Activation before activeVBHard, activate a default one
		bet the common one will be "NoPointLight, NoSpecular, No ForceNormalize" => 0.
	*/
	_LastMBRIdVP= 0;

	// activate VP.
	driver->activeVertexProgram(_VertexProgram[_LastMBRIdVP].get());
}

// ***************************************************************************
void	CMeshVPWindTree::beginMBRInstance(IDriver *driver, CScene *scene, CMeshBaseInstance *mbi, const NLMISC::CMatrix &invertedModelMat)
{
	// setup first constants for this instance
	setupPerInstanceConstants(driver, scene, mbi, invertedModelMat);

	// Get how many pointLights are setuped now.
	nlassert(scene != NULL);
	CRenderTrav		*renderTrav= &scene->getRenderTrav();
	sint	numPls= renderTrav->getNumVPLights()-1;
	clamp(numPls, 0, CRenderTrav::MaxVPLight-1);

	// Enable normalize only if requested by user. Because lighting don't manage correct "scale lighting"
	uint	idVP= (SpecularLighting?2:0) + (driver->isForceNormalize()?1:0) ;
	// correct VP id for correct number of pls.
	idVP= numPls*4 + idVP;

	// re-activate VP if idVP different from last setup
	if( idVP!=_LastMBRIdVP )
	{
		_LastMBRIdVP= idVP;
		driver->activeVertexProgram(_VertexProgram[_LastMBRIdVP].get());
	}
}

// ***************************************************************************
void	CMeshVPWindTree::endMBRMesh(IDriver *driver)
{
	// Disable the VertexProgram
	driver->activeVertexProgram(NULL);
}

// ***************************************************************************
float	CMeshVPWindTree::getMaxVertexMove()
{
	return _MaxVertexMove;
}


} // NL3D
