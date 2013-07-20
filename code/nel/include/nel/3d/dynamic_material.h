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


#ifndef DYN_MATERIAL_H
#define DYN_MATERIAL_H

#include "nel/misc/stream.h"
#include "nel/misc/variant.h"
#include <string>
#include <vector>

namespace NL3D
{

	struct SDynMaterialProp
	{
		enum EPropertyType
		{
			Color,
			Vector4,
			Float,
			Double,
			Int,
			Uint,
			Matrix4,
			Texture
		};

		std::string prop;
		std::string label;
		uint8 type;

		NLMISC::CVariant value;

		void serial( NLMISC::IStream &f );
	};



	struct SRenderPass
	{
	public:
		void addProperty( const SDynMaterialProp &prop );
		void removeProperty( const std::string &name );
		bool changeProperty( const std::string &name, const SDynMaterialProp &prop );
		void setName( const std::string &n ){ name = n; }
		void getName( std::string &n ) const { n = name; }
		void getShaderRef( std::string &s ) const{ s = shaderRef; }
		void setShaderRef( const std::string &s ){ shaderRef = s; }
		void serial( NLMISC::IStream &f );

		uint32 count(){ return properties.size(); }
		void clear(){ properties.clear(); }
		const SDynMaterialProp* getProperty( uint32 i ) const;

	private:
		std::vector< SDynMaterialProp > properties;
		std::string shaderRef;
		std::string name;
	};




	class CDynMaterial : public NLMISC::IStreamable
	{
	public:
		CDynMaterial();
		~CDynMaterial();
		void reconstruct();
		void clear();
		void serial( NLMISC::IStream &f );
		std::string getClassName(){ return "CDynMaterial"; }

		void addPass( const SRenderPass &pass );
		void removePass( const std::string &name );
		void renamePass( const std::string &from, const std::string &to );
		void movePassUp( const std::string &name );
		void movePassDown( const std::string &name );

		SRenderPass* getPass( const std::string &name );
		SRenderPass* getPass( uint32 i );
		uint32 count(){ return passes.size(); }
		void getPassList( std::vector< std::string > &l );

	private:
		std::vector< SRenderPass* > passes;
	};
}

#endif


