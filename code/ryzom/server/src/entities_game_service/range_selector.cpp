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
#include "range_selector.h"
#include "player_manager/character.h"
#include "pvp_manager/pvp_manager_2.h"
#include "phrase_manager/sabrina_area_debug.h"

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void CRangeSelector::buildDisc( CEntityBase * actor, sint32 x, sint32 y,float radius, CEntityMatrix & matrix, bool offensiveAction, bool ignoreMainTarget)
{
	clear();
	CEntityMatrixPatternSymetrical* pattern = CEntityMatrixPatternSymetrical::bestDiscPattern( uint16( radius + 1 ) );
	CEntityMatrix::CEntityIteratorDisc it = matrix.beginEntitiesInDisc(pattern,x,y, double(radius)  * double(radius) );
	CCharacter * c = dynamic_cast< CCharacter * >(actor);
	for (; !it.end() ;++it)
	{
		for (uint32 i=0;i<_Entities.size();++i)
			BOMB_IF(_Entities[i]==&*it,"BUG: We just tried to insert the same entity into a disk entity list more than once!",continue);

		if( c != 0 )
		{
			CEntityBase * areaTarget = &(*it);
			if( c != 0)
			{
				// Do not add the entity if a PVP rule excludes it from being selected
				if( ! CPVPManager2::getInstance()->canApplyAreaEffect( c, areaTarget, offensiveAction, ignoreMainTarget ) )
					continue;
				// Do not add invisible entity for player
				if( !R2_VISION::isEntityVisibleToPlayers(areaTarget->getWhoSeesMe()))
					continue;
				// Do not add invulnerable entities, they are GM and use a slot for nothing
				if( areaTarget && areaTarget->invulnerableMode() )
					continue;
			}
			else
			{
				// Do not add invisible entity for creature
				if( !R2_VISION::isEntityVisibleToMobs(areaTarget->getWhoSeesMe()))
					continue;
				// Do not add invulnerable entities, they are GM and use a slot for nothing
				if( areaTarget && areaTarget->invulnerableMode() )
					continue;
			}
		}

		_Entities.push_back(&*it);
		_Distances.push_back( it.getDistance() );
	}

	if ( DumpRangeAnalysis &&  AreaDebug.doIDump() )
	{
		AreaDebug.dumpBomb( NULL,_Entities,radius );
	}
}
	
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void CRangeSelector::buildCone(CEntityBase* source , CEntityBase* target,float height,float minWidth, float maxWidth, CEntityMatrix & matrix, bool offensiveAction, bool ignoreMainTarget)
{
	clear();
#ifdef NL_DEBUG
	nlassert(source);
	nlassert(target);
#endif

	// we first have to build 2 vectors normal to the direction of the spray
	// first vector has a norm equal to minWidth/2, the second has a norm equal to maxWidth/2
	// we aloso have to get the extreme point of the height of our trapezoid
	CAreaCoords<sint32> normalMin;
	CAreaCoords<sint32> normalMax;
	CAreaCoords<sint32> extremeHeight;
	minWidth /= 2;
	maxWidth /= 2;

	float dist = (float) sqrt( pow( (source->getX() - target->getX())/1000.0,2 ) + pow( (source->getY() - target->getY() )/ 1000.0f,2 ) );

	// first case : the source and the target are the same : the direction of the spray is given by the orientation of the entity
	if ( dist == 0.0f || source == target )
	{
		// compute a direction vector ( 1000 mm norm )
		double x = 1000 * cos( source->getHeading() );
		double y = 1000 * sin( source->getHeading() );
		
		// set the normals
		normalMin.X = sint32(- y * minWidth );
		normalMin.Y = sint32( x * minWidth );
		normalMax.X = sint32(- y * maxWidth );
		normalMax.Y = sint32( x * maxWidth );

		extremeHeight.X = sint32( source->getX() + x * height );
		extremeHeight.Y = sint32( source->getY() + y * height );
	}
	// otherwise, the direction is the vector source-entity
	else
	{
		CAreaCoords<sint32> dirVect( target->getX() - source->getX(), target->getY() - source->getY() );
		// compute norm factors
		float factorMin = minWidth / dist;
		float factorMax = maxWidth / dist;
		
		normalMin.X = sint32(- dirVect.Y * factorMin);
		normalMin.Y = sint32( dirVect.X * factorMin );
		normalMax.X = sint32(- dirVect.Y * factorMax);
		normalMax.Y = sint32( dirVect.X * factorMax );

		// get the extreme point of the height
		dirVect*=( 1.0f + height / dist );
		extremeHeight.X = dirVect.X + source->getX();
		extremeHeight.Y = dirVect.Y + source->getY();
	}

	// build the cone from our two normals vector
	CAreaQuad<sint32> quad;
	quad.Vertices[0].X = target->getX() + normalMin.X;
	quad.Vertices[0].Y = target->getY() + normalMin.Y;
	quad.Vertices[1].X = target->getX() - normalMin.X;
	quad.Vertices[1].Y = target->getY() - normalMin.Y;

	quad.Vertices[2].X = extremeHeight.X - normalMax.X;
	quad.Vertices[2].Y = extremeHeight.Y - normalMax.Y;

	quad.Vertices[3].X = extremeHeight.X + normalMax.X;
	quad.Vertices[3].Y = extremeHeight.Y + normalMax.Y;
	

	//  build the bounding rect
	CAreaRect<sint32> rect( quad );
	
	// get the entities in the cone but preselect them with a rect pattern, built from our bounding rect
	CEntityMatrixPatternRect pattern( rect.Width,rect.Height);

	_Entities.push_back( target );
	_Distances.push_back(0);
	// iterate through the entities
	CCharacter * c = dynamic_cast<CCharacter *>(source);
	for (CEntityMatrix::CEntityIteratorCone  it = matrix.beginEntitiesInCone(&pattern,&quad,rect.center(),target ); !it.end() ;++it)
	{
		CEntityBase * areaTarget = &(*it);
		if( c != 0 )
		{
			// Do not add the entity if a PVP rule excludes it from being selected
			if( ! CPVPManager2::getInstance()->canApplyAreaEffect( c, areaTarget, offensiveAction, ignoreMainTarget ) )
				continue;
			// Do not add invisible entity for player
			if( !R2_VISION::isEntityVisibleToPlayers(areaTarget->getWhoSeesMe()))
				continue;
			// Do not add invulnerable entities, they are GM and use a slot for nothing
			if( areaTarget && areaTarget->invulnerableMode() )
				continue;
		}
		else
		{
			// Do not add invisible entity for creature
			if( !R2_VISION::isEntityVisibleToMobs(areaTarget->getWhoSeesMe()))
				continue;
			// Do not add invulnerable entities, they are GM and use a slot for nothing
			if( areaTarget && areaTarget->invulnerableMode() )
				continue;
		}

		_Entities.push_back(&*it);
		_Distances.push_back( it.getDistance() );
	}

	if ( DumpRangeAnalysis &&  AreaDebug.doIDump() )
	{
		AreaDebug.dumpCone( source,_Entities,quad);
	}
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void CRangeSelector::buildChain( CEntityBase* actor, CEntityBase* target, float range,uint maxEntities, CEntityMatrix & matrix, ACTNATURE::TActionNature nature, bool ignoreMainTarget)
{
	clear();
#ifdef NL_DEBUG
	nlassert(target);
#endif
	double rangeSqr = range * range;
	float distance = (float)1E10;
	uint8 maxPatternSide = 3 + (uint8)(range /8);
	const CEntityBase * mainTarget = target;
	CEntityBase * entity = NULL;
	CCharacter * c = dynamic_cast<CCharacter *>(actor);
	CEntityMatrixPatternSymetrical* pattern = CEntityMatrixPatternSymetrical::bestDiscPattern( 0 );
	while ( maxEntities )
	{
		CEntityMatrix::CEntityIteratorChainCenter it = matrix.beginEntitiesInChainCenter(pattern,target->getX(),target->getY(), rangeSqr , &_Entities, nature,actor->getEntityRowId(), mainTarget);
		// get the nearest valid entity
		for (; !it.end(); ++it )
		{
			if ( it.getDistance() < distance)
			{
				CEntityBase * areaTarget = &(*it);
				if( c != 0 )
				{
					// Do not add the entity if a PVP rule excludes it from being selected
					if( ! CPVPManager2::getInstance()->canApplyAreaEffect( c, areaTarget, (nature == ACTNATURE::FIGHT || nature == ACTNATURE::OFFENSIVE_MAGIC), ignoreMainTarget ) )
						continue;
					// Do not add invisible entity for player
					if( !R2_VISION::isEntityVisibleToPlayers(areaTarget->getWhoSeesMe()))
						continue;
					// Do not add invulnerable entities, they are GM and use a slot for nothing
					if( areaTarget && areaTarget->invulnerableMode() )
						continue;
				}
				else
				{
					// Do not add invisible entity for creature
					if( !R2_VISION::isEntityVisibleToMobs(areaTarget->getWhoSeesMe()))
						continue;
					// Do not add invulnerable entities, they are GM and use a slot for nothing
					if( areaTarget && areaTarget->invulnerableMode() )
						continue;
				}

				distance = it.getDistance();
				entity = &*it;
			}
		}
		if ( !entity )
		{
			uint patternSide = 5;
			while ( 1 )
			{
				if ( patternSide > maxPatternSide )
					return;
				CEntityMatrixPatternBorder border(patternSide);
				CEntityMatrix::CEntityIteratorChainBorder it = matrix.beginEntitiesInChainBorder(&border,target->getX(),target->getY(), rangeSqr , &_Entities, nature, actor->getEntityRowId(), mainTarget);
				for (; !it.end(); ++it )
				{
					if ( it.getDistance() < distance )
					{
						CEntityBase * areaTarget = &(*it);
						if( c != 0 )
						{
							// Do not add the entity if a PVP rule excludes it from being selected
							if( CPVPManager2::getInstance()->canApplyAreaEffect( c, areaTarget, (nature == ACTNATURE::FIGHT || nature == ACTNATURE::OFFENSIVE_MAGIC), ignoreMainTarget ) )
								continue;
							// Do not add invisible entity player
							if( !R2_VISION::isEntityVisibleToPlayers(areaTarget->getWhoSeesMe()))
								continue;
							// Do not add invulnerable entities, they are GM and use a slot for nothing
							if( areaTarget && areaTarget->invulnerableMode() )
								continue;
						}
						else
						{
							// Do not add invisible entity creature
							if( !R2_VISION::isEntityVisibleToMobs(areaTarget->getWhoSeesMe()))
								continue;
							// Do not add invulnerable entities, they are GM and use a slot for nothing
							if( areaTarget && areaTarget->invulnerableMode() )
								continue;
						}
						distance = it.getDistance();
						entity = &*it;
					}
				}
				if ( entity )
					break;
				patternSide+=2;
			}
		}
		_Entities.push_back( entity );
		_Distances.push_back( distance );
		maxEntities--;
		target = entity;
		entity = NULL;
		distance = (float)1E10;
	}
	if ( DumpRangeAnalysis &&  AreaDebug.doIDump() )
	{
		AreaDebug.dumpChain( actor,_Entities);
	}
}


