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

#ifndef GEORGES_TYP_DIALOG
#define GEORGES_TYP_DIALOG

#include "georges_dock_widget.h"
#include "ui_georges_typ_dialog.h"

class GeorgesTypDialogPvt;

class GeorgesTypDialog : public GeorgesDockWidget
{
	Q_OBJECT
public:
	GeorgesTypDialog( QWidget *parent = NULL );
	~GeorgesTypDialog();

	bool load( const QString &fileName );
	void write();
	void newDocument( const QString &fileName );

Q_SIGNALS:
	void modified();

private Q_SLOTS:
	void onAddClicked();
	void onRemoveClicked();

	void onItemChanged( QTreeWidgetItem *item, int column );
	void onModified();
	void onModified( const QString &k, const QString &v );

private:
	void setupConnections();
	void log( const QString &msg );
	void loadTyp();

	Ui::GeorgesTypDialog m_ui;
	GeorgesTypDialogPvt *m_pvt;

	QString m_fileName;
};


#endif

