// Nel MMORPG framework - Error Reporter
//
// Copyright (C) 2015 Laszlo Kis-Adam
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


#ifndef RCERROR_WIDGET
#define RCERROR_WIDGET


#include "ui_rcerror_widget.h"

class CRCErrorSocket;

class CRCErrorWidget : public QWidget
{
	Q_OBJECT
public:
	CRCErrorWidget( QWidget *parent = NULL );
	~CRCErrorWidget();

	void setFileName( const char *fn ){ m_fileName = fn; }

private Q_SLOTS:
	void onLoad();
	void onSendClicked();
	void onCancelClicked();
	void onCBClicked();
	
	void onReportSent();
	void onReportFailed();

private:
	Ui::RCErrorWidget m_ui;
	QString m_fileName;
	CRCErrorSocket *m_socket;
};

#endif

