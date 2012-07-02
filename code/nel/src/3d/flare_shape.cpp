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

#include "nel/3d/flare_shape.h"
#include "nel/3d/flare_model.h"
#include "nel/3d/scene.h"
#include "nel/3d/driver.h"
#include "nel/3d/mesh.h"
#include "nel/3d/shape_bank.h"



namespace NL3D {


// ***************************************************************************************************************
CFlareShape::CFlareShape()  : _Color(NLMISC::CRGBA::White),
							  _DazzleColor(NLMISC::CRGBA::Black),
							  _SizeDisappear(0.f),
							  _ScaleWhenDisappear(false),
							  _AngleDisappear(0.f),
							  _Persistence(1),
							  _Spacing(1),
							  _Attenuable(false),
							  _AttenuationRange (1.0f),
							  _FirstFlareKeepSize(false),
							  _DazzleEnabled(false),
							  _DazzleAttenuationRange(0.f),
							  _MaxViewDistRatio (0.9f),
							  _InfiniteDist(false),
							  _OcclusionMeshNotFound(false),
							  _OcclusionTestMeshInheritScaleRot(false),
							  _LookAtMode(true)
{
	// init default pos
	for (uint k = 0; k < MaxFlareNum; ++k)
	{
		_Tex [k]  = NULL;
		_Size[k]  = 1.f;
		_Pos[k]   = k * (1.f / MaxFlareNum);
	}
	_DefaultPos.setDefaultValue(CVector::Null);
	setDistMax(1000);
}


// ***************************************************************************************************************
void CFlareShape::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	// Version 4 : - added occlusion test mesh, size reduction, angle modification when object is occluded
	//             - added lookat mode for first flare
	sint ver = f.serialVersion(5);
	f.serial(_Color, _Persistence, _Spacing);
	f.serial(_Attenuable);
	if (_Attenuable)
	{
		f.serial(_AttenuationRange);
	}
	f.serial(_FirstFlareKeepSize);
	if (f.isReading() && ver <= 4)
	{
		_FirstFlareKeepSize = false;
	}
	for (uint k = 0; k < MaxFlareNum; ++k)
	{
		ITexture *tex = _Tex[k];
		f.serialPolyPtr(tex);
		if (f.isReading())
		{
			_Tex[k] = tex;
		}
		f.serial(_Size[k], _Pos[k]);
	}
	f.serial(_InfiniteDist);
	if (!_InfiniteDist)
	{
		f.serial(_MaxViewDist, _MaxViewDistRatio);
	}
	f.serial(_DazzleEnabled);
	if (_DazzleEnabled)
	{
		f.serial(_DazzleColor, _DazzleAttenuationRange);
	}
	f.serial(_InfiniteDist);
	if (ver >= 2)
	{
		f.serial( _DistMax );
	}
	if (ver >= 4)
	{
		f.serial(_OcclusionTestMeshName);
		f.serial(_ScaleWhenDisappear);
		f.serial(_SizeDisappear);
		f.serial(_AngleDisappear);
		f.serial(_OcclusionTestMeshInheritScaleRot);
		f.serial(_LookAtMode);
	}
}

// ***************************************************************************************************************
CTransformShape		*CFlareShape::createInstance(CScene &scene)
{
	CFlareModel *fm = NLMISC::safe_cast<CFlareModel *>(scene.createModel(FlareModelClassId) );
	fm->Shape = this;
	fm->_Scene = &scene;
	// set default pos
	fm->ITransformable::setPos( _DefaultPos.getDefaultValue() );
	return fm;
}

// ***************************************************************************************************************
float				CFlareShape::getNumTriangles (float distance)
{
	float count = 0;
	for (uint k = 0; k < MaxFlareNum; ++k)
	{
	if (_Tex[k]) count += 2;
	}
	return count;
}

// ***************************************************************************************************************
bool				CFlareShape::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{
	// compute flare pos in world basis :
	const NLMISC::CVector pos = worldMatrix.getPos();
	for (std::vector<NLMISC::CPlane>::const_iterator it = pyramid.begin(); it != pyramid.end(); ++it)
	{
		if ((*it) * pos > _Size[0])
		{
			//nlwarning("clipped");
			return false;
		}
	}
	return true;
}

// ***************************************************************************************************************
void				CFlareShape::getAABBox(NLMISC::CAABBox &bbox) const
{
	// the flare himself is a point
	bbox.setCenter(CVector::Null);
	bbox.setHalfSize(CVector::Null);
}

// ***************************************************************************************************************
void				CFlareShape::flushTextures (IDriver &driver, uint selectedTexture)
{
	// Flush each texture
	for (uint tex=0; tex<MaxFlareNum; tex++)
	{
		if (_Tex[tex] != NULL)
		{
			// Select the good texture
			_Tex[tex]->selectTexture (selectedTexture);

			// Flush texture
			driver.setupTexture (*_Tex[tex]);
		}
	}
}

// ***************************************************************************************************************
void CFlareShape::setOcclusionTestMeshName(const std::string &shapeName)
{
	if (shapeName == _OcclusionTestMeshName) return;
	_OcclusionTestMeshName = shapeName;
	_OcclusionTestMesh = NULL;
}


// ***************************************************************************************************************
CMesh *CFlareShape::getOcclusionTestMesh(CShapeBank &sb)
{
	if (_OcclusionTestMesh) return _OcclusionTestMesh;
	if (_OcclusionMeshNotFound) return NULL;
	if (_OcclusionTestMeshName.empty()) return NULL;
	if (sb.getPresentState(_OcclusionTestMeshName)!=CShapeBank::Present)
	{
		sb.load(_OcclusionTestMeshName);
		if (sb.getPresentState(_OcclusionTestMeshName)!=CShapeBank::Present)
		{
			_OcclusionMeshNotFound = true;
			return NULL;
		}
	}
	IShape *mesh = sb.addRef(_OcclusionTestMeshName);
	if (!mesh)
	{
		_OcclusionMeshNotFound = true;
		return NULL;
	}
	_OcclusionTestMesh = dynamic_cast<CMesh *>(mesh);
	if (!_OcclusionTestMesh)
	{
		_OcclusionMeshNotFound = true;
		nlwarning("%s is not a mesh. Mesh required for occlusion testing", _OcclusionTestMeshName.c_str());
		sb.release(mesh);
		return NULL;
	}
	return _OcclusionTestMesh;
}



} // NL3D
