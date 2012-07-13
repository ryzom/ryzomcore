// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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


#ifndef PROC_H
#define PROC_H

#include "nel/misc/types_nl.h"
#include <string>
#include <vector>

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


	class CProcedure
	{
	public:
		// List of the actions
		std::vector< CProcAction > Actions;
	};
}

#endif

