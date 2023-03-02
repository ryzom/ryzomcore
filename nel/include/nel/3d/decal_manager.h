/** \file decal_manager.h
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

#ifndef NL_DECAL_MANAGER_H
#define NL_DECAL_MANAGER_H

#include <nel/misc/string_mapper.h>

#include "nel/3d/transform.h"

#include "nel/3d/decal.h"
#include "nel/3d/material.h"
#include "nel/3d/vertex_buffer.h"
#include "nel/3d/index_buffer.h"
#include "nel/3d/driver.h"




namespace NLMISC
{
	class CPlane;
}


namespace NL3D
{

class CScene;




// ***************************************************************************
/**
 * Decal Manager contains decal and computes batch rendering
 * \author Christopher Tarento
 * \author Nevrax France
 * \date 2007
 */
class CDecalManager
{

public:
	/** Constructor
	  */
	CDecalManager();
	
	/** Destructor
	  */
	~CDecalManager();
	
	/** Render all decals
	  * \param sc Owner scene
	  */
	void flush(CScene *sc);
	
	/** Add a decal to the manager
	  * \param decal Decal to add to the manager
	  * \param texName Name of the decal's texture
	  */
	void addDecal(CDecal *decal, const std::string &texName);

	/** Clear all the decals of the manager
	  *
	  */
	void clearAllDecals();

	/** Indicates if we must recompute decals position
	  * \param b true or false
	  */
	void setTouched(const bool b){_Touched = b;}

	/** Indicates if we must use vertex program for rendering
	  * \param b true or false
	  */
	void setVertexProgram(const bool b){_UseVertexProgram = b;}

private:
	///tmp
	void computeBatch();
	void renderStatic();
	void renderDynamic();

private:
	typedef CHashMap<std::string, std::vector<CDecal*> > TDecalMap;
	//typedef std::hash_map<std::string, std::vector<CMaterial> > TMaterialMap;
	//typedef std::hash_map<std::string, std::vector<CVertexBuffer> > TVBMap;
	TDecalMap								_Decals;
	//TMaterialMap							_Materials;
	//TVBMap									_VBs;

	bool									_UseVertexProgram;
	
	std::vector<CDecal*>					_StaticDecals;
	std::vector<CDecal*>					_DynamicDecals;
	CVertexBuffer							_VBRaw;//static, sorted by material
	CVertexBuffer							_VB;//dynamic
	uint									_UsedVertices;
	bool									_Touched;///someone has moved ?
};



}//NL3D
#endif