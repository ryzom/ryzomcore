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

#include "nel/3d/water_shape.h"
#include "nel/3d/water_model.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/texture_bump.h"
#include "nel/3d/texture_blend.h"
#include "nel/3d/scene.h"
#include "nel/3d/water_pool_manager.h"
#include "nel/3d/water_height_map.h"
#include <memory>


namespace NL3D {




// globals


/////////////////////////
// WATER WITH NO WAVES //
/////////////////////////

static const char *WaterVPNoWave =
"!!VP1.0                                                                        \n\
 DP4 o[HPOS].x, c[0], v[0];	          #transform vertex in view space	        \n\
 DP4 o[HPOS].y, c[1], v[0];												        \n\
 DP4 o[HPOS].z, c[2], v[0];												        \n\
 DP4 o[HPOS].w, c[3], v[0];												        \n\
 # MUL R1, R2.x, R1;                                                            \n\
 DP4 o[FOGC].x, c[4], v[0];	      #setup fog					                \n\
 MUL R3, v[0], c[5];			      #compute bump 0 uv's			            \n\
 ADD o[TEX0], R3, c[6];										                    \n\
 MUL R3, v[0], c[7];			      #compute bump 1 uv's			            \n\
 ADD o[TEX1], R3, c[8];											                \n\
 ADD R0, c[9], -v[0];			      #r1 = eye - vertex						\n\
 DP3 R1, R0, R0;				      #r1 = eye - vertex, r2 = (eye - vertex)^2	\n\
 RSQ R1, R1.x;					      #r1 = eye - vertex, r2 = 1/d(eye, vertex)	\n\
 MUL R0, R0, R1;                                                                \n\
 DP3 R1.x, R0.xyww, R0.xyww;                                                    \n\
 MAD o[TEX2], -R0, c[10], c[10];		  #envmap tex coord						\n\
 END";

// a diffuse texture is added
static const char *WaterVPNoWaveDiffuse =
"!!VP1.0\n\
DP4 o[HPOS].x, c[0], v[0];	          #transform vertex in view space	        \n\
DP4 o[HPOS].y, c[1], v[0];												        \n\
DP4 o[HPOS].z, c[2], v[0];												        \n\
DP4 o[HPOS].w, c[3], v[0];												        \n\
# MUL R1, R2.x, R1;                                                             \n\
DP4 o[FOGC].x, c[4], v[0];	          #setup fog					            \n\
MUL R3, v[0], c[5];			          #compute bump 0 uv's			            \n\
ADD o[TEX0], R3, c[6];										                    \n\
MUL R3, v[0], c[7];			          #compute bump 1 uv's			            \n\
ADD o[TEX1], R3, c[8];											                \n\
ADD R0, c[9], -v[0];			      #r1 = eye - vertex						\n\
DP3 R1, R0, R0;				          #r1 = eye - vertex, r2 = (eye - vertex)^2	\n\
RSQ R1, R1.x;					      #r1 = eye - vertex, r2 = 1/d(eye, vertex)	\n\
MUL R0, R0, R1;                                                                 \n\
MAD o[TEX2], -R0, c[10], c[10];		  #envmap tex coord							\n\
DP4 o[TEX3].x, v[0], c[11];           #compute uv for diffuse texture			\n\
DP4 o[TEX3].y, v[0], c[12];													    \n\
END";



////////////////
// WAVY WATER //
////////////////

// common start for Water vertex programs
/** The first part of the program does the following :
  * - Compute linear distance to eye
  * - Attenuate height with distance
  * - Attenuate normal with distance (e.g at max distance, the normal is (0, 0, 1)
  * - Transform vertex pos into view space
  * - compute fog coordinate
  * At the end of the program we got :
  * R1 = (eye - vertex).normed()
  * R0 = Attenuated normal at vertex
  * R4 = position of point with attenuated height
  */

static const char *WaterVPStartCode =
"!!VP1.0\n\
	ADD R1, c[7], -v[0];			      #r1 = eye - vertex						\n\
	DP3 R2, R1, R1;				          #r1 = eye - vertex, r2 = (eye - vertex)^2	\n\
	MAX R2, R2, c[16];                    # avoid imprecision around 0				\n\
	RSQ R2, R2.x;					      #r1 = eye - vertex, r2 = 1/d(eye, vertex)	\n\
	RCP R3, R2.x;																    \n\
	MAD R3, c[6].xxxx, -R3, c[6].yyyy;											    \n\
	MAX R3, c[5],	R3;															    \n\
	MUL R0, R3, v[8];			          #attenuate normal with distance		    \n\
	MUL R4.z,   R3, v[0];			      #attenuate height with distance 	        \n\
	MOV R4.xyw, v[0];														 	    \n\
	MOV R0.z,  c[4].x;			          #set normal z to 1 				        \n\
	DP3 R3.x, R0, R0;															    \n\
	RSQ R3.x,  R3.x;				      #normalize normal in R3			        \n\
	MUL R0,  R0, R3.x;															    \n\
	DP4 o[HPOS].x, c[0], R4;	          #transform vertex in view space	        \n\
	DP4 o[HPOS].y, c[1], R4;												        \n\
	DP4 o[HPOS].z, c[2], R4;												        \n\
	DP4 o[HPOS].w, c[3], R4;												        \n\
	MUL R1, R1, R2.x;		              #normalize r1, r1 = (eye - vertex).normed \n\
	# DP3 R2.x, R1.xyww, R1.xyww;                                                     \n\
	# MUL R1, R2.x, R1;                                                             \n\
	DP4 o[FOGC].x, c[18], R4;	          #setup fog					            \n\
";
/** This part of vertex program compute 2 layers of bump (for use with texture shaders)
  */
static const char *WaterVpBump2LayersCode =
"	MUL R3, v[0], c[10];			#compute bump 0 uv's			\n\
	ADD o[TEX0], R3, c[9];										    \n\
	MUL R3, v[0], c[12];			#compute bump 1 uv's			\n\
	ADD o[TEX1], R3, c[11];											\n\
	DP3 R2.x, R1, R0;												\n\
	MUL R0, R0, R2.x;												\n\
	ADD R2, R0, R0;													\n\
	ADD R0, R2, -R1;				#compute reflection vector		\n\
	MAD o[TEX2], R0, c[8], c[8];									\n\
";
/** Version with one bump map only (Texture shaders support chaining of offset textures, EMBM does not)
  */
static const char *WaterVpBump1LayersCode =
"MUL R3, v[0], c[12];			#compute bump 1 uv's				\n\
 ADD o[TEX0], R3, c[11];										    \n\
 DP3 R2.x, R1, R0;													\n\
 MUL R0, R0, R2.x;													\n\
 ADD R2, R0, R0;													\n\
 ADD R0, R2, -R1;				#compute reflection vector			\n\
 MAD o[TEX1], R0, c[8], c[8];									    \n\
";
/** Optional diffuse texture in stage 3
  */
static const char *WaterVpDiffuseMapStage3Code =
"DP4 o[TEX3].x, R4, c[13]; #compute uv for diffuse texture					\n\
 DP4 o[TEX3].y, R4, c[14];													\n\
";
/** Optional diffuse texture in stage 2
  */
static const char *WaterVpDiffuseMapStage2Code =
"DP4 o[TEX2].x, R4, c[13]; #compute uv for diffuse texture					\n\
 DP4 o[TEX2].y, R4, c[14];													\n\
";

/** Optional diffuse texture in stage 1
  */
static const char *WaterVpDiffuseMapStage1Code =
"DP4 o[TEX1].x, R4, c[13]; #compute uv for diffuse texture					\n\
 DP4 o[TEX1].y, R4, c[14];													\n\
";

// Envmap is setup in texture 0, no bump is used
static const char *WaterVpNoBumpCode =
"  DP3 R2.x, R1, R0;			 #project view vector on normal for symetry	\n\
   MUL R0, R0, R2.x;														\n\
   ADD R2, R0, R0;															\n\
   ADD R0, R2, -R1;				 #compute reflection vector					\n\
   MAD o[TEX0], R0, c[8], c[8];											    \n\
   DP4 o[FOGC].x, c[18], R4;	 #setup fog									\n\
";


// static members
uint32									CWaterShape::_XScreenGridSize = 20;
uint32									CWaterShape::_YScreenGridSize = 20;
//
uint32									CWaterShape::_XGridBorder = 4;
uint32									CWaterShape::_YGridBorder = 4;
uint32									CWaterShape::_MaxGridSize;
bool									CWaterShape::_GridSizeTouched = true;
NLMISC::CSmartPtr<CVertexProgram>		CWaterShape::_VertexProgramBump1;
NLMISC::CSmartPtr<CVertexProgram>		CWaterShape::_VertexProgramBump2;
NLMISC::CSmartPtr<CVertexProgram>		CWaterShape::_VertexProgramBump1Diffuse;
NLMISC::CSmartPtr<CVertexProgram>		CWaterShape::_VertexProgramBump2Diffuse;
NLMISC::CSmartPtr<CVertexProgram>		CWaterShape::_VertexProgramNoBump;
NLMISC::CSmartPtr<CVertexProgram>		CWaterShape::_VertexProgramNoBumpDiffuse;
// water with no waves
NLMISC::CSmartPtr<CVertexProgram>		CWaterShape::_VertexProgramNoWave;
NLMISC::CSmartPtr<CVertexProgram>		CWaterShape::_VertexProgramNoWaveDiffuse;


/** Build a vertex program for water depending on requirements
  */
static CVertexProgram *BuildWaterVP(bool diffuseMap, bool bumpMap, bool use2BumpMap)
{
	std::string vp = WaterVPStartCode;
	if (bumpMap && use2BumpMap)
	{
		vp += WaterVpBump2LayersCode;
		if (diffuseMap) vp += WaterVpDiffuseMapStage3Code;
	}
	else
	if (bumpMap)
	{
		vp += WaterVpBump2LayersCode;
		if (diffuseMap) vp += WaterVpDiffuseMapStage2Code;
	}
	else
	{
		vp += WaterVpNoBumpCode;
		if (diffuseMap) vp += WaterVpDiffuseMapStage1Code;
	}

	vp += "\nEND";
	return new CVertexProgram(vp.c_str());
	// TODO_VP_GLSL
}



//============================================
/*
 * Constructor
 */
CWaterShape::CWaterShape() :  _WaterPoolID(0), _TransitionRatio(0.6f), _WaveHeightFactor(3), _ComputeLightmap(false), _SplashEnabled(true)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	_DefaultPos.setDefaultValue(NLMISC::CVector::Null);
	_DefaultScale.setDefaultValue(NLMISC::CVector(1, 1, 1));
	_DefaultRotQuat.setDefaultValue(CQuat::Identity);

	for (sint k = 0; k < 2; ++k)
	{
		_HeightMapScale[k].set(1, 1);
		_HeightMapSpeed[k].set(0, 0);
		_HeightMapTouch[k] = true;
		_UsesSceneWaterEnvMap[k] = false;
	}
	_ColorMapMatColumn0.set(1, 0);
	_ColorMapMatColumn1.set(0, 1);
	_ColorMapMatPos.set(0, 0);
	_EnvMapMeanColorComputed = false;
}

//============================================
CRGBA CWaterShape::computeEnvMapMeanColor()
{
	// TMP :
	// just used for water rendering in multiple parts with parallel projection
	// -> drawn as an uniform polygon with envmap mean coloe
	if (!_EnvMapMeanColorComputed)
	{
		_EnvMapMeanColor = NLMISC::CRGBA(0, 0, 255);
		if (_EnvMap[0])
		{
			_EnvMap[0]->generate();
			_EnvMap[0]->convertToType(CBitmap::RGBA);
			uint32 r = 0;
			uint32 g = 0;
			uint32 b = 0;
			uint32 a = 0;
			uint numPixs = _EnvMap[0]->getHeight() * _EnvMap[0]->getWidth();
			const CRGBA *src = (const CRGBA *) (&_EnvMap[0]->getPixels(0)[0]);
			const CRGBA *last = src + numPixs;
			while (src != last)
			{
				r += src->R;
				g += src->G;
				b += src->B;
				a += src->A;
				++ src;
			}
			if (numPixs != 0)
			{
				_EnvMapMeanColor = NLMISC::CRGBA((uint8) (r / numPixs),
												 (uint8) (g / numPixs),
												 (uint8) (b / numPixs),
												 (uint8) (a / numPixs));
			}
			_EnvMap[0]->release();
		}
		_EnvMapMeanColorComputed = true;
	}
	return _EnvMapMeanColor;
}

//============================================
CWaterShape::~CWaterShape()
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	if (
		(_EnvMap[0] && dynamic_cast<CTextureBlend *>((ITexture *) _EnvMap[0]))
		|| (_EnvMap[1] && dynamic_cast<CTextureBlend *>((ITexture *) _EnvMap[1]))
		)
	{
		GetWaterPoolManager().unRegisterWaterShape(this);
	}
}

//============================================
void CWaterShape::initVertexProgram()
{
	static bool created = false;
	if (!created)
	{
		// waves
		_VertexProgramBump1 = BuildWaterVP(false, true, false);
		_VertexProgramBump2 = BuildWaterVP(false, true, true);

		_VertexProgramBump1Diffuse = BuildWaterVP(true, true, false);
		_VertexProgramBump2Diffuse = BuildWaterVP(true, true, true);

		_VertexProgramNoBump = BuildWaterVP(false, false, false);
		_VertexProgramNoBumpDiffuse = BuildWaterVP(true, false, false);
		// no waves
		_VertexProgramNoWave = new CVertexProgram(WaterVPNoWave); // TODO_VP_GLSL
		_VertexProgramNoWaveDiffuse = new CVertexProgram(WaterVPNoWaveDiffuse); // TODO_VP_GLSL
		created = true;
	}
}

//============================================
CTransformShape		*CWaterShape::createInstance(CScene &scene)
{
	CWaterModel *wm = NLMISC::safe_cast<CWaterModel *>(scene.createModel(WaterModelClassId) );
	wm->Shape = this;
	// set default pos & scale
	wm->ITransformable::setPos( _DefaultPos.getDefaultValue() );
	wm->ITransformable::setScale( _DefaultScale.getDefaultValue() );
	wm->ITransformable::setRotQuat( _DefaultRotQuat.getDefaultValue() );
	//
	wm->init();
	if (scene.getWaterCallback())
	{
		CWaterShape *ws = NLMISC::safe_cast<CWaterShape *>((IShape *) wm->Shape);
		scene.getWaterCallback()->waterSurfaceAdded(getShape(), wm->getMatrix(), ws->isSplashEnabled(), ws->getUseSceneWaterEnvMap(0) || ws->getUseSceneWaterEnvMap(1));
	}
	return wm;
}

//============================================
float CWaterShape::getNumTriangles (float distance)
{
	// TODO
	return 0;
}

//============================================
void CWaterShape::flushTextures (IDriver &driver, uint selectedTexture)
{
	// Test if bump maps are supported by driver before to flush them.
	// TEMP : can't flush texture for water, because the upload format depends on the shader
	// Only the driver can determine it.
	// BumpMaps may be uploaded with unsigned or signed format

	/*
	if (
		(driver.supportTextureShaders() && driver.supportTextureAddrMode(CMaterial::OffsetTexture))
		|| driver.supportEMBM()
	   )
	{
		for (uint k = 0; k < 2; ++k)
		{
			if (_BumpMap[k] != NULL)
				driver.setupTexture(*_BumpMap[k]);
			if (_EnvMap[k] != NULL)
				driver.setupTexture(*_EnvMap[k]);
		}
	}
	if (_ColorMap != NULL)
		driver.setupTexture(*_ColorMap);
	*/
}

//============================================
void	CWaterShape::setScreenGridSize(uint32 x, uint32 y)
{
	nlassert(x > 0 && y > 0);
	_XScreenGridSize = x;
	_YScreenGridSize = y;
	_GridSizeTouched = true;
}

//============================================
void		CWaterShape::setGridBorderSize(uint32 x, uint32 y)
{
	_XGridBorder = x;
	_YGridBorder = y;
	_GridSizeTouched = true;
}

//============================================
void CWaterShape::setShape(const NLMISC::CPolygon2D &poly)
{
	nlassert(poly.Vertices.size() != 0); // empty poly not allowed
	_Poly = poly;
	computeBBox();
}

//============================================
void CWaterShape::computeBBox()
{
	nlassert(_Poly.Vertices.size() != 0);
	NLMISC::CVector2f min, max;
	min = max = _Poly.Vertices[0];
	for (uint k = 1; k < _Poly.Vertices.size(); ++k)
	{
		min.minof(min, _Poly.Vertices[k]);
		max.maxof(max, _Poly.Vertices[k]);
	}
	_BBox.setMinMax(CVector(min.x, min.y, 0), CVector(max.x, max.y, 0));
	/* nlinfo("center x = %f, y = %f, z = %f", _BBox.getCenter().x, _BBox.getCenter().y, _BBox.getCenter().z);
	nlinfo("halsize x = %f, y = %f, z = %f", _BBox.getHalfSize().x, _BBox.getHalfSize().y, _BBox.getHalfSize().z); */
}

//============================================
void				CWaterShape::setHeightMap(uint k, ITexture *hm)
{
	nlassert(k < 2);
	if (!_BumpMap[k])
	{
		_BumpMap[k] = new CTextureBump;
	}
	static_cast<CTextureBump *>( (ITexture *) _BumpMap[k])->forceNormalize(true);
	static_cast<CTextureBump *>( (ITexture *) _BumpMap[k])->setHeightMap(hm);
	_HeightMapTouch[k] = true; // must recompute normalization factor
}

//============================================
ITexture			*CWaterShape::getHeightMap(uint k)
{
	nlassert(k < 2);
	return ((CTextureBump *) (ITexture *) _BumpMap[k] )->getHeightMap();
}

//============================================
const ITexture		*CWaterShape::getHeightMap(uint k) const
{
	nlassert(k < 2);
	return ((CTextureBump *) (ITexture *) _BumpMap[k] )->getHeightMap();
}

//============================================
void CWaterShape::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	/* ***********************************************
	 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
	 *	It can be loaded/called through CAsyncFileManager for instance
	 * ***********************************************/

	// version 4 : added scene water env map
	// version 3 : added '_Splashenabled' flag
	sint ver = f.serialVersion(4);
	// serial 'shape'
	f.serial(_Poly);
	// serial heightMap identifier
	f.serial(_WaterPoolID);
	//serial maps
	ITexture *map = NULL;
	if (f.isReading())
	{
		f.serialPolyPtr(map); _EnvMap[0] = map;
		f.serialPolyPtr(map); _EnvMap[1] = map;
		f.serialPolyPtr(map); _BumpMap[0] = map;
		f.serialPolyPtr(map); _BumpMap[1] = map;
		f.serialPolyPtr(map); _ColorMap = map;
		computeBBox();
	}
	else
	{
		map = _EnvMap[0]; f.serialPolyPtr(map);
		map = _EnvMap[1]; f.serialPolyPtr(map);
		map = _BumpMap[0]; f.serialPolyPtr(map);
		map = _BumpMap[1]; f.serialPolyPtr(map);
		map = _ColorMap; f.serialPolyPtr(map);
	}

	f.serial(_HeightMapScale[0], _HeightMapScale[1],
			 _HeightMapSpeed[0], _HeightMapSpeed[1]);

	f.serial(_ColorMapMatColumn0, _ColorMapMatColumn1, _ColorMapMatPos);

	// serial default tracks
	f.serial(_DefaultPos);
	f.serial(_DefaultScale);
	f.serial(_DefaultRotQuat);

	f.serial(_TransitionRatio);

	f.serial(_WaveHeightFactor);

	if (ver >= 1)
		f.serial (_ComputeLightmap);

	if (ver >= 2)
		f.serial (_DistMax);

	if (ver >= 3)
		f.serial(_SplashEnabled);

	if (ver >= 4)
	{
		f.serial(_UsesSceneWaterEnvMap[0], _UsesSceneWaterEnvMap[1]);
	}

	// tmp
	/*
	if (f.isReading())
	{
		_UsesSceneWaterEnvMap[0] = true;
		_UsesSceneWaterEnvMap[1] = true;
	}
	*/
}

//============================================
bool CWaterShape::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{
	for (uint k = 0; k < pyramid.size(); ++k)
	{
		if (! _BBox.clipBack(pyramid[k] * worldMatrix)) return false;
	}
	return true;
}

//============================================
void				CWaterShape::setHeightMapScale(uint k, const NLMISC::CVector2f &scale)
{
	nlassert(k < 2);
	_HeightMapScale[k] = scale;
}

//============================================
NLMISC::CVector2f	CWaterShape::getHeightMapScale(uint k) const
{
	nlassert(k < 2);
	return _HeightMapScale[k];
}

//============================================
void			    CWaterShape::setHeightMapSpeed(uint k, const NLMISC::CVector2f &speed)
{
	nlassert(k < 2);
	_HeightMapSpeed[k] = speed;
}

//============================================
NLMISC::CVector2f   CWaterShape::getHeightMapSpeed(uint k) const
{
	nlassert(k < 2);
	return _HeightMapSpeed[k];
}

//============================================
void	CWaterShape::setColorMapMat(const NLMISC::CVector2f &column0, const NLMISC::CVector2f &column1, const NLMISC::CVector2f &pos)
{
	_ColorMapMatColumn0 = column0;
	_ColorMapMatColumn1 = column1;
	_ColorMapMatPos  = pos;
}

//============================================
void	CWaterShape::getColorMapMat(NLMISC::CVector2f &column0, NLMISC::CVector2f &column1, NLMISC::CVector2f &pos)
{
	column0 = _ColorMapMatColumn0;
	column1 = _ColorMapMatColumn1;
	pos  = _ColorMapMatPos;
}

//============================================
void CWaterShape::envMapUpdate()
{
	// if the color map is a blend texture, we MUST be registered to the water pool manager, so that, the
	// setBlend message will be routed to this texture.
	if (
		(_EnvMap[0] && dynamic_cast<CTextureBlend *>((ITexture *) _EnvMap[0]))
		|| (_EnvMap[1] && dynamic_cast<CTextureBlend *>((ITexture *) _EnvMap[1]))
		)
	{
		if (!GetWaterPoolManager().isWaterShapeObserver(this))
		{
			GetWaterPoolManager().registerWaterShape(this);
		}
	}
	else
	{
		if (GetWaterPoolManager().isWaterShapeObserver(this))
		{
			GetWaterPoolManager().unRegisterWaterShape(this);
		}
	}
}

//============================================
void CWaterShape::setColorMap(ITexture *map)
{
	_ColorMap = map;
	//colorMapUpdate();
}

//============================================
void CWaterShape::setEnvMap(uint index, ITexture *envMap)
{
	nlassert(index < 2);
	_EnvMap[index] = envMap;
}

//============================================
void CWaterShape::getShapeInWorldSpace(NLMISC::CPolygon &poly) const
{
	poly.Vertices.resize(_Poly.Vertices.size());
	// compute the matrix of the object in world space, by using the default tracks
	NLMISC::CMatrix objMat;
	objMat.identity();
	objMat.translate(_DefaultPos.getDefaultValue());
	objMat.rotate(_DefaultRotQuat.getDefaultValue());
	objMat.scale(_DefaultScale.getDefaultValue());

	for (uint k = 0; k < _Poly.Vertices.size(); ++k)
	{
		poly.Vertices[k] = objMat * NLMISC::CVector(_Poly.Vertices[k].x, _Poly.Vertices[k].y, 0);
	}
}


//============================================
void CWaterShape::getShapeInWorldSpace(NLMISC::CPolygon &poly, const NLMISC::CMatrix &objMat) const
{
	poly.Vertices.resize(_Poly.Vertices.size());
	for (uint k = 0; k < _Poly.Vertices.size(); ++k)
	{
		poly.Vertices[k] = objMat * NLMISC::CVector(_Poly.Vertices[k].x, _Poly.Vertices[k].y, 0);
	}
}


//============================================
void  CWaterShape::updateHeightMapNormalizationFactors()
{
	for (uint k = 0; k < 2; ++k)
	{
		if (_HeightMapTouch[k])
		{
			if (_BumpMap[k] != NULL)
			{
				_BumpMap[k]->generate();
				_HeightMapNormalizationFactor[k] = NLMISC::safe_cast<CTextureBump *>((ITexture *)_BumpMap[k])->getNormalizationFactor();
				if (_BumpMap[k]->getReleasable())
				{
					_BumpMap[k]->release();
				}
			}
			else
			{
				_HeightMapNormalizationFactor[k] = 1.f;
			}
			_HeightMapTouch[k] = false;
		}
	}
}



//======================================================//
//						WaveMakerShape					//
//======================================================//


//============================================
CWaveMakerShape::CWaveMakerShape() : _Period(1),
				     _Radius(3),
				     _PoolID(0),
				     _Intensity(1),
				     _ImpulsionMode(true)
{
}

//============================================
CWaveMakerShape::~CWaveMakerShape()
{
}

//============================================
void CWaveMakerShape::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(0);
	f.serial(_Period, _Radius, _Intensity, _PoolID, _ImpulsionMode);
}

//============================================
CTransformShape		*CWaveMakerShape::createInstance(CScene &scene)
{
	CWaveMakerModel *wmm = NLMISC::safe_cast<CWaveMakerModel *>(scene.createModel(WaveMakerModelClassId) );
	wmm->Shape = this;
	// set default pos & scale
	wmm->ITransformable::setPos( _DefaultPos.getDefaultValue() );
	return wmm;
}

//============================================
bool	CWaveMakerShape::clip(const std::vector<CPlane>	&pyramid, const CMatrix &worldMatrix)
{
	// we just test if not too far
	const CWaterHeightMap &whm = GetWaterPoolManager().getPoolByID(_PoolID);
	const float maxDist = 0.5f * whm.getUnitSize() * whm.getSize();
	const NLMISC::CVector pos = worldMatrix.getPos();
	for (std::vector<NLMISC::CPlane>::const_iterator it = pyramid.begin(); it != pyramid.end(); ++it)
	{
		if ((*it) * pos > maxDist) return false;
	}
	return true;

}

//============================================
void	CWaveMakerShape::getAABBox(NLMISC::CAABBox &bbox) const
{
	// its just a point
	bbox.setCenter(NLMISC::CVector::Null);
	bbox.setHalfSize(NLMISC::CVector::Null);
}

} // NL3D
