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


#include "nel/3d/shader_loader.h"
#include "nel/3d/shader_manager.h"
#include "nel/3d/shader_program.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/i_xml.h"

namespace NL3D
{
	CShaderLoader::CShaderLoader()
	{
		manager = NULL;
	}

	CShaderLoader::~CShaderLoader()
	{
	}

	void CShaderLoader::loadShaders( const std::string &directory )
	{
		if( manager == NULL )
			return;

		std::vector< std::string > files;

		NLMISC::CPath::getPathContent( directory, true, false, true, files );

		std::vector< std::string >::iterator itr = files.begin();
		while( itr != files.end() )
		{
			if( NLMISC::CFile::getExtension( *itr ) == ".nelshdr" )
			{
				loadShader( *itr );
			}

			++itr;
		}
	}

	void CShaderLoader::loadShader( const std::string &file )
	{
		NLMISC::CIFile f;
		if( !f.open( file, true ) )
			return;

		NLMISC::CIXml xml;
		if( !xml.init( f ) )
			return;

		CShaderProgram *p = new CShaderProgram();
		p->serial( xml );
		manager->addShader( p );

		f.close();
	}
}

