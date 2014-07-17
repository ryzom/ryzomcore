#include "translation_manager_constants.h"
#include "uxt_editor.h"

#include <QTableWidget>
#include <QFormLayout>
#include <QCloseEvent>

#include "nel/misc/diff_tool.h"

namespace TranslationManager
{

class UXTEditorPvt
{
public:

	UXTEditorPvt()
	{
		t = new QTableWidget();
	}

	QTableWidget *t;
};


UXTEditor::UXTEditor( QMdiArea *parent ) :
CEditor( parent )
{
	editor_type = Constants::ED_UXT;
	setAttribute( Qt::WA_DeleteOnClose );

	d_ptr = new UXTEditorPvt();
}

UXTEditor::~UXTEditor()
{
	delete d_ptr;
	d_ptr = NULL;
}

void UXTEditor::open( QString filename )
{
	std::vector< STRING_MANAGER::TStringInfo > infos;
	STRING_MANAGER::loadStringFile( filename.toUtf8().constData(), infos, true );

	if( infos.size() == 0 )
		return;

	d_ptr->t->clear();
	d_ptr->t->setColumnCount( 2 );
	d_ptr->t->setRowCount( infos.size() );

	setHeaderText( "Id", "Text" );

	int i = 0;

	std::vector< STRING_MANAGER::TStringInfo >::const_iterator itr = infos.begin();
	while( itr != infos.end() )
	{
		const STRING_MANAGER::TStringInfo &info = *itr;

		QTableWidgetItem *name = new QTableWidgetItem( info.Identifier.c_str() );
		QTableWidgetItem *text1 = new QTableWidgetItem( info.Text.toUtf8().c_str() );
		
		d_ptr->t->setItem( i, 0, name );
		d_ptr->t->setItem( i, 1, text1 );
		
		++itr;
		i++;
	}

	d_ptr->t->resizeColumnsToContents();

	setWidget( d_ptr->t );
	setCurrentFile( filename );
}

void UXTEditor::save()
{
}

void UXTEditor::saveAs( QString filename )
{
}

void UXTEditor::activateWindow()
{
	showMaximized();
}


void UXTEditor::closeEvent( QCloseEvent *e )
{
	e->accept();
	close();
}

void UXTEditor::setHeaderText( const QString &id, const QString &text )
{
	QTableWidgetItem *h1 = new QTableWidgetItem( id );
	QTableWidgetItem *h2 = new QTableWidgetItem( text );
	h1->setTextAlignment( Qt::AlignLeft );
	h2->setTextAlignment( Qt::AlignLeft );
	d_ptr->t->setHorizontalHeaderItem( 0, h1 );
	d_ptr->t->setHorizontalHeaderItem( 1, h2 );
}

}
