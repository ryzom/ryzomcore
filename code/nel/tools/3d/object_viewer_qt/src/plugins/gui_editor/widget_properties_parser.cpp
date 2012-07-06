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


#include "widget_properties_parser.h"
#include <QDir>
#include <QStringList>
#include "nel/misc/debug.h"

using namespace NLMISC;

namespace GUIEditor
{
	void CWidgetPropParser::parseGUIWidgets()
	{
		QDir d( "widgets" );
		if( !d.exists() )
		{
			nlwarning( "GUI widgets directory doesn't exist!" );
			return;
		}

		QStringList nameFilters;
		nameFilters.push_back( "*.xml" );

		QStringList files = d.entryList( nameFilters, QDir::Files );
		if( files.empty() )
		{
			nlwarning( "GUI widgets directory has no files!" );
			return;
		}

		QStringListIterator itr( files );
		while( itr.hasNext() )
			parseGUIWidget( "widgets/" + itr.next() );

		resolveInheritance();

		widgetInfo = NULL;
	}

	void CWidgetPropParser::parseGUIWidget( const QString &file )
	{
		QFile f( file );
		if( f.open( QIODevice::ReadOnly ) )
		{
			parseGUIWidgetXML( f );
			f.close();
		}
		else
			nlwarning( QString( "File %1 cannot be opened!" ).arg( file ).toStdString().c_str() );
	}

	void CWidgetPropParser::parseGUIWidgetXML( QFile &file )
	{
		QXmlStreamReader reader;
		reader.setDevice( &file );

		reader.readNext();
		if( reader.atEnd() )
			return;

		while( !reader.atEnd() && !( reader.isStartElement() && ( reader.name() == "widget" ) ) )
			reader.readNext();
		if( reader.atEnd() )
			return;

		while( !reader.atEnd() && !( reader.isStartElement() && ( reader.name() == "header" ) ) )
			reader.readNext();
		if( reader.atEnd() )
			return;

		QString name = parseGUIWidgetHeader( reader );
		if( name.isEmpty() )
		{
			nlwarning( "malformed XML." );
			return;
		}

		while( !reader.atEnd() && !( reader.isStartElement() && ( reader.name() == "properties" ) ) )
			reader.readNext();
		if( reader.atEnd() )
			return;

		parseGUIWidgetProperties( reader, name );
	}

	QString CWidgetPropParser::parseGUIWidgetHeader( QXmlStreamReader &reader )
	{
		reader.readNext();
		if( reader.atEnd() )
			return QString( "" );

		SWidgetInfo info;

		while( !reader.atEnd() && !( reader.isEndElement() && ( reader.name() == "header" ) ) )
		{
			if( reader.isStartElement() )
			{
				QString key = reader.name().toString();
				QString value = reader.readElementText( QXmlStreamReader::ErrorOnUnexpectedElement );

				if( !reader.hasError() )
				{
					if( key == "name" )
						info.name = value.toStdString();
					else
					if( key == "guiname" )
						info.GUIName = value.toStdString();
					else
					if( key == "ancestor" )
						info.ancestor = value.toStdString();
					else
					if( key == "description" )
						info.description = value.toStdString();
					else
					if( key == "icon" )
						info.icon == value.toStdString();
					else
					if( key == "abstract" )
					{
						info.isAbstract = false;
						if( value == "true" )
							info.isAbstract = true;
					}
					else
						nlwarning( "Malformed XML." );
				}
			}
			
			reader.readNext();
		}
		if( reader.atEnd() )
			return QString( "" );
		if( info.name.empty() )
			return QString( "" );

		(*widgetInfo)[ info.name.c_str() ] = info;
		return QString( info.name.c_str() );
	}

	void CWidgetPropParser::parseGUIWidgetProperties( QXmlStreamReader &reader, const QString &widgetName )
	{
		reader.readNext();
		if( reader.atEnd() )
			return;

		std::map< std::string, SWidgetInfo >::iterator itr =
			widgetInfo->find( widgetName.toStdString() );
		if( itr == widgetInfo->end() )
			return;

		std::vector< SPropEntry > &v = itr->second.props;

		while( !reader.atEnd() && !( reader.isEndElement() && ( reader.name() == "properties" ) ) )
		{
			if( reader.isStartElement() && reader.name() == "property" )
			{
				SPropEntry prop;
				reader.readNext();

				while( !reader.atEnd() && !( reader.isEndElement() && ( reader.name() == "property" ) ) )
				{
					if( reader.isStartElement() )
					{
						QString key = reader.name().toString();
						QString value = reader.readElementText( QXmlStreamReader::ErrorOnUnexpectedElement );

						if( !reader.hasError() )
						{
							if( key == "name" )
								prop.propName = value.toStdString();
							else
							if( key == "type" )
								prop.propType = value.toStdString();
							else
							if( key == "default" )
								prop.propDefault = value.toStdString();
							else
								nlwarning( QString( "Unknown tag %1 within a property" ).arg( key ).toStdString().c_str() );

						}
						else
							nlwarning( "Malformed XML." );
					}
					
					reader.readNext();
				}
				if( reader.atEnd() )
					return;
				
				v.push_back( prop );
			}

			reader.readNext();
		}
	}

	bool propCompare( const SPropEntry &left, const SPropEntry &right )
	{
		return left.propName < right.propName;
	}

	void CWidgetPropParser::resolveInheritance()
	{
		for( std::map< std::string, SWidgetInfo >::iterator itr = widgetInfo->begin(); itr != widgetInfo->end(); ++itr )
		{
			resolveInheritanceFor( itr->first );
			std::sort( itr->second.props.begin(), itr->second.props.end(), propCompare );
		}
		
	}

	void CWidgetPropParser::resolveInheritanceFor( const std::string name )
	{
		if( name.empty() )
			return;

		std::map< std::string, SWidgetInfo >::iterator itr =
			widgetInfo->find( name );
		if( itr == widgetInfo->end() )
			return;

		SWidgetInfo *info = &(itr->second);
		if( info->resolved )
			return;

		if( info->ancestor.empty() )
			return;

		std::vector< SPropEntry > &props = info->props;

		std::map< std::string, SWidgetInfo >::iterator itr2 =
			widgetInfo->find( info->ancestor );
		if( itr2 == widgetInfo->end() )
			return;
		SWidgetInfo *info2 = &(itr2->second);

		do
		{
			for( std::vector< SPropEntry >::iterator propItr = info2->props.begin(); propItr != info2->props.end(); ++propItr )
				props.push_back( *propItr );

			if( !info2->resolved && !info2->ancestor.empty() )
			{
				itr2 = widgetInfo->find( info2->ancestor );
				if( itr2 != widgetInfo->end() )
					info2 = &(itr2->second);
				else
					info2 = NULL;				
			}
		}
		while( ( info2 != NULL ) && !info2->resolved && !info2->ancestor.empty() );

		info->resolved = true;
	}
}


