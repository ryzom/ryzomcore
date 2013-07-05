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


#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include "nel/misc/stream.h"
#include <string>

namespace NL3D
{
	class CShaderProgram : public NLMISC::IStreamable
	{
	public:
		CShaderProgram();
		
		~CShaderProgram();
		
		std::string getClassName(){ return "CShaderProgram"; }
		
		void serial( NLMISC::IStream &f );

		void getName( std::string &n ) const{ n = name; }
		void getDescription( std::string &d ) const{ d = description; }
		void getVP( std::string &vp ) const{ vp = vertexProgram; }
		void getFP( std::string &fp ) const{ fp = fragmentProgram; }

		void setName( const std::string &n ){ name = n; }
		void setDescription( const std::string &d ){ description = d; }
		void setVP( const std::string &vp ){ vertexProgram = vp; }
		void setFP( const std::string &fp ){ fragmentProgram = fp; }

		uint32 getVPId() const{ return vpId; }
		uint32 getFPId() const{ return fpId; }
		uint32 getPId() const{ return pId; }

		void setVPId( uint32 id ){ vpId = id; }
		void setFPId( uint32 id ){ fpId = id;} 
		void setPid( uint32 id ){ pId = id; }

	private:
		std::string name;
		std::string description;
		std::string vertexProgram;
		std::string fragmentProgram;


		uint32 vpId;
		uint32 fpId;
		uint32 pId;
	};
}


#endif


