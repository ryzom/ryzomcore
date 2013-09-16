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


#ifndef USR_SHADER_SAVER_H
#define USR_SHADER_SAVER_H

#include "nel/3d/usr_shader_visitor.h"
#include <string>

namespace NL3D
{
	class CUsrShaderProgram;
	class CUsrShaderManager;

	class CUsrShaderSaver : public IUsrShaderVisitor
	{
	public:
		CUsrShaderSaver();
		~CUsrShaderSaver();

		void setManager( CUsrShaderManager *mgr ){ manager = mgr; }

		void visit( CUsrShaderProgram *program );

		void saveShaders( const std::string &directory );
		void saveShader( const std::string &directory, const std::string &name );

	private:
		CUsrShaderManager *manager;
		std::string outputDir;
	};
}

#endif


