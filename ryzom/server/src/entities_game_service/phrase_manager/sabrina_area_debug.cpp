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



#include "stdpch.h"
#include "phrase_manager/sabrina_area_debug.h"
#include "area_geometry.h"
#include "range_selector.h"
#include "entity_manager/entity_base.h"
#include "entity_manager/entity_manager.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "phrase_manager/s_effect.h"


CSabrinaAreaDebug AreaDebug;
uint16 CSabrinaAreaDebug::FileId = 0xFFFF;

inline sint CSabrinaAreaDebug::getXPix( sint32 x )
{
	return sint ( Center + float (x - Target->getState().X) * ( float(PixelPerMeter) / 1000.0f) );
}
inline sint CSabrinaAreaDebug::getYPix( sint32 y )
{
	return sint ( Center + float ( y - Target->getState().Y) * (float(PixelPerMeter) / 1000.0f) );
}

void CSabrinaAreaDebug::init(const TDataSetRow & targetRow, float radius, uint pixelPerMeter )
{
	CEntityBase * target = CEntityBaseManager::getEntityBasePtr(targetRow);
	if (!target)
	{
		nlwarning("<CSabrinaAreaDebug init> Invalid target %u. Dump will fail ( and also the area effect, probably ) ", targetRow.getIndex());
		return;
	}

	std::string fileName;
	if  ( FileId == 0xFFFF )
	{
		for (uint i = 1; true; i++)
		{
			NLMISC::COFile file;
			fileName = NLMISC::CPath::standardizePath( NLMISC::toString("./range_tests/test%.2u.tga", i ),false );
			if ( !NLMISC::CFile::fileExists(fileName) )
			{
				FileId = i;
				break;
			}
		}
	}
	else
	{
		fileName = NLMISC::CPath::standardizePath( NLMISC::toString("./range_tests/test%.2u.tga", FileId ) , false);
	}
	FileId++;
	

	Dump = false;
	// init internal vars
	PixelPerMeter = pixelPerMeter;
	ViewRadius = radius;
	Center = (sint) (radius * (float)pixelPerMeter);
	uint side = uint( 2.0 * radius * (float)pixelPerMeter );		
	Img.setup(side,side,fileName,0,0);
	Img.setupForCol();
	Target = target;
	// init our pixel matrix
	Lines.clear();
	Lines.resize( side, std::vector<uint16>(side,0) );
	
	// init the image file
	Img.ColorMap.resize(16);
	Img.ColorMap[0] = Img.getColor16(0x00, 0x00, 0x00);// black
	Img.ColorMap[1] = Img.getColor16(0xFF, 0xFF, 0xFF);// white ( outside / un affected entity )
	Img.ColorMap[2] = Img.getColor16(0x00, 0x00, 0xFF);// red ( affected entities )
	Img.ColorMap[3] = Img.getColor16(0x00, 0xFF, 0x00);// green ( caster )
	Img.ColorMap[4] = Img.getColor16(0xFF, 0x00, 0x00);// blue ( main target )
	Img.ColorMap[5] = Img.getColor16(0x00, 0xFF, 0xFF);// yellow ( shape )
	
	
	// draw the viewed area
	drawCircle( Center,Center,Center,1 );
	for ( uint i = 0; i < Lines.size(); i++ )
	{
		for  (uint j = 0; j < Lines[i].size();j++)
		{
			if ( !Lines[i][j] )
				Lines[i][j] = 1;
			else
				break;
		}
		for  (sint j = (sint)Lines[i].size() - 1; j >= 0;j--)
		{
			if ( !Lines[i][j] )
				Lines[i][j] = 1;
			else
				break;
		}
	}
	Dump = true;
}

void CSabrinaAreaDebug::dumpBomb( CEntityBase * caster, const std::vector<CEntityBase*> & entities, float radius )
{
	if( !Target) 
		return;
	// draw effect
	drawCircle( Center,Center, uint( radius * PixelPerMeter  ),5 );
	// dump
	dump(caster,entities);
}


void CSabrinaAreaDebug::dumpCone( CEntityBase * caster, const std::vector<CEntityBase*> & entities, const CAreaQuad<sint32> & cone )
{
	if( !Target) 
		return;

	// draw cone
	for ( uint i = 0; i < 4; i++ )
	{
		sint xs = getXPix( cone.Vertices[i].X );
		sint ys = getYPix( cone.Vertices[i].Y );

		sint xe = getXPix( cone.Vertices[(i+1)%4].X );
		sint ye = getYPix( cone.Vertices[(i+1)%4].Y );

		drawLine( xs,ys,xe,ye,5 );
	}
	// dump
	dump(caster,entities);
}

void CSabrinaAreaDebug::dumpChain( CEntityBase * caster, const std::vector<CEntityBase*> & entities)
{
	if( !Target) 
		return;
	
	
	// draw chain
	for ( uint i = 0; i < entities.size() -1; i++ )
	{
		sint xs = getXPix( entities[i]->getState().X );
		sint ys = getYPix( entities[i]->getState().Y );

		sint xe = getXPix( entities[i+1]->getState().X );
		sint ye = getYPix( entities[i+1]->getState().Y );
		
		
		drawLine( xs,ys,xe,ye,5 );
	}
	// dump
	dump(caster,entities);
}

inline void CSabrinaAreaDebug::drawEntity( CEntityBase * entity, uint color )
{
	sint x = getXPix( entity->getState().X );
	sint y = getYPix( entity->getState().Y );

	for ( sint i = -5; i <= 5; i++ )
	{
		if ( !setPoint(x + i ,y,color) && i == 0)
			nlwarning("failed to draw an entity!! (x)");
		if ( !setPoint(x ,y + i,color) && i == 0)
			nlwarning("failed to draw an entity!! (y)");
	}
}

void CSabrinaAreaDebug::dump(CEntityBase * caster,const  std::vector<CEntityBase*> &entities)
{
	Dump = false;
	// draw all the entities in the viewed area
	CRangeSelector selector;
	selector.buildDisc(caster,Target->getX(),Target->getY(),ViewRadius,EntityMatrix,true);
	for ( uint i = 0; i < selector.getEntities().size(); i++ )
	{
		if (selector.getEntities()[i])
		{
			if ( std::find(entities.begin(),entities.end(),selector.getEntities()[i]) == entities.end() )
			{
				drawEntity(selector.getEntities()[i],1);
			}
		}
		else
			nlwarning("bad entity");
	}
	// draw affected entities
	for ( uint i = 0; i < entities.size(); i++ )
	{
		drawEntity(entities[i],2);
	}
	// draw caster
	if (caster)
		drawEntity(caster,3);
	// draw target
	drawEntity(Target,4);
	for ( uint i = 0; i < Lines.size(); i++ )
	{
		for (uint j = 0; j < Lines[i].size(); j++)
		{
			Img.set( j, Lines[i][j] );
		}
		Img.writeLine();
	}
	Img.File.close();
	Dump = true;
}


