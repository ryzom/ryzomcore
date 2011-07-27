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

// NeL includes
#include <nel/ligo/ligo_config.h>

// Qt includes
#include <QtCore/QStringList>
#include <QtCore/QFile>

namespace WorldEditor
{

BaseTreeItem::BaseTreeItem(BaseTreeItem *parent)
{
	m_parentItem = parent;
	m_itemData << QIcon() << "" << "" << "";
}

BaseTreeItem::BaseTreeItem(const QList<QVariant> &data, BaseTreeItem *parent)
{
	m_parentItem = parent;
	m_itemData = data;
}

BaseTreeItem::~BaseTreeItem()
{
	qDeleteAll(m_childItems);
}

void BaseTreeItem::appendChild(BaseTreeItem *item)
{
	m_childItems.append(item);
}

void BaseTreeItem::deleteChild(int row)
{
	delete m_childItems.takeAt(row);
}

BaseTreeItem *BaseTreeItem::child(int row)
{
	return m_childItems.value(row);
}

int BaseTreeItem::childCount() const
{
	return m_childItems.count();
}

int BaseTreeItem::columnCount() const
{
	return m_itemData.count();
}

QVariant BaseTreeItem::data(int column) const
{
	return m_itemData.value(column);
}

void BaseTreeItem::setData(int column, const QVariant &data)
{
	m_itemData[column] = data;
}

BaseTreeItem *BaseTreeItem::parent()
{
	return m_parentItem;
}

int BaseTreeItem::row() const
{
	if (m_parentItem)
		return m_parentItem->m_childItems.indexOf(const_cast<BaseTreeItem *>(this));

	return 0;
}

void BaseTreeItem::setModified(bool value)
{
	m_modified = value;
}

bool BaseTreeItem::isModified() const
{
	return m_modified;
}

PrimitiveItem::PrimitiveItem(NLLIGO::IPrimitive *primitive, BaseTreeItem *parent)
	: BaseTreeItem(parent),
	  m_primitive(primitive)
{
	setData(1, QString(m_primitive->getName().c_str()));
	setData(2, QString(m_primitive->getClassName().c_str()));

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
	setData(0, icon);

	setData(3, QString(className.c_str()));
}
/*
PrimitiveItem::PrimitiveItem(const PrimitiveItem &other)
{
}
*/
PrimitiveItem::~PrimitiveItem()
{
}

NLLIGO::IPrimitive *PrimitiveItem::primitive() const
{
	return m_primitive;
}

const NLLIGO::CPrimitiveClass *PrimitiveItem::primitiveClass() const
{
	return NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig->getPrimitiveClass(*m_primitive);
}


RootPrimitiveItem::RootPrimitiveItem(const QString &name, NLLIGO::CPrimitives *primitives, BaseTreeItem *parent)
	: PrimitiveItem(primitives->RootNode, parent),
	  m_primitives(primitives)
{
	setData(1, name);
}
/*
RootPrimitiveItem::RootPrimitiveItem(const RootPrimitiveItem &other)
{
}
*/
RootPrimitiveItem::~RootPrimitiveItem()
{
}

} /* namespace WorldEditor */
