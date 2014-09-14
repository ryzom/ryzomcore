// Ryzom Core Studio - Georges Editor Plugin
//
// Copyright (C) 2014 Laszlo Kis-Adam
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


#ifndef EXPRESSION_EDITOR
#define EXPRESSION_EDITOR

#include "ui_expression_editor.h"

class QGraphicsScene;

class ExpressionEditor : public QWidget
{
	Q_OBJECT
public:
	ExpressionEditor( QWidget *parent = NULL );
	~ExpressionEditor();

protected:
	void contextMenuEvent( QContextMenuEvent *e );

private Q_SLOTS:
	void onDeleteSelection();
	void onSelectionChanged();
	void onLinkItems();
	void onUnLinkItems();
	void addNode( int slotCount );
	void onAddNode1();
	void onAddNode2();
	void onAddNode3();

private:

	Ui::ExpressionEditor m_ui;
	QGraphicsScene *m_scene;

	int m_selectionCount;
	int m_nodeCount;
};

#endif

