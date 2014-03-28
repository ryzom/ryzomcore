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


#include "nel/3d/usr_shader_manager.h"
#include "nel/3d/usr_shader_program.h"
#include "nel/3d/usr_shader_visitor.h"

namespace NL3D
{
	CUsrShaderManager::CUsrShaderManager()
	{
	}

	CUsrShaderManager::~CUsrShaderManager()
	{
		clear();
	}


	void CUsrShaderManager::clear()
	{
		std::map< std::string, CUsrShaderProgram* >::iterator itr = programs.begin();
		while( itr != programs.end() )
		{
			delete itr->second;
			++itr;
		}
		programs.clear();
	}

	void CUsrShaderManager::getShaderList( std::vector< std::string > &v )
	{
		v.clear();

		std::string n;
		std::map< std::string, CUsrShaderProgram* >::iterator itr = programs.begin();
		while( itr != programs.end() )
		{
			itr->second->getName( n );
			v.push_back( n );
			++itr;
		}
	}

	bool CUsrShaderManager::addShader( CUsrShaderProgram *program )
	{
		std::string n;
		program->getName( n );

		std::map< std::string, CUsrShaderProgram* >::iterator itr
			= programs.find( n );
		if( itr != programs.end() )
			return false;

		programs[ n ] = program;
		return true;
	}

	bool CUsrShaderManager::removeShader( const std::string &name )
	{
		std::map< std::string, CUsrShaderProgram* >::iterator itr
			= programs.find( name );
		if( itr == programs.end() )
			return false;

		delete itr->second;
		itr->second = NULL;
		programs.erase( itr );

		return true;
	}

	bool CUsrShaderManager::changeShader( const std::string &name, CUsrShaderProgram *program )
	{
		std::map< std::string, CUsrShaderProgram* >::iterator itr
			= programs.find( name );
		if( itr == programs.end() )
			return false;

		CUsrShaderProgram *p = itr->second;
		std::string s;

		program->getName( s );
		p->setName( s );

		program->getDescription( s );
		p->setDescription( s );

		program->getVP( s );
		p->setVP( s );

		program->getFP( s );
		p->setFP( s );

		return true;		
	}

	bool CUsrShaderManager::getShader( const std::string &name, CUsrShaderProgram *program )
	{
		std::map< std::string, CUsrShaderProgram* >::iterator itr
			= programs.find( name );
		if( itr == programs.end() )
			return false;

		CUsrShaderProgram *p = itr->second;
		std::string s;

		program->setName( name );
		
		p->getDescription( s );
		program->setDescription( s );

		p->getVP( s );
		program->setVP( s );

		p->getFP( s );
		program->setFP( s );

		return true;
	}

	void CUsrShaderManager::visitShaders( IUsrShaderVisitor *visitor )
	{
		std::map< std::string, CUsrShaderProgram* >::iterator itr = programs.begin();
		while( itr != programs.end() )
		{
			visitor->visit( itr->second );
			++itr;
		}
	}

	void CUsrShaderManager::visitShader( const std::string &name, IUsrShaderVisitor *visitor )
	{
		std::map< std::string, CUsrShaderProgram* >::iterator itr =
			programs.find( name );
		if( itr == programs.end() )
			return;

		visitor->visit( itr->second );
	}
}


