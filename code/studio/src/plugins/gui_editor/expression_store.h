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

#ifndef EXPRESSION_STORE
#define EXPRESSION_STORE

#include <QString>
#include <QList>
#include "expression_info.h"

//struct ExpressionInfo;
class ExpressionStorePvt;

class ExpressionStore
{
public:
	ExpressionStore();
	~ExpressionStore();

	bool load();

	void getExpressions( QList< const ExpressionInfo* > &l ) const;

	const ExpressionInfo* getInfo( const QString &name );

private:
	ExpressionStorePvt *m_pvt;
};

#endif

