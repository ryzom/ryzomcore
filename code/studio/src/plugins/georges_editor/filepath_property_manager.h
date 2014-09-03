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

#ifndef FILEPATH_PROPERTY_MANAGER
#define FILEPATH_PROPERTY_MANAGER

#define QT_QTPROPERTYBROWSER_IMPORT 

#include <QWidget>
#include <3rdparty/qtpropertybrowser/qtpropertymanager.h>

/////////////////////////////////////////////////////////////////////// Manager /////////////////////////////////////////////////////////////////

class FileManagerPvt;

class FileManager : public QtAbstractPropertyManager
{
	Q_OBJECT
public:
	FileManager( QObject *parent = NULL );
	~FileManager();

	QString value( const QtProperty *p ) const;

public Q_SLOTS:
	void setValue( QtProperty *p, const QString &v );

Q_SIGNALS:
	void valueChanged( QtProperty *p, const QString &v );

protected:
	bool hasValue( const QtProperty *p ) const;
	QString valueText( const QtProperty *p ) const;
	void initializeProperty( QtProperty *p );
	void uninitializeProperty( QtProperty *p );

private:
	FileManagerPvt *m_pvt;

	Q_DISABLE_COPY( FileManager );
};



//////////////////////////////////////////////////////// Factory ///////////////////////////////////////////////////////////////////

class FileEditFactoryPvt;

class FileEditFactory : public QtAbstractEditorFactory< FileManager >
{
	Q_OBJECT
public:
	FileEditFactory( QObject *parent = NULL );
	~FileEditFactory();

protected:
	void connectPropertyManager( FileManager *manager );
	void disconnectPropertyManager( FileManager *manager );
	QWidget* createEditor( FileManager *manager, QtProperty *p, QWidget *parent );
	
private Q_SLOTS:
	void onPropertyChanged( QtProperty *p, const QString &value );
	void onSetValue( const QString &value );
	void onEditorDestroyed( QObject *editor );

private:
	FileEditFactoryPvt *m_pvt;

	Q_DISABLE_COPY( FileEditFactory );
};


//////////////////////////////////////////////////////// Editor ////////////////////////////////////////////////////////////////////

class FileEditPvt;

class FileEdit : public QWidget
{
	Q_OBJECT
public:
	FileEdit( QWidget *parent = NULL );
	~FileEdit();

	void setValue( const QString &value );

Q_SIGNALS:
	void valueChanged( const QString &value );
	void destroyed( QObject *editor );

private Q_SLOTS:
	void onButtonClicked();

private:
	void setupUI();
	void setupConnections();
	FileEditPvt *m_pvt;

	Q_DISABLE_COPY( FileEdit );
};

#endif
