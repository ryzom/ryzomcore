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

#include "expression_store.h"
#include "expression_info.h"
#include "expression_loader.h"
#include <QMap>
#include <QDir>
#include "gui_editor_config.h"

class ExpressionStorePvt
{
public:

	~ExpressionStorePvt()
	{
		qDeleteAll( expressions );
		expressions.clear();
	}


	QMap< QString, ExpressionInfo* > expressions;
};

ExpressionStore::ExpressionStore()
{
	m_pvt = new ExpressionStorePvt();
}

ExpressionStore::~ExpressionStore()
{
	delete m_pvt;
	m_pvt = NULL;
}


bool ExpressionStore::load()
{
    QDir d( EXPRESSIONS_DIR );
	if( !d.exists() )
		return false;

	QFileInfoList l = d.entryInfoList();
	QListIterator< QFileInfo > itr( l );
	if( !itr.hasNext() )
		return false;

	ExpressionLoader loader;

	while( itr.hasNext() )
	{
		const QFileInfo &info = itr.next();
		if( !info.isFile() )
			continue;

		if( info.suffix() != "xml" )
			continue;

		ExpressionInfo *expr = loader.load( info.absoluteFilePath() );
		if( expr == NULL )
			continue;

		m_pvt->expressions[ expr->name ] = expr;
	}

	return false;
}

void ExpressionStore::getExpressions( QList< const ExpressionInfo* > &l ) const
{
	l.clear();

	QMap< QString, ExpressionInfo* >::const_iterator itr = m_pvt->expressions.constBegin();
	while( itr != m_pvt->expressions.constEnd() )
	{
		l.push_back( itr.value() );
		++itr;
	}
}

const ExpressionInfo* ExpressionStore::getInfo( const QString &name )
{
	QMap< QString, ExpressionInfo* >::const_iterator itr = m_pvt->expressions.find( name );
	if( itr == m_pvt->expressions.end() )
		return NULL;
	else
		return itr.value();
}



