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

#include "georgesform_model.h"

// System Includes
#include <libxml/parser.h>

// NeL includes
#include <nel/misc/types_nl.h>
#include <nel/misc/rgba.h>
#include <nel/misc/path.h>
#include <nel/misc/debug.h>
#include <nel/georges/u_form_elm.h>
#include <nel/georges/u_type.h>
#include <nel/georges/u_form_dfn.h>
#include <nel/georges/form.h>

// Qt includes
#include <QColor>
#include <QBrush>
#include <QApplication>
#include <QStyle>
#include <QDebug>
#include <QStylePainter>
#include <QStyleOption>
#include <QLabel>
#include <QPixmap>

// project includes
#include "formitem.h"
#include "georges_editor_form.h"
#include "actions.h"

using namespace NLGEORGES;

namespace GeorgesQt 
{

    CGeorgesFormModel::CGeorgesFormModel(UForm *form, QMap< QString, QStringList> deps,
        QString comment, QStringList parents, bool *expanded, QObject *parent) : QAbstractItemModel(parent) 
    {
        m_form = form;
        m_rootData << "Value" << "Data" << "Extra";// << "Type";
        m_rootItem = new CFormItem();
        m_dependencies = deps;
        m_comments = comment;
        m_parents = parents;
        m_parentRows = new QList<const QModelIndex*>;
        m_expanded = expanded;

        setupModelData();
    }

    CGeorgesFormModel::~CGeorgesFormModel() 
    {
        delete m_rootItem;
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
		case Qt::DecorationRole:
			{
				// Based on the _Type return a QIcon from resources.
				CFormItem *item = getItem(p_index);
				return item->getItemImage(m_rootItem);
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
        return m_rootItem;
    }

    /******************************************************************************/

    bool CGeorgesFormModel::setData(const QModelIndex &index, const QVariant &value, int role) 
    {

        if (role != Qt::EditRole)
            return false;
		
		CFormItem *item = getItem(index);

		if(!item->isEditable(index.column()))
			return false;

		GeorgesEditorForm::UndoStack->push(new CUndoFormArrayRenameCommand(this,item,value));

        Q_EMIT dataChanged(index, index);

        //setupModelData();
        return true;
    }

    /******************************************************************************/

    Qt::ItemFlags CGeorgesFormModel::flags(const QModelIndex& index) const {

        if (!index.isValid())
            return 0;

        Qt::ItemFlags returnValue = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

		CFormItem *item = getItem(index);
		
        if(item->isEditable(index.column()))
		{		
			returnValue |= Qt::ItemIsEditable;
		}

        return returnValue;

    }

    /******************************************************************************/

    QVariant CGeorgesFormModel::headerData(int section,
        Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal)
        {
            if (role == Qt::DisplayRole)
                return m_rootItem->data(section);
            if (role == Qt::TextAlignmentRole)
                return Qt::AlignLeft;
            if (section == 0 && role == Qt::DecorationRole)
            {
                // transparent pixmap as we paint it ourself with tree brach
                // if we extend the HeaderView::paintSection for the CE_HeaderLabel
                // we could drop this
                QPixmap pixmap = QPixmap(
                    QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize),
                    QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize));
                // Create new picture for transparent
                QPixmap transparent(pixmap.size());

                // Do transparency
                transparent.fill(Qt::transparent);
                QPainter p(&transparent);
                p.setCompositionMode(QPainter::CompositionMode_Source);
                p.drawPixmap(0, 0, pixmap);
                p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
                // Set transparency level to 150 (possible values are 0-255)
                // The alpha channel of a color specifies the transparency effect, 
                // 0 represents a fully transparent color, while 255 represents 
                // a fully opaque color.
                p.fillRect(transparent.rect(), QColor(0, 0, 0, 0));
                p.end();

                // Set original picture's reference to new transparent one
                pixmap = transparent;
                return pixmap;
            }
        }
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
            parentItem = m_rootItem;
        else
            parentItem = static_cast<CFormItem*>(parent.internalPointer());

        CFormItem *childItem = parentItem->child(row);
        if (childItem)
            return createIndex(row, column, childItem);
        else
            return QModelIndex();
    }

	QModelIndex CGeorgesFormModel::index(int row, int column, CFormItem *item) const
	{
		if(item == m_rootItem)
			return QModelIndex();

		return createIndex(row, 0, item);
	}

    /******************************************************************************/

    QModelIndex CGeorgesFormModel::parent(const QModelIndex &index) const
    {
        if (!index.isValid())
            return QModelIndex();

        CFormItem *childItem = static_cast<CFormItem*>(index.internalPointer());
        CFormItem *parentItem = childItem->parent();

        if (parentItem == m_rootItem)
            return QModelIndex();

        return createIndex(parentItem->row(), 0, parentItem);
    }

    /******************************************************************************/

    int CGeorgesFormModel::rowCount(const QModelIndex &parent) const {

        CFormItem *parentItem;
        if (parent.column() > 0)
            return 0;

        if (!parent.isValid())
            parentItem = m_rootItem;
        else
            parentItem = static_cast<CFormItem*>(parent.internalPointer());

        return parentItem->childCount();

    }

    /******************************************************************************/

    int CGeorgesFormModel::columnCount(const QModelIndex &parent) const {

        if (parent.isValid())
            return static_cast<CFormItem*>(parent.internalPointer())->columnCount();
        else
            return m_rootItem->columnCount();

    }

    /******************************************************************************/

    void CGeorgesFormModel::loadFormData(UFormElm *root, CFormItem *parent) {
		return;
    }

    CFormItem *CGeorgesFormModel::addStruct (CFormItem *parent,
                                             NLGEORGES::CFormElmStruct *_struct,
                                             NLGEORGES::CFormDfn *parentDfn,
                                             const char *name,
                                             uint structId,
                                             const char *formName,
                                             uint slot)
{
        // The form pointer
        NLGEORGES::CForm *formPtr = static_cast<NLGEORGES::CForm*>(m_form);

        // Add the new node
        CFormItem *newNode = parent->add(CFormItem::Form, name, structId, formName, slot, m_form);

        // Can be NULL in virtual DFN
        if (parentDfn)
        {
                // Get the parents
                std::vector<NLGEORGES::CFormDfn *> arrayDfn;
                arrayDfn.reserve (parentDfn->countParentDfn ());
                parentDfn->getParentDfn (arrayDfn);

                // For each child
                uint elm=0;
                for (uint dfn=0; dfn<arrayDfn.size(); dfn++)
                {
                        for (uint i=0; i<arrayDfn[dfn]->getNumEntry (); i++)
                        {
                                // Get the entry ref
                                NLGEORGES::CFormDfn::CEntry &entry = arrayDfn[dfn]->getEntry (i);

                                // Form entry name
                                std::string entryName = (std::string (formName)+"."+entry.getName ());

                                // Is a struct ?
                                if ( (entry.getType () == NLGEORGES::UFormDfn::EntryDfn) || (entry.getType () == NLGEORGES::UFormDfn::EntryVirtualDfn) )
								{
                                        // Is an array of struct ?
                                        if (entry.getArrayFlag ())
                                        {
                                                // Get it from the form
                                                CFormElmArray *nextArray = NULL;
                                                if (_struct && _struct->Elements[elm].Element)
                                                        nextArray = NLMISC::safe_cast<NLGEORGES::CFormElmArray*> (_struct->Elements[elm].Element);

                                                // Else, get it from the parent if we are not a virtual DFN (don't inheritate)

                                                // todo array of virtual struct
                                                if (!nextArray && (entry.getType () != NLGEORGES::UFormDfn::EntryVirtualDfn) )
                                                {
                                                        // For each parent form
                                                        for (uint parent=0; parent<formPtr->getParentCount (); parent++)
                                                        {
                                                                // Get the node by name
                                                                NLGEORGES::UFormElm *uNode;
                                                                if (formPtr->getParent (parent)->getRootNode ().getNodeByName (&uNode, entryName.c_str(), NULL, false) && uNode)
                                                                {
                                                                        nextArray = NLMISC::safe_cast<NLGEORGES::CFormElmArray*> (uNode);
                                }
                                                        }
                                                }

                                                // Add the new struct
                                                addArray (newNode, nextArray, entry.getDfnPtr (), entry.getName().c_str(), elm, entryName.c_str (), slot);
                                        }
                                        else
                                        {
                                                // Add it
                                                NLGEORGES::CFormElmStruct *nextForm = NULL;

                                                // Get it from the form
                                                if (_struct && _struct->Elements[elm].Element)
                                                        nextForm = NLMISC::safe_cast<NLGEORGES::CFormElmStruct*> (_struct->Elements[elm].Element);

                                                // Else, get it from the parent
                                                if (!nextForm)
                                                {
                                                        // For each parent form
                                                        for (uint parent=0; parent<formPtr->getParentCount (); parent++)
                                                        {
                                                                // Get the node by name
                                                                NLGEORGES::UFormElm *uNode;
                                                                if (formPtr->getParent (parent)->getRootNode ().getNodeByName (&uNode, entryName.c_str(), NULL, false) && uNode)
                                                                {
                                                                        nextForm = NLMISC::safe_cast<NLGEORGES::CFormElmStruct*> (uNode);
                                                                }
                                                        }
                                                }

                                                // Virtual Dfn pointer
                                                NLGEORGES::CFormElmVirtualStruct *vStruct = ((entry.getType () == NLGEORGES::UFormDfn::EntryVirtualDfn) && nextForm) ?
                                                        NLMISC::safe_cast<NLGEORGES::CFormElmVirtualStruct*> (nextForm) : NULL;

                                                NLGEORGES::CFormDfn *tmpDfn = vStruct ?
                                                            ((NLGEORGES::CFormDfn*)vStruct->FormDfn) : entry.getDfnPtr();
                                                // Add the new struct
                                                addStruct (newNode, nextForm, tmpDfn, entry.getName().c_str(), elm, entryName.c_str(), slot);
                                        }
                                }
                                // Array of type ?
                                else if ( entry.getArrayFlag () )
                                {
                                        NLGEORGES::CFormElmArray *nextArray = NULL;

                                        // Get it from the form
                                        if (_struct && _struct->Elements[elm].Element)
                                                nextArray = NLMISC::safe_cast<NLGEORGES::CFormElmArray*> (_struct->Elements[elm].Element);

                                        // Else, get it from the parent
                                        if (!nextArray)
                                        {
                                                // For each parent form
                                                for (uint parent=0; parent<formPtr->getParentCount (); parent++)
                                                {
                                                        // Get the node by name
                                                        NLGEORGES::UFormElm *uNode;
                                                        if (formPtr->getParent (parent)->getRootNode ().getNodeByName (&uNode, entryName.c_str(), NULL, false) && uNode)
                                                        {
                                                                nextArray = NLMISC::safe_cast<NLGEORGES::CFormElmArray*> (uNode);
                                                        }
                                                }
                                        }

                                        // Add the new array
                                        addArray ( newNode, nextArray, NULL, entry.getName().c_str(), elm, entryName.c_str(), slot );
                                }

                                // Next element
                                elm++;
                        }
                }
        }

        return newNode;
}

// ***************************************************************************

CFormItem *CGeorgesFormModel::addArray(CFormItem *parent,
                                       NLGEORGES::CFormElmArray *array,
                                       NLGEORGES::CFormDfn *rootDfn,
                                       const char *name,
                                       uint structId,
                                       const char *formName,
                                       uint slot)
{
        // Add the new node
        CFormItem *newNode = parent->add (CFormItem::Form, name, structId, formName, slot, m_form);

        // The array exist
        if (array)
        {
                // For each array element
                for (uint elm=0; elm<array->Elements.size(); elm++)
                {
                        // The form name
                        char formArrayElmName[512];
                        NLMISC::smprintf (formArrayElmName, 512, "%s[%d]", formName, elm);

                        // The name
                        char formArrayName[512];
                        if (array->Elements[elm].Name.empty ())
                        {
                                NLMISC::smprintf (formArrayName, 512, "#%d", elm);
                        }
                        else
                        {
                                NLMISC::smprintf (formArrayName, 512, "%s", array->Elements[elm].Name.c_str());
                        }

                        // Is a struct
                        if (rootDfn)
                        {
                                // Get struct ptr
                                NLGEORGES::CFormElmStruct *elmPtr =  array->Elements[elm].Element ? static_cast<NLGEORGES::CFormElmStruct*>(array->Elements[elm].Element) : NULL;
                                addStruct (newNode, elmPtr, rootDfn, formArrayName, elm, formArrayElmName, slot);
                        }
                        else
                        {
                                NLGEORGES::CFormElmArray *elmPtr = array->Elements[elm].Element ? static_cast<NLGEORGES::CFormElmArray*>(array->Elements[elm].Element) : NULL;
                                newNode->add (CFormItem::Form, formArrayName, elm, formArrayElmName, slot, m_form);
                        }
                }
        }

        return newNode;
}


void CGeorgesFormModel::arrayResized( const QString &name, int size )
{
	CFormItem *item = m_rootItem->findItem( name );
	if( item == NULL )
		return;

	NLGEORGES::UFormElm *elm = NULL;

	item->form()->getRootNode().getNodeByName( &elm, name.toUtf8().constData() );

	if( elm == NULL )
		return;

	NLGEORGES::CFormElmArray *celm = dynamic_cast< NLGEORGES::CFormElmArray* >( elm );
	if( celm == NULL )
		return;

	item->clearChildren();

	for( int i = 0; i < celm->Elements.size(); i++ )
	{
		NLGEORGES::CFormElmArray::CElement &e = celm->Elements[ i ];

		QString formName = name;
		formName += '[';
		formName += QString::number( i );
		formName += ']';

		QString n;
		if( e.Name.empty() )
			n = "#" + QString::number( i );
		else
			n = e.Name.c_str();

		item->add( CFormItem::Form, n.toUtf8().constData(), i, formName.toUtf8().constData(), 0, item->form() );
	}

}



    /******************************************************************************/

    void CGeorgesFormModel::loadFormHeader() 
    {

 /*       if (m_parents.size())
        {
            CFormItem *fi_pars = new CFormItem(m_rootElm, QList<QVariant>() << "parents" << "" << "", m_rootItem);
            m_rootItem->appendChild(fi_pars);

            Q_FOREACH(QString str, m_parents) 
            {
                fi_pars->appendChild(new CFormItem(m_rootElm, QList<QVariant>() << str << "" << "", fi_pars));
            }
        }*/

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
        m_rootElm = &((NLGEORGES::CForm*)m_form)->Elements;
        NLGEORGES::CFormElmStruct *rootstruct = &((NLGEORGES::CForm*)m_form)->Elements;
        //loadFormHeader();
        addStruct(m_rootItem, rootstruct, rootstruct->FormDfn, "Content", 0xffffffff, "", 0);
        //loadFormData(m_rootElm, m_rootItem);
    }

    /******************************************************************************/

    void CGeorgesFormModel::setShowParents( bool show ) { 
        m_showParents = show;
        Q_EMIT layoutAboutToBeChanged();
        Q_EMIT layoutChanged();
    }
    void CGeorgesFormModel::setShowDefaults( bool show ) 
    { 
        m_showDefaults = show;
        Q_EMIT layoutAboutToBeChanged();
        Q_EMIT layoutChanged();
    }

    void CGeorgesFormModel::addParentForm(QString parentForm)
    {
        beginResetModel();
        m_parents.push_back(parentForm);
        delete m_rootItem;
        m_rootItem = new CFormItem();
        setupModelData();
        endResetModel();
    }

    void CGeorgesFormModel::removeParentForm(QString parentForm)
    {
        beginResetModel();
        m_parents.removeOne(parentForm);

        delete m_rootItem;
        m_rootItem = new CFormItem();
        setupModelData();
        endResetModel();
    }
} /* namespace GeorgesQt */

/* end of file */
