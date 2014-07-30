#include "add_widget_widget.h"
#include "widget_info_tree.h"
#include <vector>
#include <string>
#include <QMessageBox>

namespace GUIEditor
{

	AddWidgetWidget::AddWidgetWidget( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		setupConnections();
	}

	AddWidgetWidget::~AddWidgetWidget()
	{
	}

	void AddWidgetWidget::setCurrentGroup( const QString &g )
	{
		groupEdit->setText( g );
	}

	void AddWidgetWidget::setupWidgetInfo( const CWidgetInfoTree *tree )
	{
		std::vector< std::string > names;
		tree->getNames( names, false );

		widgetCB->clear();

		std::sort( names.begin(), names.end() );

		std::vector< std::string >::const_iterator itr = names.begin();
		while( itr != names.end() )
		{
			widgetCB->addItem( QString( itr->c_str() ) );
			++itr;
		}

	}

	void AddWidgetWidget::setupConnections()
	{
		connect( cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( close() ) );
		connect( addButton, SIGNAL( clicked( bool ) ), this, SLOT( onAddClicked() ) );
	}

	void AddWidgetWidget::onAddClicked()
	{
		if( groupEdit->text().isEmpty() )
		{
			QMessageBox::warning( NULL,
				tr( "WARNING" ),
				tr( "You need to be adding the new widget into a group!" ),
				QMessageBox::Ok );

			return;
		}

		if( nameEdit->text().isEmpty() )
		{
			QMessageBox::warning( NULL,
				tr( "WARNING" ),
				tr( "You need to specify a name for your new widget!" ),
				QMessageBox::Ok );

			return;
		}

		close();

		Q_EMIT adding( groupEdit->text(), widgetCB->currentText(), nameEdit->text() );
	}
}

