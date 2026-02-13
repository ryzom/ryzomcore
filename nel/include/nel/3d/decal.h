/** \file decal.h
 * 
 *
 * $Id$
 */

/* Copyright, 2007 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_DECAL_H
#define NL_DECAL_H


#include "nel/3d/transform.h"

#include "nel/3d/material.h"
#include "nel/3d/shadow_map.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/shadow_poly_receiver.h"
#include "nel/misc/polygon.h"


namespace NLMISC
{

class CPlane;

}


namespace NL3D
{

class CScene;
class CShadowMap;
class CShadowPolyReceiver;
class CDecalManager;

class UScene;
class UDriver;


// ***************************************************************************
const NLMISC::CClassId		DecalId=NLMISC::CClassId(0x6a570fe6, 0x32323e16);


// ***************************************************************************
/**
 * Decals are render on mesh
 * \author Christopher Tarento
 * \author Nevrax France
 * \date 2007
 */
class CDecalContext
{
public:
	CDecalContext();

public:
	std::vector<CPlane>		WorldClipPlanes;
	CAABBox					WorldBBox;
	NLMISC::CPolygon2D		Poly2D;
	CMatrix					WorldMatrix;
	std::vector<CVector>	*DestTris;
	bool					Clipping;
};


/**
 * Decals are render on mesh
 * \author Christopher Tarento
 * \author Nevrax France
 * \date 2007
 */
class CDecal : public CTransform
{
public:
	/** Contructor
	  */
	CDecal();

	/** Destructor
	  */
	~CDecal();

	/** Initilization
	  * Implementation for Ctransform
	  */
	void initModel();

	/** Registering decal as valid model
	  * Needed for scene auto registering process
	  */
	static void registerBasic();

	/** Render traversal
	  * No-op. Add this decal to the decal manager
	  */
	void traverseRender();

	/** Get Material
	  * \return decal material
	  */
	CMaterial &getMaterial(){return _Mat;}
	
	/** Get Vertices
	  * Recompute vertices if needed and return update vector
	  * \return vector of vertices ready to be rendered
	  */
	std::vector<CVector> &getVertices(const bool useVertexProgram);
	
	/** Set UV coordinate
	  * Allow decal to share a single texture
	  * \param v1 (0,0) corner
	  * \param v2 (1,1) corner
	  */
	void setUVCoord(const CUV uv1, const CUV uv2);

	/** Enable decal Clipping
	  * Rendering is direct or after processing clipping
	  * \param clip enable or disable clipping
	  */
	void enableClipping(const bool clip){_DecalContext.Clipping = clip;}

	/** Indicates if this decal is a static one
	  * Allow to manager specific optimizations 
	  * \param isStatic value
	  */
	void setStatic(const bool isStatic){_IsStatic = isStatic;}
	

private:
	/** Creator
	  * Used when we add a decal to the scene
	  */
	static CTransform	*creator() {return new CDecal();}
	
	/** Compute decal collision and projection on shape
	  * Called by getVertices
	  * \param useVertexProgram Indicate if we must use vertex program for rendering
	  */
	void computeDecal(const bool useVertexProgram);

	/////////////////////////////////////////////////////////////

public:
	/** Get the Matrix that transforms local coordinates in uv coordinates
	  * matrix to map (x,y) = (0,0) to (u,v) = (0,1) & (x,y) = (0,1) to (u,v) = (0,0) in local decal space
	  * \return reverse Matrix
	  */
	static CMatrix getReverseUVMatrix()
	{
		CMatrix m;
		m.setRot(CVector::I,-CVector::J,CVector::K);
		m.setPos(CVector::J);
		return m;
	};

private:
	CVertexBuffer _Vb;
	
	CIndexBuffer _Ib;
	
	CMaterial _Mat;
	///////////////////////////////////////////////////////////////
	bool						_Touched;
			
	CVector						_LastCamPos;
	CVector						_ClipCorners[4];
	CVector						_RefPosition;
	float						_WorldToUVMatrix[4][4];
	CMatrix						_InvertedWorldMatrix;
	CMatrix						_TextureMatrix;
	float						_WorldMatrixFlat[16];

	CShadowMap					*_ShadowMap;
	
	
	CVertexBuffer _VB;
	bool _VBInitialized;
	std::vector<CVector>		_Vertices;
	bool						_IsStatic;
	
	CUV							_UV1;
	CUV							_UV2;
	CDecalContext				_DecalContext;
	//////////////////////////////////////////////////////////////


};

}//NL3D
#endif
