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


#include "nel/3d/dyn_mat_loader.h"
#include "nel/3d/dynamic_material.h"
#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"

namespace NL3D
{
	CDynMatLoader::CDynMatLoader()
	{
		mat = NULL;
	}

	CDynMatLoader::~CDynMatLoader()
	{
		mat = NULL;
	}

	bool CDynMatLoader::loadFrom( const std::string &fileName )
	{
		NLMISC::CIFile ifile;
		if( !ifile.open( fileName, true ) )
		{
			nlinfo( "Error opening file %s", fileName.c_str() );
			return false;
		}

		NLMISC::CIXml xml;
		if( !xml.init( ifile ) )
		{
			ifile.close();
			nlinfo( "Error initializing XML stream for file %s", fileName.c_str() );
			return false;
		}

		mat = new CDynMaterial();
		mat->serial( xml );

		ifile.close();

		return true;
	}
}


