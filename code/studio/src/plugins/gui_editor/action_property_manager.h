// Ryzom Core Studio GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#ifndef ACTION_PROPERTY_MANAGER
#define ACTION_PROPERTY_MANAGER

#define QT_QTPROPERTYBROWSER_IMPORT

#include "3rdparty/qtpropertybrowser/qtpropertybrowser.h"
#include <QString>

/////////////////////////////////////////////////////// Manager ///////////////////////////////////////////////////////////////////////////

struct ActionPropertyManagerPrivate;

class ActionPropertyManager : public QtAbstractPropertyManager
{
	Q_OBJECT
	
public:
	ActionPropertyManager( QObject *parent = NULL );
	~ActionPropertyManager();

	QString value( const QtProperty *p ) const;

public Q_SLOTS:
	void setValue( QtProperty *p, const QString &value );

Q_SIGNALS:
	void valueChanged( QtProperty *p, const QString &value );

protected:
	bool hasValue( const QtProperty *p ) const;
	QString valueText( const QtProperty *p ) const;
	void initializeProperty( QtProperty *p );
	void uninitializeProperty( QtProperty *p );

private:
	ActionPropertyManagerPrivate *d_ptr;

	Q_DISABLE_COPY( ActionPropertyManager );
};


////////////////////////////////////////////////////////////////// Factory /////////////////////////////////////////////////////////////////////////

struct ActionEditorFactoryPrivate;

class ActionEditorFactory : public QtAbstractEditorFactory< ActionPropertyManager >
{
	Q_OBJECT

public:
	ActionEditorFactory( QObject *parent = NULL );
	~ActionEditorFactory();

protected:
	void connectPropertyManager( ActionPropertyManager *manager );
	void disconnectPropertyManager( ActionPropertyManager *manager );

	QWidget* createEditor( ActionPropertyManager *manager, QtProperty *p, QWidget *parent );

private Q_SLOTS:
	void onPropertyChanged( QtProperty *p, const QString &value );
	void onSetValue( const QString &value );
	void onEditorDestroyed( QObject *editor );

private:
	ActionEditorFactoryPrivate *d_ptr;

	Q_DISABLE_COPY( ActionEditorFactory );
};


///////////////////////////////////////////////////////////////// Editor ///////////////////////////////////////////////////////////////////////////

class QLineEdit;
class QToolButton;

class ActionPropertyEditor : public QWidget
{
	Q_OBJECT
public:
	ActionPropertyEditor( QWidget *parent = NULL );
	~ActionPropertyEditor();

public Q_SLOTS:
	void setValue( const QString &value );

protected:
	void showEvent( QShowEvent *e );

private Q_SLOTS:
	void onToolButtonClicked();
	void onTextChanged( const QString &text );

Q_SIGNALS:
	void valueChanged( const QString &value );

private:
	void setupUi();
	void setupConnections();
	void disableConnections();


	QLineEdit *lineEdit;
	QToolButton *toolButton;
};

#endif

