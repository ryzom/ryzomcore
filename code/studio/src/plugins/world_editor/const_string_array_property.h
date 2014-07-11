// Ryzom Core Studio World Editor plugin <http://dev.ryzom.com/projects/ryzom/>
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


#ifndef CONST_STR_ARR_PROP_H
#define CONST_STR_ARR_PROP_H

#define QT_QTPROPERTYBROWSER_IMPORT

#include "3rdparty/qtpropertybrowser/qtpropertybrowser.h"
#include <QString>
#include <QStringList>

/////////////////////////////////////////////////////// Manager ///////////////////////////////////////////////////////////////////////////

struct ConstStrArrPropMgrPriv;

class ConstStrArrPropMgr : public QtAbstractPropertyManager
{
	Q_OBJECT
	
public:
	ConstStrArrPropMgr( QObject *parent = NULL );
	~ConstStrArrPropMgr();

	QString value( const QtProperty *p ) const;

public Q_SLOTS:
	void setValue( QtProperty *p, const QString &value );
	void setStrings( QtProperty *p, const QStringList &strings );

Q_SIGNALS:
	void valueChanged( QtProperty *p, const QString &value );
	void stringsChanged( QtProperty *p, const QStringList &strings );

protected:
	bool hasValue( const QtProperty *p ) const;
	QString valueText( const QtProperty *p ) const;
	void initializeProperty( QtProperty *p );
	void uninitializeProperty( QtProperty *p );

private:
	ConstStrArrPropMgrPriv *d_ptr;

	Q_DISABLE_COPY( ConstStrArrPropMgr );
};


////////////////////////////////////////////////////////////////// Factory /////////////////////////////////////////////////////////////////////////

struct ConstStrArrEditorFactoryPriv;

class ConstStrArrEditorFactory : public QtAbstractEditorFactory< ConstStrArrPropMgr >
{
	Q_OBJECT

public:
	ConstStrArrEditorFactory( QObject *parent = NULL );
	~ConstStrArrEditorFactory();

protected:
	void connectPropertyManager( ConstStrArrPropMgr *manager );
	void disconnectPropertyManager( ConstStrArrPropMgr *manager );

	QWidget* createEditor( ConstStrArrPropMgr *manager, QtProperty *p, QWidget *parent );

private Q_SLOTS:
	void onPropertyChanged( QtProperty *p, const QString &value );
	void onStringsChanged( QtProperty *p, const QStringList &strings );
	void onSetValue( const QString &value );
	void onEditorDestroyed( QObject *editor );

private:
	ConstStrArrEditorFactoryPriv *d_ptr;

	Q_DISABLE_COPY( ConstStrArrEditorFactory );
};


///////////////////////////////////////////////////////////////// Editor ///////////////////////////////////////////////////////////////////////////

class QLineEdit;
class QToolButton;

class ConstStrArrEditor : public QWidget
{
	Q_OBJECT
public:
	ConstStrArrEditor( QWidget *parent = NULL );
	~ConstStrArrEditor();
	void setStrings( const QStringList &strings );

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
	QStringList strings;
};

#endif

