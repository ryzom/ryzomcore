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

#include "nel/3d/shader.h"
#include "nel/3d/driver.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"

using namespace std;
using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************

CShader::~CShader()
{
	// Must kill the drv mirror of this shader.
	_DrvInfo.kill();
}

// ***************************************************************************

CShader::CShader()
{
	_ShaderChanged = true;
}

// ***************************************************************************

void CShader::setText (const char *text)
{
	_Text = text;
	_ShaderChanged = true;
}

// ***************************************************************************

void CShader::setName (const char *name)
{
	_Name = name;
	_ShaderChanged = true;
}

// ***************************************************************************

bool CShader::loadShaderFile (const char *filename)
{
	_Text = "";
	// Lookup
	string _filename = CPath::lookup(filename, false, true, true);
	if (!_filename.empty())
	{
		// File length
		uint size = CFile::getFileSize (_filename);
		_Text.reserve (size+1);

		try
		{
			CIFile file;
			if (file.open (_filename))
			{
				// Read it
				while (!file.eof ())
				{
					char line[512];
					file.getline (line, 512);
					_Text += line;
				}

				// Set the shader name
				_Name = CFile::getFilename (filename);
				return true;
			}
			else
			{
				nlwarning ("Can't open the file %s for reading", _filename.c_str());
			}
		}
		catch (Exception &e)
		{
			nlwarning ("Error while reading %s : %s", _filename.c_str(), e.what());
		}
	}
	return false;
}

// ***************************************************************************

IShaderDrvInfos::~IShaderDrvInfos()
{
	_Driver->removeShaderDrvInfoPtr(_DriverIterator);
}

} // NL3D
