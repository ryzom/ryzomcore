/*
Georges Editor Qt
Copyright (C) 2010 Adrian Jaekel <aj at elane2k dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "georgesform_model.h"

// NeL includes
#include <nel/misc/types_nl.h>
#include <nel/misc/rgba.h>
#include <nel/misc/path.h>
#include <nel/georges/u_form_elm.h>
#include <nel/georges/u_type.h>
#include <nel/georges/u_form_dfn.h>

// Qt includes
#include <QColor>
#include <QBrush>
#include <QApplication>
#include <QStyle>
#include <QDebug>

// project includes
#include "formitem.h"

using namespace NLGEORGES;

namespace NLQT 
{

	CGeorgesFormModel::CGeorgesFormModel(UFormElm *rootElm, QMap< QString, QStringList> deps,
		QString comment, QStringList parents, QObject *parent) : QAbstractItemModel(parent) 
	{
		QList<QVariant> rootData;
		rootData << "Value" << "Data" << "Extra" << "Type";
		_rootElm = rootElm;
		_rootItem = new CFormItem(_rootElm, rootData);
		_dependencies = deps;
		_comments = comment;
		_parents = parents;
		_parentRows = new QList<const QModelIndex*>;

		setupModelData();
	}

	CGeorgesFormModel::~CGeorgesFormModel() 
	{
		delete _rootItem;
	}

	/******************************************************************************/

	QVariant CGeorgesFormModel::data(const QModelIndex &p_index, int p_role) const 
	{
		if (!p_index.isValid())
			return QVariant();

		switch (p_role) 
		{
		case Qt::DisplayRole:
			{			
				return getItem(p_index)->data(p_index.column());
			}
		case Qt::BackgroundRole:
			{
				if (getItem(p_index)->valueFrom() == UFormElm::ValueDefaultDfn)
					return QBrush(QColor(255,0,0,30));
				if (getItem(p_index)->nodeFrom() == UFormElm::NodeParentForm)
					return QBrush(QColor(0,255,0,30));
				return QVariant();
			}
		case Qt::DecorationRole:
			{
				if (p_index.column() == 2) 
				{
					//p_index.
					QModelIndex in = index(p_index.row(),p_index.column()-1,p_index.parent());
					CFormItem *item = getItem(in);

					QString value = item->data(1).toString();
					//QString path = NLMISC::CPath::lookup(value.toStdString(),false).c_str();

					if (value.contains(".shape")) 
					{
						return QIcon(":/images/pqrticles.png");
					} 
					else if(value.contains(".tga") || value.contains(".png")) 
					{
						qDebug() << p_index << p_role;
						QString path = NLMISC::CPath::lookup(value.toStdString(),false).c_str();
						return QIcon(":/images/pqrticles.png");
					}
				}
				return QVariant();
				break;
			}
		default:
			return QVariant();
		}
	}

	/******************************************************************************/

	CFormItem *CGeorgesFormModel::getItem(const QModelIndex &index) const 
	{
		if (index.isValid()) 
		{
			CFormItem *item = static_cast<CFormItem*>(index.internalPointer());
			if (item) 
				return item;
		}
		return _rootItem;
	}

	/******************************************************************************/

	bool CGeorgesFormModel::setData(const QModelIndex &index, const QVariant &value,
		int role) 
	{

		if (role != Qt::EditRole)
			return false;

		CFormItem *item = getItem(index);
		bool result = item->setData(index.column(), value);

		if (result)
			Q_EMIT dataChanged(index, index);

		return result;
	}

	/******************************************************************************/

	Qt::ItemFlags CGeorgesFormModel::flags(const QModelIndex& index) const {

		if (!index.isValid())
			return 0;

		Qt::ItemFlags returnValue = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

		if(index.column() == 1)
			returnValue |= Qt::ItemIsEditable;

		return returnValue;

	}

	/******************************************************************************/

	QVariant CGeorgesFormModel::headerData(int section,
		Qt::Orientation orientation, int role) const
	{
		if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
			return _rootItem->data(section);

		return QVariant();

	}

	/******************************************************************************/

	QModelIndex CGeorgesFormModel::index(int row, int column, const QModelIndex &parent)
		const
	{
		if (!hasIndex(row, column, parent))
			return QModelIndex();

		CFormItem *parentItem;

		if (!parent.isValid())
			parentItem = _rootItem;
		else
			parentItem = static_cast<CFormItem*>(parent.internalPointer());

		CFormItem *childItem = parentItem->child(row);
		if (childItem)
			return createIndex(row, column, childItem);
		else
			return QModelIndex();
	}

	/******************************************************************************/

	QModelIndex CGeorgesFormModel::parent(const QModelIndex &index) const
	{
		if (!index.isValid())
			return QModelIndex();

		CFormItem *childItem = static_cast<CFormItem*>(index.internalPointer());
		CFormItem *parentItem = childItem->parent();

		if (parentItem == _rootItem)
			return QModelIndex();

		return createIndex(parentItem->row(), 0, parentItem);
	}

	/******************************************************************************/

	int CGeorgesFormModel::rowCount(const QModelIndex &parent) const {

		CFormItem *parentItem;
		if (parent.column() > 0)
			return 0;

		if (!parent.isValid())
			parentItem = _rootItem;
		else
			parentItem = static_cast<CFormItem*>(parent.internalPointer());

		return parentItem->childCount();

	}

	/******************************************************************************/

	int CGeorgesFormModel::columnCount(const QModelIndex &parent) const {

		if (parent.isValid())
			return static_cast<CFormItem*>(parent.internalPointer())->columnCount();
		else
			return _rootItem->columnCount();

	}

	/******************************************************************************/

	void CGeorgesFormModel::loadFormData(UFormElm *root, CFormItem *parent) {

		if (!root) 
			return;

		uint num = 0;
		UFormElm::TWhereIsNode *whereN = new UFormElm::TWhereIsNode;
		UFormElm::TWhereIsValue *whereV = new UFormElm::TWhereIsValue;

		if (root->isStruct()) 
		{
			//((CFormElm*)root)->getForm()->getComment();
			uint structSize = 0;
			root->getStructSize(structSize);
			while (num < structSize) 
			{
				// Append a new item to the current parent's list of children.
				std::string elmName;
				if(root->getStructNodeName(num, elmName)) 
				{
					QList<QVariant> columnData;
					//QVariant value;
					std::string value;
					//NLMISC::CRGBA value_color;
					//uint value_uint;
					//sint value_sint;
					//double value_double;
					QString elmtType = "";
					UFormElm *elmt = 0;
					if(root->getNodeByName(&elmt, elmName.c_str(),  whereN, true)) 
					{
						if (elmt) 
						{
							if (elmt->isArray())
								elmtType = "Array";						
							if (elmt->isStruct())
								elmtType = "Struct";
							if (elmt->isAtom()) {
								elmtType = "Atom";
								uint numDefinitions = 0;
								const UType *type = elmt->getType();
								if (type) 
								{
									numDefinitions = type->getNumDefinition();
									root->getValueByName(value, elmName.c_str(),UFormElm::Eval,whereV);
									switch (type->getType()) 
									{
									case UType::UnsignedInt:
										value = QString("%1").arg(QString("%1").arg(value.c_str()).toDouble()).toStdString();
										elmtType.append("_uint");break;
									case UType::SignedInt:
										value = QString("%1").arg(QString("%1").arg(value.c_str()).toDouble()).toStdString();
										elmtType.append("_sint");break;
									case UType::Double:
										value = QString("%1").arg(QString("%1").arg(value.c_str()).toDouble(),0,'f',1).toStdString();
										elmtType.append("_double");break;
									case UType::String:
										elmtType.append("_string");break;
									case UType::Color:
										elmtType.append("_color");break;
									default:
										elmtType.append("_unknownType");
									}
								}

								if (numDefinitions) 
								{
									std::string l, v;
									QString tmpLabel, tmpValue;
									for (uint i = 0; i < numDefinitions; i++) 
									{
										type->getDefinition(i,l,v);
										tmpLabel = l.c_str();
										tmpValue = v.c_str();
										if (type->getType() == UType::SignedInt) 
										{
											if (QString("%1").arg(value.c_str()).toDouble() == tmpValue.toDouble()) {
												value = l;
												break;
											}
										}
										if (type->getType() == UType::String) 
										{
											if (QString(value.c_str()) == tmpValue) 
											{
												value = l;
												break;
											}
										}
									}
								}
							}
							if (elmt->isVirtualStruct())
							{
								root->getValueByName(value, elmName.c_str(),UFormElm::Eval,whereV);
								elmtType = "VirtualStruct";
							}
							switch (*whereN) 
							{
							case UFormElm::NodeForm:
								elmtType.append("_fromForm");       break;
							case UFormElm::NodeParentForm:
								elmtType.append("_fromParentForm"); break;
							case UFormElm::NodeDfn:
								elmtType.append("_isDFN");          break;
							case UFormElm::NodeType:
								elmtType.append("_isType");         break;
							default:
								elmtType.append("_noNode");
							}
							switch (*whereV) 
							{
							case UFormElm::ValueForm:
								elmtType.append("_formValue");   break;
							case UFormElm::ValueParentForm:
								elmtType.append("_parentValue"); break;
							case UFormElm::ValueDefaultDfn:
								elmtType.append("_dfnValue");    break;
							case UFormElm::ValueDefaultType:
								elmtType.append("_typeValue");   break;
							default:
								elmtType.append("_noValue");
							}
							columnData << QString(elmName.c_str()) << QString(value.c_str()) << "" << elmtType;
							parent->appendChild(new CFormItem(elmt, columnData, parent, *whereV, *whereN));
							//if (parents.last()->childCount() > 0) {
							//	parents << parents.last()->child(parents.last()->childCount()-1);
							//}
							loadFormData(elmt, parent->child(parent->childCount()-1));
						} 
						else
						{
							// add Defaults
							//columnData << QString(elmName.c_str()) << QString("default") << QString("default");
							//parent->appendChild(new CFormItem(elmt, columnData, parent, UFormElm::ValueDefaultDfn, UFormElm::NodeDfn));
						} 
					}
				}

				num++;
			}
		}
		if (root->isArray())
		{
			uint arraySize = 0;
			root->getArraySize(arraySize);
			while (num < arraySize)
			{
				std::string elmName;
				if(root->getArrayNodeName(elmName, num))
				{
					QList<QVariant> columnData;
					std::string value;
					QString elmtType = "";
					//root->getValueByName(value, elmName.c_str());

					UFormElm *elmt = 0;
					if(root->getArrayNode(&elmt,0) && elmt) 
					{
						if (elmt->isArray())
							elmtType = "Array";
						if (elmt->isStruct()) {
							elmtType = "Struct";
						}
						if (elmt->isAtom()) 
						{
							elmt->getValue(value);
							elmtType = "Atom";
						}
						if (elmt->isVirtualStruct())
							elmtType = "VirtualStruct";
						columnData << QString(elmName.c_str()) << QString(value.c_str()) << "" << elmtType;
						parent->appendChild(new CFormItem(elmt, columnData, parent));
						loadFormData(elmt, parent->child(parent->childCount()-1));
					}
				}
				num++;
			}
		}
	}

	/******************************************************************************/

	void CGeorgesFormModel::loadFormHeader() 
	{

		CFormItem *fi_pars = new CFormItem(_rootElm, QList<QVariant>() << "parents", _rootItem);
		_rootItem->appendChild(fi_pars);

		Q_FOREACH(QString str, _parents) 
		{
			fi_pars->appendChild(new CFormItem(_rootElm, QList<QVariant>() << str, fi_pars));
		}

		/*QStringList dfns = _dependencies["dfn"];
		QStringList typs = _dependencies["typ"];

		_dependencies.remove("dfn");
		_dependencies.remove("typ");

		CFormItem *fi_dep = new CFormItem(_rootElm, QList<QVariant>() << "dependencies", _rootItem);
		_rootItem->appendChild(fi_dep);

		if (!dfns.isEmpty()) {
		CFormItem *fi_dfn = new CFormItem(_rootElm, QList<QVariant>() << "dfn", fi_dep);
		fi_dep->appendChild(fi_dfn);
		foreach(QString str, dfns) {
		fi_dfn->appendChild(new CFormItem(_rootElm, QList<QVariant>() << str, fi_dfn));
		}
		}
		if (!typs.isEmpty()) {
		CFormItem *fi_typ = new CFormItem(_rootElm, QList<QVariant>() << "typ", fi_dep);
		fi_dep->appendChild(fi_typ);
		foreach(QString str, typs) {
		fi_typ->appendChild(new CFormItem(_rootElm, QList<QVariant>() << str, fi_typ));
		}
		}
		if (!_dependencies.isEmpty()) {
		CFormItem *fi_other = new CFormItem(_rootElm, QList<QVariant>() << "other", fi_dep);
		fi_dep->appendChild(fi_other);
		foreach(QStringList list, _dependencies) {
		foreach(QString str, list) {
		fi_other->appendChild(new CFormItem(_rootElm, QList<QVariant>() << str, fi_other));
		}
		}
		}*/
	}

	/******************************************************************************/

	void CGeorgesFormModel::setupModelData()
	{
		loadFormHeader();
		loadFormData(_rootElm, _rootItem);
	}
} /* namespace NLQT */

/* end of file */
