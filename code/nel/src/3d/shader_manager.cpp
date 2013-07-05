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


#include "nel/3d/shader_manager.h"
#include "nel/3d/shader_program.h"
#include "nel/3d/shader_visitor.h"

namespace NL3D
{
	CShaderManager::CShaderManager()
	{
	}

	CShaderManager::~CShaderManager()
	{
		clear();
	}


	void CShaderManager::clear()
	{
		std::tr1::unordered_map< std::string, CShaderProgram* >::iterator itr = programs.begin();
		while( itr != programs.end() )
		{
			delete itr->second;
			++itr;
		}
		programs.clear();
	}

	void CShaderManager::getShaderList( std::vector< std::string > &v )
	{
		v.clear();

		std::string n;
		std::tr1::unordered_map< std::string, CShaderProgram* >::iterator itr = programs.begin();
		while( itr != programs.end() )
		{
			itr->second->getName( n );
			v.push_back( n );
			++itr;
		}
	}

	void CShaderManager::addShader( CShaderProgram *program )
	{
		std::string n;
		program->getName( n );

		std::tr1::unordered_map< std::string, CShaderProgram* >::iterator itr
			= programs.find( n );
		if( itr != programs.end() )
			return;

		programs[ n ] = program;
	}

	void CShaderManager::changeShader( const std::string &name, CShaderProgram *program )
	{
		std::tr1::unordered_map< std::string, CShaderProgram* >::iterator itr
			= programs.find( name );
		if( itr == programs.end() )
			return;

		CShaderProgram *p = itr->second;

		std::string s;

		program->getName( s );
		p->setName( s );

		program->getDescription( s );
		p->setDescription( s );

		program->getVP( s );
		p->setVP( s );

		program->getFP( s );
		p->setFP( s );
		
	}

	void CShaderManager::visitShaders( IShaderVisitor *visitor )
	{
		std::tr1::unordered_map< std::string, CShaderProgram* >::iterator itr = programs.begin();
		while( itr != programs.end() )
		{
			visitor->visit( itr->second );
			++itr;
		}
	}

	void CShaderManager::visitShader( const std::string &name, IShaderVisitor *visitor )
	{
		std::tr1::unordered_map< std::string, CShaderProgram* >::iterator itr =
			programs.find( name );
		if( itr == programs.end() )
			return;

		visitor->visit( itr->second );
	}
}


