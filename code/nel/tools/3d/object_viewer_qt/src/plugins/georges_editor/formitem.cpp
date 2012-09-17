// Object Viewer Qt - Georges Editor Plugin - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2011  Adrian Jaekel <aj at elane2k dot com>
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

#include "formitem.h"

// Qt includes

// NeL includes
#include <nel/misc/o_xml.h>
#include <nel/georges/u_type.h>
#include <nel/georges/form.h>

using namespace NLGEORGES;

namespace GeorgesQt 
{

	CFormItem::CFormItem(NLGEORGES::UFormElm* elm, const QList<QVariant> &data, CFormItem *parent,
        NLGEORGES::UFormElm::TWhereIsValue wV, NLGEORGES::UFormElm::TWhereIsNode wN)
	{
		parentItem = parent;
		itemData = data;
		formElm = elm;
		whereV = wV;
		whereN = wN;
	}

	CFormItem::~CFormItem() 
	{
		qDeleteAll(childItems);
	}

	void CFormItem::appendChild(CFormItem *item) 
	{
		childItems.append(item);
	}

	CFormItem *CFormItem::child(int row) 
	{
		return childItems.value(row);
	}

	int CFormItem::childCount() const 
	{
		return childItems.count();
	}

	int CFormItem::columnCount() const 
	{
		//nlinfo("columnCount %d",itemData.count());
		return itemData.count();
	}

	QVariant CFormItem::data(int column) const 
	{
		return itemData.value(column);
	}

	CFormItem *CFormItem::parent()
	{
		return parentItem;
	}

	int CFormItem::row() const 
	{
		if (parentItem)
			return parentItem->childItems.indexOf(const_cast<CFormItem*>(this));

		return 0;
	}

	bool CFormItem::setData(int column, const QVariant &value) 
	{
		if (column < 0 || column >= itemData.size())
			return false;

		// TODO: default values
		if (!formElm)
			return false;

		itemData[column] = value;
		if (formElm->isAtom()) 
		{
			const NLGEORGES::UType *type = formElm->getType();
			if (type) 
			{
				switch (type->getType()) 
				{
				case NLGEORGES::UType::UnsignedInt:
				case NLGEORGES::UType::SignedInt:
				case NLGEORGES::UType::Double:
				case NLGEORGES::UType::String:
					if (parentItem->formElm->isArray())
					{
						//((NLGEORGES::CFormElm*)parentItem->formElm);//->arrayInsertNodeByName(
						//if(parentItem->formElm->getArrayNode(elmName, num))
						//{
						//}

						bool ok;
						// TODO: the node can be renamed from eg "#0" to "foobar"
						int arrayIndex = itemData[0].toString().remove("#").toInt(&ok);
						if(ok)
						{
							NLGEORGES::UFormElm *elmt = 0;
							if(parentItem->formElm->getArrayNode(&elmt, arrayIndex) && elmt)
							{
								if (elmt->isAtom()) 
								{
									((NLGEORGES::CFormElmAtom*)elmt)->setValue(value.toString().toStdString().c_str());
									nldebug(QString("array element string %1 %2")
									.arg(itemData[0].toString()).arg(value.toString())
									.toStdString().c_str());
								}
							}
						}
					}
					else
					{
						if(parentItem->formElm->setValueByName(
							value.toString().toStdString().c_str(),
							itemData[0].toString().toStdString().c_str()))
						{
							nldebug(QString("string %1 %2")
							.arg(itemData[0].toString()).arg(value.toString())
							.toStdString().c_str());
						}
						else
						{
							nldebug(QString("FAILED string %1 %2")
							.arg(itemData[0].toString()).arg(value.toString())
							.toStdString().c_str());
						}
					}
					break;
				case NLGEORGES::UType::Color:
					nldebug("Color is TODO");
					break;
				default:
					break;
				}
			}
		}
		else
		{
			nldebug("setting sth other than Atom");
		}
		//formElm->setValueByName();
		return true;
	}

//    CFormItem *CFormItem::add (/*TSub type,*/ const char *name, uint structId, const char *formName, uint slot)
//    {
            // Add at the end
//            uint index = _Children.size();
//            _Children.push_back (new CGeorgesEditDocSub);

//            _Children[index]->_Type = type;
//            _Children[index]->_Name = name;
//            _Children[index]->_Parent = this;
//            _Children[index]->_StructId = structId;
//            _Children[index]->_FormName = formName;
//            _Children[index]->_Slot  = slot;
//            return _Children[index];
//        CFormItem *newNode = new CFormItem();
//        appendChild(newNode);
//        return NULL;
//    }

    CFormItem *CFormItem::add(NLGEORGES::UFormElm* root, std::string elmName)
    {
        CFormItem *newItem = NULL;
        UFormElm::TWhereIsNode *whereN = new UFormElm::TWhereIsNode;
        UFormElm::TWhereIsValue *whereV = new UFormElm::TWhereIsValue;
        // Append a new item to the current parent's list of children.
//        std::string elmName;
//        if(root->getStructNodeName(num, elmName))
//        {
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
                    if (elmt->isAtom())
                    {
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
                        else
                        {
                            elmtType.append("_noType");
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
                        elmtType.append("_fromForm");	   break;
                    case UFormElm::NodeParentForm:
                        elmtType.append("_fromParentForm"); break;
                    case UFormElm::NodeDfn:
                        elmtType.append("_isDFN");		  break;
                    case UFormElm::NodeType:
                        elmtType.append("_isType");		 break;
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
                        elmtType.append("_dfnValue");	break;
                    case UFormElm::ValueDefaultType:
                        elmtType.append("_typeValue");   break;
                    default:
                        elmtType.append("_noValue");
                    }
                    columnData << QString(elmName.c_str()) << QString(value.c_str()) << "";// << elmtType;

                    newItem = new CFormItem(elmt, columnData, this, *whereV, *whereN);
                    this->appendChild(newItem);

                    return newItem;
                    //if (parents.last()->childCount() > 0) {
                    //	parents << parents.last()->child(parents.last()->childCount()-1);
                    //}

                    // The building of the tree should be haoppening in the mode,.
                    //loadFormData(elmt, parent->child(parent->childCount()-1));
                }
                else
                {
                    // add Defaults
                    // TODO: spams warnings for non ATOM values but i dont get type of non existing nodes
                    bool success = root->getValueByName(value, elmName.c_str(),UFormElm::Eval,whereV);
                    switch (*whereN)
                    {
                    case UFormElm::NodeForm:
                        elmtType.append("_fromForm");	   break;
                    case UFormElm::NodeParentForm:
                        elmtType.append("_fromParentForm"); break;
                    case UFormElm::NodeDfn:
                        elmtType.append("_isDFN");		  break;
                    case UFormElm::NodeType:
                        elmtType.append("_isType");		 break;
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
                        elmtType.append("_dfnValue");	break;
                    case UFormElm::ValueDefaultType:
                        elmtType.append("_typeValue");   break;
                    default:
                        elmtType.append("_noValue");
                    }

                    columnData << QString(elmName.c_str()) << QString(value.c_str()) << "";// << elmtType;
                    newItem = new CFormItem(elmt, columnData, this, *whereV, *whereN);
                    this->appendChild(newItem);
                    return newItem;
                }
//            }
//            else
//            {
//                nlinfo("getNodeByName returned false");
//            }
        }
    }

}
