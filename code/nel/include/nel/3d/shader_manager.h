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


#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <unordered_map>
#include <vector>
#include <string>

namespace NL3D
{
	class CShaderProgram;
	class IShaderVisitor;

	class CShaderManager
	{
	public:
		CShaderManager();
		~CShaderManager();
		void clear();
		void getShaderList( std::vector< std::string > &v );
		void addShader( CShaderProgram *program );
		void changeShader( const std::string &name, CShaderProgram *program );
		void visitShaders( IShaderVisitor *visitor );
		void visitShader( const std::string &name, IShaderVisitor *visitor );

	private:
		std::tr1::unordered_map< std::string, CShaderProgram* > programs;
	};
}

#endif

