// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2013  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
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


#ifndef PROC_H
#define PROC_H

#include "nel/misc/types_nl.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>

namespace NLGUI
{
	class CParamBlock
	{
	public:
		// -1 if not a param id, but a string
		sint32 NumParam;
		std::string String;

		CParamBlock()
		{
			NumParam = -1;
		}
	};


	class CProcAction
	{
	public:
		// a condition to launch this action handler (is an expression)
		std::vector< CParamBlock > CondBlocks;

		// the action handler (may be proc!!)
		std::string Action;
		std::string Parameters;
		std::string Conditions;

		// A list of string/or param number => to build the final params at execution
		std::vector< CParamBlock > ParamBlocks;

		// build a paramBlock from a string
		void buildParamBlock( const std::string &params );
		// from ParamBlock, and a paramList (skip the 0th), build params.
		void buildParams( const std::vector< std::string > &paramList, std::string &params ) const;

		void buildCondBlock( const std::string &params );

		void buildCond( const std::vector< std::string > &paramList, std::string &cond ) const;

		static void buildBlocks( const std::string &in, std::vector< CParamBlock > &out );

		static void eval( const std::vector< std::string > &inArgs, const std::vector< CParamBlock > &inBlocks, std::string &out );

	};


	class CActionNameIs
	{
	public:
		CActionNameIs( const std::string &n )
		{
			name = n;
		}

		bool operator()( const CProcAction &action )
		{
			if( action.Action == name )
				return true;
			else
				return false;
		}

	private:
		std::string name;
	};


	class CProcedure
	{
	public:
		// List of the actions
		std::vector< CProcAction > Actions;

		bool hasAction( const std::string &name ) const
		{
			std::vector< CProcAction >::const_iterator itr
				= std::find_if( Actions.begin(), Actions.end(), CActionNameIs( name ) );
			if( itr != Actions.end() )
				return true;
			else
				return false;
		}

		bool swap( uint32 i1, uint32 i2 )
		{
			if( i1 == i2 )
				return false;
			if( i1 >= Actions.size() )
				return false;
			if( i2 >= Actions.size() )
				return false;

			CProcAction a = Actions[ i1 ];
			Actions[ i1 ] = Actions[ i2 ];
			Actions[ i2 ] = a;

			return true;
		}

		bool addAction( const std::string &name )
		{
			Actions.push_back( CProcAction() );
			Actions.back().Action = name;

			return true;
		}

		bool removeAction( uint32 i )
		{
			if( i >= Actions.size() )
				return false;
			std::vector< CProcAction >::iterator itr = Actions.begin() + i;
			Actions.erase( itr );

			return true;
		}

	};
	
	typedef	std::map< std::string, CProcedure > TProcedureMap;
}

#endif

