#include "translation_manager_constants.h"
#include "uxt_editor.h"

#include <QTableWidget>
#include <QFormLayout>
#include <QCloseEvent>
#include <QMessageBox>

#include "nel/misc/diff_tool.h"

namespace TranslationManager
{

class UXTEditorPvt
{
public:

	UXTEditorPvt()
	{
		t = new QTableWidget();
		changed = false;
	}

	QTableWidget *t;
	std::vector< STRING_MANAGER::TStringInfo > infos;
	bool changed;
};


UXTEditor::UXTEditor( QMdiArea *parent ) :
CEditor( parent )
{
	editor_type = Constants::ED_UXT;
	setAttribute( Qt::WA_DeleteOnClose );

	d_ptr = new UXTEditorPvt();

	blockTableSignals( false );
}

UXTEditor::~UXTEditor()
{
	delete d_ptr;
	d_ptr = NULL;
}

void UXTEditor::open( QString filename )
{
	std::vector< STRING_MANAGER::TStringInfo > &infos = d_ptr->infos;
	
	infos.clear();
	STRING_MANAGER::loadStringFile( filename.toUtf8().constData(), infos, true );

	if( d_ptr->infos.size() == 0 )
		return;

	blockTableSignals( true );

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

	blockTableSignals( false );

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
	if( d_ptr->changed )
	{
		int reply = QMessageBox::question( this,
										tr( "Table changed" ),
										tr( "The table has changed. Would you like to save your changes?" ),
										QMessageBox::Yes,
										QMessageBox::No
										);

		if( reply == QMessageBox::Yes )
			save();

	}

	e->accept();
	close();
}

void UXTEditor::onCellChanged( int row, int column )
{
	QTableWidgetItem *item = d_ptr->t->item( row, column );
	STRING_MANAGER::TStringInfo &info = d_ptr->infos[ row ];

	if( column == 0 )
		info.Identifier = item->text().toUtf8().constData();
	else
	if( column == 1 )
		info.Text = item->text().toUtf8().constData();

	d_ptr->changed = true;
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

void UXTEditor::blockTableSignals( bool block )
{
	if( block )
		disconnect( d_ptr->t, SIGNAL( cellChanged( int, int ) ), this, SLOT( onCellChanged( int, int ) ) );
	else
		connect( d_ptr->t, SIGNAL( cellChanged( int, int ) ), this, SLOT( onCellChanged( int, int ) ) );
}

}
