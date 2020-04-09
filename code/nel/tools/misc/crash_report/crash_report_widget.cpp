// Nel MMORPG framework - Error Reporter
//
// Copyright (C) 2015  Laszlo KIS-ADAM (dfighter) <dfighter1985@gmail.com>
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010-2015  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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


#include "crash_report_widget.h"
#include "crash_report_socket.h"
#include "crash_report_data.h"
#include <QTimer>
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QFile>
#include <QPushButton>
#include <QHBoxLayout>
#include <QCheckBox>

CCrashReportWidget::CCrashReportWidget( QWidget *parent ) :
QWidget( parent )
{
	m_developerMode = false;
	m_forceSend = false;
	m_devSendReport = false;
	m_returnValue = ERET_NULL;

	m_ui.setupUi( this );

	m_socket = new CCrashReportSocket( this );

	QTimer::singleShot( 1, this, SLOT( onLoad() ) );

	connect( m_ui.emailCB, SIGNAL( stateChanged( int ) ), this, SLOT( onCBClicked() ) );

	connect( m_socket, SIGNAL( reportSent() ), this, SLOT( onReportSent() ) );
	connect( m_socket, SIGNAL( reportFailed() ), this, SLOT( onReportFailed() ) );
}

CCrashReportWidget::~CCrashReportWidget()
{
	m_socket = NULL;
}

void CCrashReportWidget::setup( const std::vector< std::pair< std::string, std::string > > &params )
{
	for(uint i = 0; i < params.size(); ++i)
	{
		const std::pair< std::string, std::string > &p = params[i];
		const std::string &k = p.first;
		const std::string &v = p.second;

		if( k == "log" )
		{
			m_fileName = v.c_str();
			if( !QFile::exists( m_fileName ) )
				m_fileName.clear();
		}
		else
		if( k == "host" )
		{
			m_socket->setURL( v.c_str() );
		}
		else
		if( k == "title" )
		{
			setWindowTitle( v.c_str() );
		}
		else
		if( k == "dev" )
		{
			m_developerMode = true;
		}
		else
		if( k == "sendreport" )
		{
			m_forceSend = true;
		}
	}

	if( m_fileName.isEmpty() )
	{
		m_ui.reportLabel->hide();
		m_ui.reportEdit->hide();
	}

	
	if( m_socket->url().isEmpty() || m_fileName.isEmpty() )
	{
		m_ui.descriptionEdit->hide();
		m_ui.emailCB->hide();
		m_ui.emailEdit->hide();
		m_ui.descrLabel->hide();
	}

	QHBoxLayout *hbl = new QHBoxLayout( this );

	if( m_developerMode )
	{
		if( !m_socket->url().isEmpty() && !m_fileName.isEmpty() )
		{
			m_ui.emailCB->setEnabled( false );

			QCheckBox *cb = new QCheckBox( tr( "Send report" ), this );
			m_ui.gridLayout->addWidget( cb, 4, 0, 1, 1 );

			m_ui.gridLayout->addWidget( m_ui.emailCB, 5, 0, 1, 1 );
			m_ui.gridLayout->addWidget( m_ui.emailEdit, 6, 0, 1, 1 );

			connect(cb, SIGNAL(stateChanged(int)), this, SLOT(onSendCBClicked()));
			if (m_forceSend)
				cb->setChecked(true);
		}

		hbl->addStretch();

		QPushButton *alwaysIgnoreButton = new QPushButton( tr( "Always Ignore" ), this );
		QPushButton *ignoreButton = new QPushButton( tr( "Ignore" ), this );
		QPushButton *abortButton = new QPushButton( tr( "Abort" ), this );
		QPushButton *breakButton = new QPushButton(tr("Break"), this);
		breakButton->setAutoDefault(true);

		hbl->addWidget( alwaysIgnoreButton );
		hbl->addWidget( ignoreButton );
		hbl->addWidget( abortButton );
		hbl->addWidget( breakButton );

		m_ui.gridLayout->addLayout( hbl, 7, 0, 1, 3 );

		connect( alwaysIgnoreButton, SIGNAL( clicked( bool ) ), this, SLOT( onAlwaysIgnoreClicked() ) );
		connect( ignoreButton, SIGNAL( clicked( bool ) ), this, SLOT( onIgnoreClicked() ) );
		connect( abortButton, SIGNAL( clicked( bool ) ), this, SLOT( onAbortClicked() ) );
		connect( breakButton, SIGNAL( clicked( bool ) ), this, SLOT( onBreakClicked() ) );
	}
	else
	{
		hbl->addStretch();

		// If -host is specified, offer the send function
		if( !m_socket->url().isEmpty() && !m_fileName.isEmpty() )
		{
			if (!m_forceSend)
			{
				QPushButton *cancelButton = new QPushButton(tr("Don't send report"), this);
				connect(cancelButton, SIGNAL(clicked(bool)), this, SLOT(onCancelClicked()));
				hbl->addWidget(cancelButton);
			}

			QPushButton *sendButton = new QPushButton( tr( "Send report" ), this );
			sendButton->setAutoDefault(true);
			connect( sendButton, SIGNAL( clicked( bool ) ), this, SLOT( onSendClicked() ) );
			hbl->addWidget( sendButton );
		}
		// Otherwise only offer exit
		else
		{
			QPushButton *exitButton = new QPushButton( tr( "Exit" ), this );
			connect( exitButton, SIGNAL( clicked( bool ) ), this, SLOT( onCancelClicked() ) );
			hbl->addWidget(exitButton);
			exitButton->setAutoDefault(true);
		}

		m_ui.gridLayout->addLayout( hbl, 6, 0, 1, 3 );
	}
}

void CCrashReportWidget::onLoad()
{
	if( m_fileName.isEmpty() )
		return;

	QFile f( m_fileName );
	bool b = f.open( QFile::ReadOnly | QFile::Text );
	if( !b )
	{
		m_fileName.clear();
		return;
	}

	QTextStream ss( &f );
	ss.setCodec("UTF-8");
	m_ui.reportEdit->setPlainText( ss.readAll() );
	f.close();
}

void CCrashReportWidget::onSendClicked()
{
	if( m_developerMode && !m_devSendReport )
	{
		close();
		return;
	}

	if( m_socket->url().isEmpty() || m_fileName.isEmpty() )
	{
		close();
		return;
	}

	QApplication::setOverrideCursor( Qt::WaitCursor );

	SCrashReportData data;
	data.description = m_ui.descriptionEdit->toPlainText();
	data.report = m_ui.reportEdit->toPlainText();
	
	if( m_ui.emailCB->isChecked() )
		data.email = m_ui.emailEdit->text();

	m_socket->sendReport( data );
}

void CCrashReportWidget::onCancelClicked()
{
	removeAndQuit();
}

void CCrashReportWidget::onCBClicked()
{
	m_ui.emailEdit->setEnabled( m_ui.emailCB->isChecked() );
}

void CCrashReportWidget::onSendCBClicked()
{
	bool b = m_ui.emailCB->isEnabled();

	if( b )
	{
		m_ui.emailCB->setChecked( false );
	}

	m_ui.emailCB->setEnabled( !b );

	m_devSendReport = !m_devSendReport;
}

void CCrashReportWidget::onAlwaysIgnoreClicked()
{
	m_returnValue = ERET_ALWAYS_IGNORE;
	onSendClicked();
}

void CCrashReportWidget::onIgnoreClicked()
{
	m_returnValue = ERET_IGNORE;
	onSendClicked();
}

void CCrashReportWidget::onAbortClicked()
{
	m_returnValue = ERET_ABORT;
	onSendClicked();
}

void CCrashReportWidget::onBreakClicked()
{
	m_returnValue = ERET_BREAK;
	onSendClicked();
}


void CCrashReportWidget::onReportSent()
{
	QApplication::setOverrideCursor( Qt::ArrowCursor );

	QMessageBox::information( this,
								tr( "Report sent" ),
								tr( "The report has been sent." ) );

	removeAndQuit();
}

void CCrashReportWidget::onReportFailed()
{
	QApplication::setOverrideCursor( Qt::ArrowCursor );

	QMessageBox::information( this,
								tr( "Report failed" ),
								tr( "Failed to send the report..." ) );

	removeAndQuit();
}

void CCrashReportWidget::removeAndQuit()
{
	if( !m_fileName.isEmpty() )
		QFile::remove( m_fileName );

	close();
}

