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

#ifndef NL_SHADER_H
#define NL_SHADER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include <list>


namespace NL3D {

using NLMISC::CRefCount;


class	IDriver;

// List typedef.
class	IShaderDrvInfos;
typedef	std::list<IShaderDrvInfos*>		TShaderDrvInfoPtrList;
typedef	TShaderDrvInfoPtrList::iterator	ItShaderDrvInfoPtrList;

/**
  * Interface for shader driver infos.
  */
class IShaderDrvInfos : public CRefCount
{
private:
	IDriver				*_Driver;
	ItShaderDrvInfoPtrList		_DriverIterator;

public:
	IShaderDrvInfos(IDriver	*drv, ItShaderDrvInfoPtrList it) {_Driver= drv; _DriverIterator= it;}
	// The virtual dtor is important.
	virtual ~IShaderDrvInfos();
};


/**
 * Shader resource for the driver. It is just a container for a ".fx" text file.
 */
/* *** IMPORTANT ********************
 * *** IF YOU MODIFY THE STRUCTURE OF THIS CLASS, PLEASE INCREMENT IDriver::InterfaceVersion TO INVALIDATE OLD DRIVER DLL
 * **********************************
 */
// --------------------------------------------------
class CShader
{
public:
	CShader();
	~CShader();

	// Load a shader file
	bool loadShaderFile (const char *filename);

	// Set the shader text
	void setText (const char *text);

	// Get the shader text
	const char *getText () const { return _Text.c_str(); }

	// Set the shader name
	void setName (const char *name);

	// Get the shader name
	const char *getName () const { return _Name.c_str(); }

public:
	// Private. For Driver only.
	bool								_ShaderChanged;
	NLMISC::CRefPtr<IShaderDrvInfos>	_DrvInfo;
private:
	// The shader
	std::string					_Text;
	// The shader name
	std::string					_Name;
};


} // NL3D


#endif // NL_SHADER_H

/* End of shader.h */
