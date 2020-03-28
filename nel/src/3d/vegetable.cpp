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

#include "nel/3d/vegetable.h"
#include "nel/misc/common.h"
#include "nel/3d/vegetable_manager.h"
#include "nel/misc/fast_floor.h"


using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NL3D
{


// ***************************************************************************
// Generate random value, but seed is spacial. Take a high frequency, so it gets more the aspect of random.
static	NLMISC::CNoiseValue		RandomGenerator(0,1, 7.68f);


// ***************************************************************************
CVegetable::CVegetable()
{
	// Ground style density.
	setAngleGround(0);

	// Density not maximised.
	MaxDensity= -1;

	// No scale.
	Sxy.Abs= Sz.Abs= 1;
	Sxy.Rand= Sz.Rand= 0;
	// No rotation.
	Rx.Abs= Ry.Abs= Rz.Abs= 0;
	Rx.Rand= Ry.Rand= Rz.Rand= 0;
	// No BendFactor.
	BendFactor.Abs= 1;
	BendFactor.Rand= 0;
	BendFrequencyFactor= 1;

	// Appear at 0.
	DistType= 0;

	_Manager= NULL;
}


// ***************************************************************************
void	CVegetable::setAngleGround(float cosAngleMin)
{
	_AngleType= AngleGround;

	_CosAngleMin= cosAngleMin;
	// We must be at densityFactor==1, when cosAngle==1, keeping the same formula.
	_CosAngleMax= 1 + (1-cosAngleMin);

	// precalc
	_CosAngleMiddle= (_CosAngleMin + _CosAngleMax)/2;
	_OOCosAngleDist= _CosAngleMax - _CosAngleMiddle;
	if(_OOCosAngleDist)
		_OOCosAngleDist= 1.0f / _OOCosAngleDist;
}

// ***************************************************************************
void	CVegetable::setAngleCeiling(float cosAngleMax)
{
	_AngleType= AngleCeiling;

	_CosAngleMax= cosAngleMax;
	// We must be at densityFactor==1, when cosAngle==-1, keeping the same formula.
	_CosAngleMin= -1 - (cosAngleMax-(-1));

	// precalc
	_CosAngleMiddle= (_CosAngleMin + _CosAngleMax)/2;
	_OOCosAngleDist= _CosAngleMax - _CosAngleMiddle;
	if(_OOCosAngleDist)
		_OOCosAngleDist= 1.0f / _OOCosAngleDist;
}

// ***************************************************************************
void	CVegetable::setAngleWall(float cosAngleMin, float cosAngleMax)
{
	_AngleType= AngleWall;

	_CosAngleMin= cosAngleMin;
	_CosAngleMax= cosAngleMax;

	// precalc
	_CosAngleMiddle= (_CosAngleMin + _CosAngleMax)/2;
	_OOCosAngleDist= _CosAngleMax - _CosAngleMiddle;
	if(_OOCosAngleDist)
		_OOCosAngleDist= 1.0f / _OOCosAngleDist;
}


// ***************************************************************************
void	CVegetable::registerToManager(CVegetableManager *manager)
{
	nlassert(manager);
	_Manager= manager;
	_VegetableShape= _Manager->getVegetableShape(ShapeName);
}


// ***************************************************************************
void	CVegetable::generateGroupEx(float nbInst, const CVector &posInWorld, const CVector &surfaceNormal, uint vegetSeed, std::vector<CVector2f> &instances) const
{
	nlassert(_Manager);

	// Density modulation.
	//===================

	// compute cos of angle between surfaceNormal and K(0,0,1).
	float	cosAngle= surfaceNormal.z;
	// compute angleFactor density. Use a quadratic, because f'(_CosAngleMiddle)==0.
	float	angleFact= 1 - sqr((cosAngle - _CosAngleMiddle) * _OOCosAngleDist);
	angleFact= max(0.f, angleFact);
	// modulate density with angleFactor.
	nbInst*= angleFact;

	// modulate result by Global Manager density
	nbInst*= _Manager->getGlobalDensity();

	// Now, 0<=nbInst<+oo. If we have 0.1, it means that we have 10% chance to spawn an instance.
	// So add a "random" value (with help of a noise with High frequency)
	// if nbInst==0, we should never have any instance (which may arise if evalOneLevelRandom()==1).
	// hence the 0.99f* which ensure that we do nbInst+= [0..1[.
	nbInst+= 0.99f * RandomGenerator.evalOneLevelRandom(posInWorld);

	// and then get only the integral part.
	sint	nbInstances= NLMISC::OptFastFloor(nbInst);
	nbInstances= max(0, nbInstances);

	// resize the instances
	instances.resize(nbInstances);

	// Position generation.
	//===================
	// For now, generate them randomly.
	static CVector2f	dSeed(0.513f, 0.267f);	// random values.
	CVector				seed= posInWorld;
	seed.z+= vegetSeed * 0.723f;	// 0.723f is a random value.
	for(sint i=0; i<nbInstances; i++)
	{
		instances[i].x= RandomGenerator.evalOneLevelRandom(seed);
		seed.x+= dSeed.x;
		instances[i].y= RandomGenerator.evalOneLevelRandom(seed);
		seed.y+= dSeed.y;
	}
}


// ***************************************************************************
void	CVegetable::generateGroup(const CVector &posInWorld, const CVector &surfaceNormal, float area, uint vegetSeed, std::vector<CVector2f> &instances) const
{
	// number of instances to generate
	float	dens= Density.eval(posInWorld);
	if(MaxDensity >= 0)
		dens= min(dens, MaxDensity);
	float	nbInst= area * dens;

	// modulate by normal and generate them.
	generateGroupEx(nbInst, posInWorld, surfaceNormal, vegetSeed, instances);
}


// ***************************************************************************
void	CVegetable::generateGroupBiLinear(const CVector &posInWorld, const CVector posInWorldBorder[4], const CVector &surfaceNormal, float area, uint vegetSeed, std::vector<CVector2f> &instances) const
{
	sint	i;
	const	float evenDistribFact= 12.25f;		// an arbitrary value to have a higher frequency for random.

	// compute how many instances to generate on borders of the patch
	// ==================
	float	edgeDensity[4];
	for(i=0; i<4; i++)
	{
		// Get number of instances generated on edges
		edgeDensity[i]= area * Density.eval(posInWorldBorder[i]);
		if(MaxDensity >= 0)
			edgeDensity[i]= min(edgeDensity[i], area * MaxDensity);
		edgeDensity[i]= max(0.f, edgeDensity[i]);
	}
	// Average on center of the patch for each direction.
	float	edgeDensityCenterX;
	float	edgeDensityCenterY;
	edgeDensityCenterX= 0.5f * (edgeDensity[0] + edgeDensity[1]);
	edgeDensityCenterY= 0.5f * (edgeDensity[2] + edgeDensity[3]);


	// Average for all the patch
	float	nbInstAverage= 0.5f * (edgeDensityCenterX + edgeDensityCenterY);


	// generate instances on the patch
	// ==================
	generateGroupEx(nbInstAverage, posInWorld, surfaceNormal, vegetSeed, instances);



	// move instances x/y to follow edge repartition
	// ==================
	// If on a direction, both edges are 0 density, then must do a special formula
	bool	middleX= edgeDensityCenterX<=1;
	bool	middleY= edgeDensityCenterY<=1;
	float	OOEdgeDCX=0.0;
	float	OOEdgeDCY=0.0;
	if(!middleX)	OOEdgeDCX= 1.0f / edgeDensityCenterX;
	if(!middleY)	OOEdgeDCY= 1.0f / edgeDensityCenterY;
	// for all instances
	for(i=0; i<(sint)instances.size(); i++)
	{
		float		x= instances[i].x;
		float		y= instances[i].y;
		// a seed for random.
		CVector		randSeed(x*evenDistribFact, y*evenDistribFact, 0);

		// X change.
		if(middleX)
		{
			// instances are grouped at middle. this is the bijection of easeInEaseOut
			x= x+x - easeInEaseOut(x);
			x= x+x - easeInEaseOut(x);
			instances[i].x= x;
		}
		else
		{
			// Swap X, randomly. swap more on border
			// evaluate the density in X direction we have at this point.
			float	densX= edgeDensity[0]*(1-x) + edgeDensity[1]* x ;
			// If on the side of the lowest density
			if(densX < edgeDensityCenterX)
			{
				// may swap the position
				float	rdSwap= (densX * OOEdgeDCX );
				// (densX * OOEdgeDCX) E [0..1[. The more it is near 0, the more is has chance to be swapped.
				rdSwap+= RandomGenerator.evalOneLevelRandom( randSeed );
				if(rdSwap<1)
					instances[i].x= 1 - instances[i].x;
			}
		}

		// Y change.
		if(middleY)
		{
			// instances are grouped at middle. this is the bijection of easeInEaseOut
			y= y+y - easeInEaseOut(y);
			y= y+y - easeInEaseOut(y);
			instances[i].y= y;
		}
		else
		{
			// Swap Y, randomly. swap more on border
			// evaluate the density in Y direction we have at this point.
			float	densY= edgeDensity[2]*(1-y) + edgeDensity[3]* y ;
			// If on the side of the lowest density
			if(densY < edgeDensityCenterY)
			{
				// may swap the position
				float	rdSwap= (densY * OOEdgeDCY);
				// (densY * OOEdgeDCY) E [0..1[. The more it is near 0, the more is has chance to be swapped.
				rdSwap+= RandomGenerator.evalOneLevelRandom( randSeed );
				if(rdSwap<1)
					instances[i].y= 1 - instances[i].y;
			}
		}

	}

}


// ***************************************************************************
void	CVegetable::reserveIgAddInstances(CVegetableInstanceGroupReserve &vegetIgReserve, TVegetableWater vegetWaterState, uint numInstances) const
{
	nlassert(_Manager);

	if (_VegetableShape)
		_Manager->reserveIgAddInstances(vegetIgReserve, _VegetableShape, (CVegetableManager::TVegetableWater)vegetWaterState, numInstances);
}


// ***************************************************************************
void	CVegetable::generateInstance(CVegetableInstanceGroup *ig, const NLMISC::CMatrix &posInWorld,
		const NLMISC::CRGBAF &modulateAmbientColor, const NLMISC::CRGBAF &modulateDiffuseColor, float blendDistMax,
		TVegetableWater vegetWaterState, CVegetableUV8 dlmUV) const
{
	nlassert(_Manager);


	CVector		seed= posInWorld.getPos();

	// Generate Matrix.
	// ===============

	// Generate a random Scale / Rotation matrix.
	CMatrix		randomMat;
	// setup rotation
	CVector		rot;
	rot.x= Rx.eval(seed);
	rot.y= Ry.eval(seed);
	rot.z= Rz.eval(seed);
	randomMat.setRot(rot, CMatrix::ZXY);
	// scale.
	if(Sxy.Abs!=0 || Sxy.Rand!=0 || Sz.Abs!=0 || Sz.Rand!=0)
	{
		CVector		scale;
		scale.x= scale.y= Sxy.eval(seed);
		scale.z= Sz.eval(seed);
		randomMat.scale(scale);
	}

	// Final Matrix.
	CMatrix		finalMatrix;
	finalMatrix= posInWorld * randomMat;

	// Generate Color and factor
	// ===============
	CRGBAF		materialColor(1,1,1,1);
	// evaluate gradients. If none, color not modified.
	Color.eval(seed, materialColor);
	// modulate with user
	CRGBAF		ambient, diffuse;
	if(_VegetableShape && _VegetableShape->Lighted)
	{
		ambient= modulateAmbientColor * materialColor;
		diffuse= modulateDiffuseColor * materialColor;
	}
	else
	{
		ambient= materialColor;
		diffuse= materialColor;
	}

	// Generate a bendFactor
	float	bendFactor= BendFactor.eval(seed);
	// Generate a bendPhase
	float	bendPhase= BendPhase.eval(seed);


	// Append to the vegetableManager
	// ===============
	if (_VegetableShape)
	{
		_Manager->addInstance(ig, _VegetableShape, finalMatrix, ambient, diffuse,
			bendFactor, bendPhase, BendFrequencyFactor, blendDistMax,
			(CVegetableManager::TVegetableWater)vegetWaterState, dlmUV);
	}
}


// ***************************************************************************
void	CVegetable::serial(NLMISC::IStream &f)
{
	/*
	Version 1:
		- add BendFrequencyFactor
	Version 0:
		- base version
	*/
	sint	ver= f.serialVersion(1);

	f.serial(ShapeName);
	f.serial(Density);
	f.serial(MaxDensity);
	f.serial(_CosAngleMin, _CosAngleMax, _CosAngleMiddle, _OOCosAngleDist);
	f.serialEnum(_AngleType);
	f.serial(Sxy, Sz);
	f.serial(Rx, Ry, Rz);
	f.serial(BendFactor);
	f.serial(BendPhase);
	f.serial(Color);
	f.serial(DistType);

	if(ver>=1)
		f.serial(BendFrequencyFactor);
	else
		BendFrequencyFactor= 1;
}


} // NL3D
