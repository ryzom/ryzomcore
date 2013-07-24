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


#include "nel/3d/shape_material_serializer.h"
#include "nel/3d/shape.h"
#include "nel/3d/material.h"
#include "nel/3d/dynamic_material.h"
#include "nel/3d/mesh_base.h"
#include "nel/misc/file.h"
#include "nel/misc/o_xml.h"

namespace NL3D
{
	ShapeMatSerial::ShapeMatSerial()
	{
		shape = NULL;
	}

	ShapeMatSerial::~ShapeMatSerial()
	{
		shape = NULL;
	}

	void ShapeMatSerial::serial( const char *sPath )
	{
		if( shape == NULL )
			return;

		nlinfo( "Exporting materials of %s", sPath );

		std::string path = sPath;
		std::string fname;
		std::string::size_type idx;

		idx = path.find_last_of( '.' );
		path = path.substr( 0, idx );

		CMeshBase *mb = dynamic_cast< CMeshBase* >( shape );
		if( mb != NULL )
		{
			uint n = mb->getNbMaterial();
			nlinfo( "exporting %u materials", n );

			for( int i = 0; i < n; i++ )
			{
				CMaterial &m = mb->getMaterial( i );
				CDynMaterial *dm = m.getDynMat();
				if( dm == NULL )
				{
					m.createDynMat();
					dm = m.getDynMat();
				}

				fname = path + "_";
				fname += char( '0' + i );
				fname += ".nelmat";

				nlinfo( "exporting to %s", fname.c_str() );

				NLMISC::COFile o;
				if( o.open( fname ) )
				{
					NLMISC::COXml xml;
					if( xml.init( &o ) )
					{
						dm->serial( xml );
						xml.flush();
					}
					else
					{
						nlerror( "Error initializing XML output stream for %s", fname.c_str() );
					}
					o.close();
				}
				else
				{
					nlerror( "Error creating file %s", fname.c_str() );
				}

			}
		}

	}
}




