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

namespace GUIEditor
{
	CProjectFileParser::CProjectFileParser()
	{
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

	void CProjectFileParser::getProjectFileNames( std::vector< std::string > &names ) const
	{
		names.resize( fileNames.size() );
		std::copy( fileNames.begin(), fileNames.end(), names.begin() );
	}

	bool CProjectFileParser::parseXMLFile(QFile &f)
	{
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

		if( !parseFiles( reader ) )
			return false;
		
		return true;
	}

	bool CProjectFileParser::parseHeader( QXmlStreamReader &reader )
	{
		while( !reader.atEnd() && !( reader.isStartElement() && ( reader.name() == "name" ) ) )
			reader.readNext();
		if( reader.atEnd() )
			return false;

		QString name = reader.readElementText( QXmlStreamReader::ErrorOnUnexpectedElement );
		if( name.isEmpty() )
			return false;
		projectName = name.toStdString();

		while( !reader.atEnd() && !( reader.isEndElement() ) && ( reader.name() == "header" ) )
			reader.readNext();
		if( reader.atEnd() )
			return false;

		return true;
	}

	bool CProjectFileParser::parseFiles( QXmlStreamReader &reader )
	{
		while( !reader.atEnd() && !( reader.isStartElement() && reader.name() == "files" ) )
			reader.readNext();
		if( reader.atEnd() )
			return false;

		while( !reader.atEnd() && !( reader.isEndElement() && ( reader.name() == "files" ) ) )
		{
			if( reader.isStartElement() && ( reader.name() == "file" ) )
			{
				QString fileName = reader.readElementText( QXmlStreamReader::ErrorOnUnexpectedElement );
				if( !fileName.isEmpty() )
					fileNames.push_back( fileName.toStdString() );
			}

			reader.readNext();
		}
		if( fileNames.empty() )
			return false;

		return true;
	}
}



