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


