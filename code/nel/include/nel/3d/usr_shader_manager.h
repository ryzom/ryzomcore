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


#ifndef USR_SHADER_MANAGER_H
#define USR_SHADER_MANAGER_H

#include <map>
#include <vector>
#include <string>

namespace NL3D
{
	class CUsrShaderProgram;
	class IUsrShaderVisitor;

	class CUsrShaderManager
	{
	public:
		CUsrShaderManager();
		~CUsrShaderManager();
		void clear();
		void getShaderList( std::vector< std::string > &v );
		bool addShader( CUsrShaderProgram *program );
		bool removeShader( const std::string &name );
		bool changeShader( const std::string &name, CUsrShaderProgram *program );
		bool getShader( const std::string &name, CUsrShaderProgram *program );
		void visitShaders( IUsrShaderVisitor *visitor );
		void visitShader( const std::string &name, IUsrShaderVisitor *visitor );

	private:
		std::map< std::string, CUsrShaderProgram* > programs;
	};
}

#endif

