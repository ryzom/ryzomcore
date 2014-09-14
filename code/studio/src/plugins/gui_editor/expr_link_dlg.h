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


#ifndef EXPR_LINK_DLG
#define EXPR_LINK_DLG

#include <QDialog>
#include <QList>
#include "ui_expr_link_dlg.h"
#include "expr_slot_info.h"

class ExprLinkDlg : public QDialog
{
	Q_OBJECT
public:
	ExprLinkDlg( QWidget *parent = NULL );
	~ExprLinkDlg();

	void load( const QList< SlotInfo > &a, const QList< SlotInfo > &b, const QString &aname, const QString &bname );

	int getSlotA() const;
	int getSlotB() const;

private Q_SLOTS:
	void onOKClicked();
	void onCancelClicked();

private:
	Ui::ExprLinkDialog m_ui;
};

#endif

