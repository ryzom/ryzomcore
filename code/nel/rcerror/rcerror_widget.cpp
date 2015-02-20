// Ryzom Core MMORPG framework - Error Reporter
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


#include "rcerror_widget.h"
#include "rcerror_socket.h"
#include "rcerror_data.h"
#include <QTimer>
#include <QTextStream>
#include <QFile>
#include <QMessageBox>

RCErrorWidget::RCErrorWidget( QWidget *parent ) :
QWidget( parent )
{
	m_ui.setupUi( this );

	m_socket = new RCErrorSocket( this );

	QTimer::singleShot( 1, this, SLOT( onLoad() ) );

	connect( m_ui.sendButton, SIGNAL( clicked( bool ) ), this, SLOT( onSendClicked() ) );
	connect( m_ui.canceButton, SIGNAL( clicked( bool ) ), this, SLOT( onCancelClicked() ) );
	connect( m_ui.emailCB, SIGNAL( stateChanged( int ) ), this, SLOT( onCBClicked() ) );

	connect( m_socket, SIGNAL( reportSent() ), this, SLOT( onReportSent() ) );
}

RCErrorWidget::~RCErrorWidget()
{
	m_socket = NULL;
}

void RCErrorWidget::onLoad()
{
	QFile f( m_fileName );
	bool b = f.open( QFile::ReadOnly | QFile::Text );
	if( !b )
	{
		QMessageBox::information( this,
									tr( "No log file found" ),
									tr( "There was no log file found, therefore nothing to report. Exiting..." ) );
		close();
	}

	QTextStream ss( &f );
	m_ui.reportEdit->setPlainText( ss.readAll() );
	f.close();
}

void RCErrorWidget::onSendClicked()
{
	m_ui.sendButton->setEnabled( false );
	QApplication::setOverrideCursor( Qt::WaitCursor );

	RCErrorData data;
	data.description = m_ui.descriptionEdit->toPlainText();
	data.report = m_ui.reportEdit->toPlainText();
	data.email = m_ui.emailEdit->text();

	m_socket->sendReport( data );
}

void RCErrorWidget::onCancelClicked()
{
	close();
}

void RCErrorWidget::onCBClicked()
{
	m_ui.emailEdit->setEnabled( m_ui.emailCB->isChecked() );
}

void RCErrorWidget::onReportSent()
{
	QApplication::setOverrideCursor( Qt::ArrowCursor );

	QMessageBox::information( this,
								tr( "Report sent" ),
								tr( "The report has been sent." ) );

	close();
}

