#include "georges_dock_widget.h"

GeorgesDockWidget::GeorgesDockWidget( QWidget *parent ) :
QDockWidget( parent )
{
	m_modified = false;
	m_undoStack = NULL;
}

GeorgesDockWidget::~GeorgesDockWidget()
{
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
