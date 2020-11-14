// Ryzom Core Studio - GUI Editor Plugin
//
// Copyright (C) 2014 Laszlo Kis-Adam
// Copyright (C) 2010 Ryzom Core <http://ryzomcore.org/>
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

#include "expression_loader.h"
#include "expression_info.h"

#include <QFile>
#include <QXmlStreamReader>

class ExpressionLoaderPvt
{
public:

	bool parseName()
	{
		QString text = reader.readElementText( QXmlStreamReader::ErrorOnUnexpectedElement );
		if( reader.hasError() )
			return false;

		m_info->name = text;

		return true;
	}

	bool parseValue()
	{
		QString text = reader.readElementText( QXmlStreamReader::ErrorOnUnexpectedElement );
		if( reader.hasError() )
			return false;

		if( text.toLower() == "true" )
			m_info->value = true;
		else
			m_info->value = false;

		return true;
	}

	bool parseCategory()
	{
		QString text = reader.readElementText( QXmlStreamReader::ErrorOnUnexpectedElement );
		if( reader.hasError() )
			return false;

		m_info->category = text;

		return true;
	}

	bool parseVariable()
	{
		QString text = reader.readElementText( QXmlStreamReader::ErrorOnUnexpectedElement );
		if( reader.hasError() )
			return false;

		if( text.toLower() == "true" )
			m_info->variable = true;
		else
			m_info->variable = false;

		return true;
	}

	bool parseSlot()
	{
		QString text = reader.readElementText( QXmlStreamReader::ErrorOnUnexpectedElement );
		if( reader.hasError() )
			return false;

		m_info->slotNames.push_back( text );

		return true;
	}

	bool parseSlots()
	{
		bool error = false;

		while( !reader.atEnd() )
		{
			reader.readNext();

			if( reader.isStartElement() )
			{
				QString name = reader.name().toString();
				if( name == "slot" )
					error = !parseSlot();
			}
			else
			if( reader.isEndElement() )
			{
				if( reader.name() == "slots" )
					break;
			}
		}

		if( reader.atEnd() )
			return false;

		return true;
	}

	bool load( QFile *f )
	{
		reader.clear();
		reader.setDevice( f );

		bool error = false;

		// start of document
		reader.readNext();
		if( reader.atEnd() )
			return false;

		// root node
		reader.readNext();
		if( reader.atEnd() )
			return false;

		if( reader.isStartElement() )
		{
			// Not an expression file?
			if( reader.name() != "expression" )
				return false;
		}

		while( !reader.atEnd() )
		{
			reader.readNext();

			if( reader.isStartElement() )
			{
				QString name = reader.name().toString();

				if( name == "name" )
					error = !parseName();
				else
				if( name == "value" )
					error = !parseValue();
				else
				if( name == "category" )
					error = !parseCategory();
				else
				if( name == "variable" )
					error = !parseVariable();
				else
				if( name == "slots" )
					error = !parseSlots();
			}

			if( error )
				break;
		}

		if( error || reader.hasError() )
		{
			return false;
		}

		return true;
	}

	void setInfo( ExpressionInfo *info ){ m_info = info; }

private:
	QXmlStreamReader reader;
	ExpressionInfo *m_info;
};

ExpressionLoader::ExpressionLoader()
{
	m_pvt = new ExpressionLoaderPvt;
}

ExpressionLoader::~ExpressionLoader()
{
	delete m_pvt;
	m_pvt = NULL;
}

ExpressionInfo* ExpressionLoader::load( const QString &filename )
{
	ExpressionInfo *info = NULL;
	bool ok = false;

	QFile f( filename );
	if( !f.open( QIODevice::ReadOnly | QIODevice::Text ) )
		return NULL;

	info = new ExpressionInfo();
	m_pvt->setInfo( info );
	ok = m_pvt->load( &f );

	f.close();

	if( !ok )
	{
		delete info;
		info = NULL;
	}

	return info;
}


