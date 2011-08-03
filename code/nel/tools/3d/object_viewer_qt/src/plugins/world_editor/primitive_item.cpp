// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Dzmitry Kamiahin <dnk-88@tut.by>
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
#include "primitive_item.h"
#include "world_editor_misc.h"
#include "world_editor_constants.h"

#include "../landscape_editor/landscape_editor_constants.h"

// NeL includes
#include <nel/ligo/ligo_config.h>

// Qt includes
#include <QtCore/QStringList>
#include <QtCore/QFile>

namespace WorldEditor
{

Node::Node()
	: m_parent(0)
{
}

Node::~Node()
{
	if (m_parent)
		m_parent->removeChildNode(this);

	qDeleteAll(m_children);
	nlassert(m_children.isEmpty());
	m_data.clear();
}

void Node::prependChildNode(Node *node)
{
	// Node is already a child
	nlassert(!m_children.contains(node));

	// Node already has a parent
	nlassert(!m_children.contains(node));

	m_children.prepend(node);
	node->m_parent = this;
}

void Node::appendChildNode(Node *node)
{
	// Node is already a child
	nlassert(!m_children.contains(node));

	// Node already has a parent
	nlassert(!m_children.contains(node));

	m_children.append(node);
	node->m_parent = this;
}

void Node::insertChildNodeBefore(Node *node, Node *before)
{
	// Node is already a child
	nlassert(!m_children.contains(node));

	// Node already has a parent
	nlassert(!m_children.contains(node));

	int idx = before ? m_children.indexOf(before) : -1;
	if (idx == -1)
		m_children.append(node);
	else
		m_children.insert(idx, node);
	node->m_parent = this;
}

void Node::insertChildNodeAfter(Node *node, Node *after)
{
	// Node is already a child
	nlassert(!m_children.contains(node));

	// Node already has a parent
	nlassert(!m_children.contains(node));

	int idx = after ? m_children.indexOf(after) : -1;
	if (idx == -1)
		m_children.append(node);
	else
		m_children.insert(idx + 1, node);
	node->m_parent = this;
}

void Node::removeChildNode(Node *node)
{
	nlassert(m_children.contains(node));
	nlassert(node->parent() == this);

	m_children.removeOne(node);

	node->m_parent = 0;
}

Node *Node::child(int row)
{
	return m_children.at(row);
}

int Node::childCount() const
{
	return m_children.count();
}

QVariant Node::data(int key) const
{
	return m_data[key];
}

void Node::setData(int key, const QVariant &data)
{
	m_data[key] = data;
}

Node *Node::parent()
{
	return m_parent;
}

int Node::row() const
{
	if (m_parent)
		return m_parent->m_children.indexOf(const_cast<Node *>(this));

	return 0;
}

Node::NodeType Node::type() const
{
	return BasicNodeType;
}

WorldEditNode::WorldEditNode(const QString &name)
{
	setData(Qt::DisplayRole, name);
	setData(Qt::DecorationRole, QIcon(Constants::ICON_WORLD_EDITOR));
}

WorldEditNode::~WorldEditNode()
{
}

Node::NodeType WorldEditNode::type() const
{
	return WorldEditNodeType;
}

LandscapeNode::LandscapeNode(const QString &name)
{
	setData(Qt::DisplayRole, name);
	setData(Qt::DecorationRole, QIcon(LandscapeEditor::Constants::ICON_ZONE_ITEM));
}

LandscapeNode::~LandscapeNode()
{
}

Node::NodeType LandscapeNode::type() const
{
	return LandscapeNodeType;
}

PrimitiveNode::PrimitiveNode(NLLIGO::IPrimitive *primitive)
	: m_primitive(primitive)
{
	setData(Qt::DisplayRole, QString(m_primitive->getName().c_str()));
	setData(Qt::ToolTipRole, QString(m_primitive->getClassName().c_str()));

	std::string className;
	m_primitive->getPropertyByName("class", className);

	// Set Icon
	QString nameIcon = QString("./old_ico/%1.ico").arg(className.c_str());
	QIcon icon(nameIcon);
	if (!QFile::exists(nameIcon))
	{
		if (primitive->getParent() == NULL)
			icon = QIcon("./old_ico/root.ico");
		else if (primitive->getNumChildren() == 0)
			icon = QIcon("./old_ico/property.ico");
		else
			icon = QIcon("./old_ico/folder_h.ico");
	}
	setData(Qt::DecorationRole, icon);

	//setData(3, QString(className.c_str()));
}

PrimitiveNode::~PrimitiveNode()
{
}

NLLIGO::IPrimitive *PrimitiveNode::primitive() const
{
	return m_primitive;
}

const NLLIGO::CPrimitiveClass *PrimitiveNode::primitiveClass() const
{
	return NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig->getPrimitiveClass(*m_primitive);
}

RootPrimitiveNode *PrimitiveNode::rootPrimitiveNode()
{
	Node *node = this;
	while (node && (node->type() != Node::RootPrimitiveNodeType))
		node = node->parent();
	return (RootPrimitiveNode *)node;
}

Node::NodeType PrimitiveNode::type() const
{
	return PrimitiveNodeType;
}

RootPrimitiveNode::RootPrimitiveNode(const QString &name, NLLIGO::CPrimitives *primitives)
	: PrimitiveNode(primitives->RootNode),
	  m_primitives(primitives)
{
	setData(Qt::DisplayRole, name);
}

RootPrimitiveNode::~RootPrimitiveNode()
{
}

NLLIGO::CPrimitives *RootPrimitiveNode::primitives() const
{
	return m_primitives;
}

Node::NodeType RootPrimitiveNode::type() const
{
	return RootPrimitiveNodeType;
}

} /* namespace WorldEditor */
