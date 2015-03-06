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

#ifndef GEORGES_DOCK_WIDGET
#define GEORGES_DOCK_WIDGET

#include <QDockWidget>

class QUndoStack;

class GeorgesDockWidget : public QDockWidget
{
	Q_OBJECT
public:
	GeorgesDockWidget( QWidget *parent = NULL );
	~GeorgesDockWidget();

	void setUndoStack( QUndoStack *stack ){ m_undoStack = stack; }

	bool isModified() const{ return m_modified; }
	void setModified( bool b ){ m_modified = b; }

	QString fileName() const{ return m_fileName; }

	virtual bool load( const QString &fileName ) = 0;
	virtual void write() = 0;

protected:
	void closeEvent( QCloseEvent *e );

Q_SIGNALS:
	void closing( GeorgesDockWidget *d );

protected:
	QString buildLogMsg( const QString &msg );
	virtual void log( const QString &msg ) = 0;

	QString m_fileName;
	bool m_modified;
	QUndoStack *m_undoStack;
};

#endif
