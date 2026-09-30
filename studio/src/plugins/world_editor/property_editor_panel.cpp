// Ryzom Core Studio - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

// Project includes
#include "property_editor_panel.h"
#include "primitive_property_form.h"
#include "primitives_model.h"

// Qt includes
#include <QLabel>
#include <QSplitter>
#include <QVBoxLayout>

namespace WorldEditor
{

PropertyEditorPanel::PropertyEditorPanel(QWidget *parent)
	: QWidget(parent),
	  m_splitter(0),
	  m_note(0),
	  m_undoStack(0),
	  m_model(0),
	  m_scene(0)
{
	m_splitter = new QSplitter(Qt::Horizontal, this);
	m_splitter->setChildrenCollapsible(false);

	m_note = new QLabel(this);
	m_note->setWordWrap(true);
	m_note->setContentsMargins(4, 2, 4, 2);
	m_note->hide();

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->addWidget(m_note);
	layout->addWidget(m_splitter);

	setEditorCount(1);
}

void PropertyEditorPanel::setEditorCount(int count)
{
	while (m_editors.size() < count)
	{
		PrimitivePropertyForm *editor = new PrimitivePropertyForm(m_splitter);
		editor->setContext(m_undoStack, m_model, m_scene);
		m_splitter->addWidget(editor);
		m_editors.append(editor);
	}

	while (m_editors.size() > count)
	{
		PrimitivePropertyForm *editor = m_editors.takeLast();
		editor->setParent(0);
		delete editor;
	}
}

void PropertyEditorPanel::setContext(QUndoStack *undoStack, PrimitivesTreeModel *model,
									 WorldEditorScene *scene)
{
	if (m_model != 0)
		disconnect(m_model, SIGNAL(propertyChanged(Node *)), this, SLOT(refreshNode(Node *)));

	m_undoStack = undoStack;
	m_model = model;
	m_scene = scene;
	Q_FOREACH (PrimitivePropertyForm *editor, m_editors)
		editor->setContext(m_undoStack, m_model, m_scene);

	if (m_model != 0)
		connect(m_model, SIGNAL(propertyChanged(Node *)), this, SLOT(refreshNode(Node *)));
}

void PropertyEditorPanel::refreshNode(Node *node)
{
	Q_FOREACH (PrimitivePropertyForm *editor, m_editors)
	{
		if ((editor->node() == node) && !editor->isPushing())
			editor->setNode(node);
	}
}

void PropertyEditorPanel::clearProperties()
{
	setEditorCount(1);
	m_editors.first()->clear();
	m_note->hide();
}

void PropertyEditorPanel::updateSelection(Node *node)
{
	NodeList nodes;
	if (node != 0)
		nodes.append(node);

	updateSelection(nodes);
}

void PropertyEditorPanel::updateSelection(const NodeList &nodes)
{
	if (nodes.isEmpty())
	{
		clearProperties();
		return;
	}

	const int shown = qMin(nodes.size(), int(MAX_EDITORS));
	setEditorCount(shown);

	for (int i = 0; i < shown; ++i)
		m_editors.at(i)->setNode(nodes.at(i));

	if (nodes.size() > shown)
	{
		m_note->setText(tr("Showing %1 of %2 selected primitives.")
						.arg(shown).arg(nodes.size()));
		m_note->show();
	}
	else
	{
		m_note->hide();
	}
}

} /* namespace WorldEditor */
