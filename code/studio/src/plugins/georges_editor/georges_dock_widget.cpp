// Ryzom Core Studio - Georges Editor Plugin
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

#include "georges_dock_widget.h"
#include <QMessageBox>

GeorgesDockWidget::GeorgesDockWidget( QWidget *parent ) :
QDockWidget( parent )
{
	m_modified = false;
	m_undoStack = NULL;
}

GeorgesDockWidget::~GeorgesDockWidget()
{
}

void GeorgesDockWidget::closeEvent( QCloseEvent *e )
{
	if( isModified() )
	{
		QString title = windowTitle();
		title.remove( '*' );

		int reply = QMessageBox::question( this,
											tr( "Closing dialog" ),
											title + tr( " has been modified.\nWould you like to save the changes?" ),
											QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel );

		if( reply == QMessageBox::Cancel )
		{
			e->ignore();
			return;
		}

		if( reply == QMessageBox::Yes )
			write();
	}

	Q_EMIT closing( this );
}

QString GeorgesDockWidget::buildLogMsg( const QString &msg )
{
	QString user = getenv( "USER" );
	if( user.isEmpty() )
		user = getenv( "USERNAME" );
	if( user.isEmpty() )
		user = "anonymous";
	
	QTime time = QTime::currentTime();
	QDate date = QDate::currentDate();
	
	QString dateString = date.toString( "ddd MMM dd" );
	QString timeString = time.toString( "HH:mm:ss" );
	
	QString logMsg;
	logMsg += dateString;
	logMsg += ' ';
	logMsg += timeString;
	logMsg += ' ';
	logMsg += QString::number( date.year() );
	logMsg += ' ';
	logMsg += "(";
	logMsg += user;
	logMsg += ")";
	logMsg += ' ';
	logMsg += msg;

	return logMsg;
}
