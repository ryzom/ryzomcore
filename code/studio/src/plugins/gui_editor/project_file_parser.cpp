// Object Viewer Qt GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#include "project_file_parser.h"
#include "nel/misc/debug.h"

namespace GUIEditor
{
	CProjectFileParser::CProjectFileParser()
	{
		loaded = false;
	}

	CProjectFileParser::~CProjectFileParser()
	{
	}

	bool CProjectFileParser::parseProjectFile( std::string &name )
	{
		QFile file( name.c_str() );
		if( !file.open( QIODevice::ReadOnly ) )
			return false;

		if( !parseXMLFile( file ) )
		{
			file.close();
			return false;
		}

		file.close();
		
		return true;
	}

	void CProjectFileParser::getProjectFiles( SProjectFiles &projectFiles ) const
	{
		if( !loaded )
			return;

		projectFiles.guiFiles.resize( files.guiFiles.size() );
		projectFiles.mapFiles.resize( files.mapFiles.size() );
		std::copy( files.guiFiles.begin(), files.guiFiles.end(), projectFiles.guiFiles.begin() );
		std::copy( files.mapFiles.begin(), files.mapFiles.end(), projectFiles.mapFiles.begin() );
		projectFiles.projectName = files.projectName;
		projectFiles.masterGroup = files.masterGroup;
		projectFiles.activeGroup = files.activeGroup;
	}

	unsigned long CProjectFileParser::getProjectVersion() const
	{
		if( !loaded )
			return SProjectFiles::OLD;

		return files.version;
	}

	void CProjectFileParser::clear()
	{
		files.projectName = "";
		files.version = SProjectFiles::OLD;
		files.activeGroup = "";
		files.guiFiles.clear();
		files.mapFiles.clear();
	}

	bool CProjectFileParser::parseXMLFile(QFile &f)
	{
		loaded = false;

		QXmlStreamReader reader;
		reader.setDevice( &f );

		reader.readNext();
		if( reader.atEnd() )
			return false;

		while( !reader.atEnd() && !( reader.isStartElement() && ( reader.name() == "project" ) ) )
			reader.readNext();
		if( reader.atEnd() )
			return false;

		while( !reader.atEnd() && !( reader.isStartElement() && ( reader.name() == "header" ) ) )
			reader.readNext();
		if( reader.atEnd() )
			return false;

		if( !parseHeader( reader ) )
			return false;

		if( !parseGUIFiles( reader ) )
			return false;

		if( !parseMapFiles( reader ) )
			return false;
		
		loaded = true;
		return true;
	}

	bool CProjectFileParser::parseHeader( QXmlStreamReader &reader )
	{
		while( !reader.atEnd() && !( reader.isEndElement() && ( reader.name() == "header" ) ) )
		{
			if( reader.isStartElement() )
			{
				if( reader.name() == "name" )
				{
					QString name = reader.readElementText( QXmlStreamReader::ErrorOnUnexpectedElement );
					if( name.isEmpty() )
						return false;
					files.projectName = name.toUtf8().constData();
				}
				else
				if( reader.name() == "version" )
				{
					QString name = reader.readElementText( QXmlStreamReader::ErrorOnUnexpectedElement );
					if( name.isEmpty() )
						return false;
					files.version = static_cast< unsigned long >( name.toLong() );
				}
				else
				if( reader.name() == "mastergroup" )
				{
					QString mg = reader.readElementText( QXmlStreamReader::ErrorOnUnexpectedElement );
					if( mg.isEmpty() )
						return false;
					files.masterGroup = mg.toUtf8().constData();
				}
				else
				if( reader.name() == "activegroup" )
				{
					QString ag = reader.readElementText( QXmlStreamReader::ErrorOnUnexpectedElement );
					if( ag.isEmpty() )
						return false;
					files.activeGroup = ag.toUtf8().constData();
				}
			}

			reader.readNext();
		}
		if( reader.atEnd() )
			return false;

		return true;
	}

	bool CProjectFileParser::parseGUIFiles( QXmlStreamReader &reader )
	{
		while( !reader.atEnd() && !( reader.isStartElement() && reader.name() == "guifiles" ) )
			reader.readNext();
		if( reader.atEnd() )
			return false;

		while( !reader.atEnd() && !( reader.isEndElement() && ( reader.name() == "guifiles" ) ) )
		{
			if( reader.isStartElement() && ( reader.name() == "file" ) )
			{
				QString fileName = reader.readElementText( QXmlStreamReader::ErrorOnUnexpectedElement );
				if( fileName.isEmpty() )
					return false;
				files.guiFiles.push_back( fileName.toUtf8().constData() );

			}

			reader.readNext();
		}
		reader.readNext();
		if( reader.atEnd() )
			return false;

		if( files.guiFiles.empty() )
			return false;

		return true;
	}

	bool CProjectFileParser::parseMapFiles( QXmlStreamReader &reader )
	{
		while( !reader.atEnd() && !( reader.isStartElement() && reader.name() == "mapfiles" ) )
			reader.readNext();
		if( reader.atEnd() )
			return false;

		while( !reader.atEnd() && !( reader.isEndElement() && ( reader.name() == "mapfiles" ) ) )
		{
			if( reader.isStartElement() && ( reader.name() == "file" ) )
			{
				QString fileName = reader.readElementText( QXmlStreamReader::ErrorOnUnexpectedElement );
				if( fileName.isEmpty() )
					return false;
				files.mapFiles.push_back( fileName.toUtf8().constData() );

			}

			reader.readNext();
		}
		if( files.mapFiles.empty() )
		{
			nlinfo( "No map file(s) specified in project file." );
		}

		return true;
	}
}



