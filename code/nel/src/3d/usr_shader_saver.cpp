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


#include "nel/3d/usr_shader_saver.h"
#include "nel/3d/usr_shader_manager.h"
#include "nel/3d/usr_shader_program.h"
#include "nel/misc/file.h"
#include "nel/misc/o_xml.h"

namespace NL3D
{
	CUsrShaderSaver::CUsrShaderSaver()
	{
		manager = NULL;
	}

	CUsrShaderSaver::~CUsrShaderSaver()
	{
	}

	void CUsrShaderSaver::visit( CUsrShaderProgram *program )
	{
		std::string fn;
		program->getName( fn );
		fn += ".nlshdr";

		fn = outputDir + "/" + fn;

		NLMISC::COFile of;
		if( !of.open( fn, false, true ) )
			return;

		NLMISC::COXml xml;
		if( !xml.init( &of ) )
			return;

		program->serial( xml );

		xml.flush();
		of.close();

	}

	void CUsrShaderSaver::saveShaders( const std::string &directory )
	{
		outputDir = directory;
		manager->visitShaders( this );
	}

	void CUsrShaderSaver::saveShader( const std::string &directory, const std::string &name )
	{
		outputDir = directory;
		manager->visitShader( name, this );
	}
}


