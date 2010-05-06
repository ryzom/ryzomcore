#include "qnelwindow.h"

#include <QSplitter>

#include "qnelwidget.h"

QNelWindow::QNelWindow (QWidget *parent, Qt::WFlags f) : QMainWindow (parent, f) 
{
	initWindow();
}

void QNelWindow::initWindow()
{

	/// Create ogre widget.

	/// Create our gui stuff
	QSplitter *splitter = new QSplitter (Qt::Vertical);
	//myWidget = new CameraTrackWidget (splitter);
	QNelWidget *nelWidget = new QNelWidget();
	splitter->addWidget((QWidget*)nelWidget);
	//myWidget2 = new CameraTrackWidget (splitter);
	//splitter->addWidget (myWidget2);

	nelWidget->show();
	//myWidget2->show();
	splitter->show();

	setCentralWidget (splitter);
}
