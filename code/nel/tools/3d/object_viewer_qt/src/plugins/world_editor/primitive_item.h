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

#ifndef PRIMITIVE_ITEM_H
#define PRIMITIVE_ITEM_H

// Project includes

// NeL includes
#include <nel/ligo/primitive.h>
#include <nel/ligo/primitive_class.h>

// Qt includes
#include <QList>
#include <QIcon>
#include <QVariant>

namespace WorldEditor
{
class RootPrimitiveNode;

/*
@class Node
@brief
@details
*/
class Node
{
public:

	enum NodeType
	{
		BasicNodeType,
		WorldEditNodeType,
		RootPrimitiveNodeType,
		LandscapeNodeType,
		PrimitiveNodeType,
		UserNodeType = 1024
	};

	Node();
	virtual ~Node();

	/// Remove child node from the child list.
	void removeChildNode(Node *node);

	/// Insert node at the beginning of the list.
	void prependChildNode(Node *node);

	/// Insert node at the end of the list.
	void appendChildNode(Node *node);

	/// Insert node in front of the node pointed to by the pointer before.
	void insertChildNodeBefore(Node *node, Node *before);

	/// Insert node in back of the node pointed to by the pointer after.
	void insertChildNodeAfter(Node *node, Node *after);

	/// Insert node in pos
	void insertChildNode(int pos, Node *node);

	/// Return the node at index position row in the child list.
	Node *child(int row);

	/// Return the number of nodes in the list.
	int childCount() const;

	/// Return a row index this node.
	int row() const;

	/// Return a pointer to this node's parent item. If this node does not have a parent, 0 is returned.
	Node *parent();

	/// Set this node's custom data for the key key to value.
	void setData(int key, const QVariant &data);

	/// Return this node's custom data for the key key as a QVariant.
	QVariant data(int key) const;

	/// Return a type this node.
	virtual NodeType type() const;

private:
	Q_DISABLE_COPY(Node)

	Node *m_parent;
	QList<Node *> m_children;
	QHash<int, QVariant> m_data;
};

/*
@class WorldEditNode
@brief
@details
*/
class WorldEditNode: public Node
{
public:
	WorldEditNode(const QString &name);
	virtual ~WorldEditNode();

	void setContext(const QString &name);
	QString context() const;
	void setDataPath(const QString &path);
	QString dataPath() const;

	virtual NodeType type() const;

private:
	QString m_context;
	QString m_dataPath;
};

/*
@class LandscapeNode
@brief
@details
*/
class LandscapeNode: public Node
{
public:
	LandscapeNode(const QString &name, int id);
	virtual ~LandscapeNode();

	int id() const;
	QString fileName() const;

	virtual NodeType type() const;

private:

	QString m_fileName;
	int m_id;
};

/*
@class PrimitiveNode
@brief
@details
*/
class PrimitiveNode: public Node
{
public:
	explicit PrimitiveNode(NLLIGO::IPrimitive *primitive);
	virtual ~PrimitiveNode();

	NLLIGO::IPrimitive *primitive() const;
	const NLLIGO::CPrimitiveClass *primitiveClass() const;
	RootPrimitiveNode *rootPrimitiveNode();

	virtual NodeType type() const;

private:
	NLLIGO::IPrimitive *m_primitive;
};

/*
@class RootPrimitiveNode
@brief
@details
*/
class RootPrimitiveNode: public PrimitiveNode
{
public:
	RootPrimitiveNode(const QString &name, NLLIGO::CPrimitives *primitives);
	virtual ~RootPrimitiveNode();

	NLLIGO::CPrimitives *primitives() const;

	void setFileName(const QString &fileName);
	QString fileName() const;
	virtual NodeType type() const;

private:

	QString m_fileName;
	NLLIGO::CPrimitives *m_primitives;
};

typedef QList<Node *> NodeList;

} /* namespace WorldEditor */

// Enable the use of QVariant with this class.
Q_DECLARE_METATYPE(WorldEditor::Node *)

#endif // PRIMITIVE_ITEM_H
