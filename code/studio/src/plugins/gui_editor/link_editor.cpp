// Object Viewer Qt GUI Editor plugin <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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


#include "link_editor.h"
#include "nel/gui/interface_group.h"
#include "nel/gui/widget_manager.h"
#include "expression_editor.h"
#include <QMenu>

namespace GUIEditor
{
	class LinkEditorPvt
	{
	public:

		LinkEditorPvt()
		{
			ee = new ExpressionEditor();
			ee->load();
		}

		~LinkEditorPvt()
		{
			delete ee;
			ee = NULL;
		}

		ExpressionEditor *ee;
	};


	LinkEditor::LinkEditor( QWidget *parent ) :
	QWidget( parent )
	{
		setupUi( this );
		setup();

		m_pvt = new LinkEditorPvt();

		connect( okButton, SIGNAL( clicked( bool ) ), this, SLOT( onOKButtonClicked() ) );
		connect( cancelButton, SIGNAL( clicked( bool ) ), this, SLOT( hide() ) );
		connect( expressionEdit, SIGNAL( customContextMenuRequested( const QPoint& ) ), this, SLOT( onTextEditContextMenu( const QPoint& ) ) );
		connect( m_pvt->ee, SIGNAL( closing() ), this, SLOT( onEEClosing() ) );
	}

	LinkEditor::~LinkEditor()
	{
		delete m_pvt;
		m_pvt = NULL;
	}

	void LinkEditor::setup()
	{
		expressionEdit->clear();
		targetEdit->clear();
		ahEdit->clear();
		ahParamEdit->clear();
		condEdit->clear();
	}

	void LinkEditor::setLinkId( uint32 linkId )
	{
		setup();
		currentLinkId = linkId;

		IParser *parser = CWidgetManager::getInstance()->getParser();
		SLinkData data;

		if( !parser->getLinkData( currentLinkId, data ) )
			return;

		expressionEdit->setPlainText( data.expr.c_str() );
		targetEdit->setText( data.target.c_str() );
		ahEdit->setText( data.action.c_str() );
		ahParamEdit->setText( data.params.c_str() );
		condEdit->setText( data.cond.c_str() );
	}

	void LinkEditor::clear()
	{
		expressionEdit->clear();
		targetEdit->clear();
		ahEdit->clear();
		ahParamEdit->clear();
		condEdit->clear();
	}

	void LinkEditor::onOKButtonClicked()
	{
		IParser *parser = CWidgetManager::getInstance()->getParser();
		SLinkData data;

		if( !parser->getLinkData( currentLinkId, data ) )
			return;

		data.expr = expressionEdit->toPlainText().toUtf8().constData();		
		data.target = targetEdit->text().toUtf8().constData();
		data.action = ahEdit->text().toUtf8().constData();
		data.params = ahParamEdit->text().toUtf8().constData();
		data.cond   = condEdit->text().toUtf8().constData();
		parser->updateLinkData( data.id, data );

		Q_EMIT okClicked();

		hide();
	}

	void LinkEditor::onTextEditContextMenu( const QPoint &pos )
	{
		QMenu *menu = expressionEdit->createStandardContextMenu();
		QAction *a = menu->addAction( "Expression Editor" );
		connect( a, SIGNAL( triggered() ), this, SLOT( onEE() ) );

		menu->exec( mapToGlobal( pos ) );
	}

	void LinkEditor::onEE()
	{
		m_pvt->ee->show();
	}

	void LinkEditor::onEEClosing()
	{
		expressionEdit->setPlainText( m_pvt->ee->result() );
	}
}
