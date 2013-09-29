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
#include "config.h"

CConfig::CConfig()
{
}

CConfig::~CConfig()
{
}

bool CConfig::load( const char *fileName )
{
	try
	{
		cf.load( fileName );

		std::string def = getString( "RootConfigFilename" );
		if( def.compare( "" ) != 0 )
			dcf.load( def );
	}
	catch( NLMISC::Exception &e )
	{
		nlwarning( "%s", e.what() );
		return false;
	}

	return true;
}

bool CConfig::reload()
{
	try
	{
		cf.clear();
		cf.reparse();
	}
	catch( NLMISC::Exception &e )
	{
		nlwarning( "%s", e.what() );
		return false;
	}
	return true;
}

void CConfig::revertToDefault()
{
	// If there's no default config, all we can do is revert the current changes
	if( !dcf.loaded() )
	{
		reload();
		return;
	}

	// If there is a default config, we can however revert to the default!
	// Code taken from the original config tool
	uint32 count = cf.getNumVar();
	uint32 i = 0;
	for( i = 0; i < count; i++ )
	{
		NLMISC::CConfigFile::CVar *dst = cf.getVar( i );

		// Comment from the original
		// Temp: avoid changing this variable (debug: binded to the actual texture set installed)
		if( dst->Name.compare( "HDTextureInstalled" ) == 0 )
			continue;

		NLMISC::CConfigFile::CVar *src = dcf.getVarPtr( dst->Name );
		if( ( src != NULL ) && !dst->Root &&
				( ( src->Type == NLMISC::CConfigFile::CVar::T_INT ) && ( dst->Type == NLMISC::CConfigFile::CVar::T_INT ) ||
				  ( src->Type == NLMISC::CConfigFile::CVar::T_REAL ) && ( dst->Type == NLMISC::CConfigFile::CVar::T_INT ) ||
				  ( src->Type == NLMISC::CConfigFile::CVar::T_INT ) && ( dst->Type == NLMISC::CConfigFile::CVar::T_REAL ) ||
				  ( src->Type == NLMISC::CConfigFile::CVar::T_REAL ) && ( dst->Type == NLMISC::CConfigFile::CVar::T_REAL ) ||
				  ( src->Type == NLMISC::CConfigFile::CVar::T_STRING ) && ( dst->Type == NLMISC::CConfigFile::CVar::T_STRING ) ) )
		{

			if( src->Type == NLMISC::CConfigFile::CVar::T_INT )
			{
				setInt( src->Name.c_str(), src->asInt() );
			}
			else if( src->Type == NLMISC::CConfigFile::CVar::T_REAL )
			{
				setFloat( src->Name.c_str(), src->asFloat() );
			}
			else if( src->Type == NLMISC::CConfigFile::CVar::T_STRING )
			{
				setString( src->Name.c_str(), src->asString() );
			}
		}
	}
}

bool CConfig::save()
{
	try
	{
		cf.save();
	}
	catch( NLMISC::Exception &e )
	{
		nlwarning( "%s", e.what() );
		return false;
	}
	return true;
}

bool CConfig::getBool( const char *key )
{
	NLMISC::CConfigFile::CVar *var = cf.getVarPtr( key );

	if( var != NULL )
	{
		return var->asBool();
	}
	else
	{
		nlwarning( "Couldn't find key %s in %s.", key, cf.getFilename().c_str() );
		return false;
	}
}

sint32 CConfig::getInt( const char *key )
{
	NLMISC::CConfigFile::CVar *var = cf.getVarPtr( key );

	if( var != NULL )
	{
		return var->asInt();
	}
	else
	{
		nlwarning( "Couldn't find key %s in %s.", key, cf.getFilename().c_str() );
		return 0;
	}
}

float CConfig::getFloat( const char *key )
{
	NLMISC::CConfigFile::CVar *var = cf.getVarPtr( key );

	if( var != NULL )
	{
		return var->asFloat();
	}
	else
	{
		nlwarning( "Couldn't find key %s in %s.", key, cf.getFilename().c_str() );
		return 0.0f;
	}
}

std::string CConfig::getString( const char *key )
{
	NLMISC::CConfigFile::CVar *var = cf.getVarPtr( key );

	if( var != NULL )
	{
		return var->asString();
	}
	else
	{
		nlwarning( "Couldn't find key %s in %s.", key, cf.getFilename().c_str() );
		return "";
	}
}

void CConfig::setBool( const char *key, bool value )
{
	NLMISC::CConfigFile::CVar *var = cf.getVarPtr( key );

	if( var != NULL )
	{
		if( var->Type == NLMISC::CConfigFile::CVar::T_BOOL )
		{
			if( value )
				var->setAsString( "true" );
			else
				var->setAsString( "false" );
		}
		else
		{
			nlwarning( "Key %s in %s is not a boolean.", key, cf.getFilename().c_str() );
		}
	}
	else
	{
		nlwarning( "Couldn't find key %s in %s.", key, cf.getFilename().c_str() );
	}
}

void CConfig::setInt(const char *key, sint32 value)
{
	NLMISC::CConfigFile::CVar *var = cf.getVarPtr( key );

	if( var != NULL )
	{
		if( var->Type == NLMISC::CConfigFile::CVar::T_INT )
			var->setAsInt( value );
		else
			nlwarning( "Key %s in %s is not an integer.", key, cf.getFilename().c_str() );
	}
	else
	{
		nlwarning( "Couldn't find key %s in %s.", key, cf.getFilename().c_str() );
	}
}

void CConfig::setFloat( const char *key, float value )
{
	NLMISC::CConfigFile::CVar *var = cf.getVarPtr( key );

	if( var != NULL )
	{
		if( var->Type == NLMISC::CConfigFile::CVar::T_REAL )
			var->setAsFloat( value );
		else
			nlwarning( "Key %s in %s is not a float.", key, cf.getFilename().c_str() );
	}
	else
	{
		nlwarning( "Couldn't find key %s in %s.", key, cf.getFilename().c_str() );
	}
}

void CConfig::setString( const char *key, const std::string &value )
{
	NLMISC::CConfigFile::CVar *var = cf.getVarPtr( key );

	if( var != NULL )
	{
		if( var->Type == NLMISC::CConfigFile::CVar::T_STRING )
			var->setAsString( value );
		else
			nlwarning( "Key %s in %s is not a string.", key, cf.getFilename().c_str() );
	}
	else
	{
		nlwarning( "Couldn't find key %s in %s.", key, cf.getFilename().c_str() );
	}
}
