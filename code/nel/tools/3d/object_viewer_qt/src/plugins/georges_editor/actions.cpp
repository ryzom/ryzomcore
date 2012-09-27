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

// Project includes
#include "actions.h"
#include "formitem.h"

// Qt includes

// NeL includes
#include <nel/misc/debug.h>
#include <nel/misc/file.h>
#include <nel/misc/o_xml.h>
#include <nel/georges/u_form_loader.h>
#include <nel/georges/form.h>
#include <nel/georges/u_form.h>
#include <nel/georges/u_type.h>

namespace GeorgesQt 
{

	CUndoFormArrayRenameCommand::CUndoFormArrayRenameCommand(CFormItem *item, QString newValue, uint elementId, QUndoCommand *parent)
		: QUndoCommand("Rename Form Array", parent), m_item(item), m_newValue(newValue), m_elementId(elementId)	
	{ }

	void CUndoFormArrayRenameCommand::redo()
	{
		// Get the parent node
		const NLGEORGES::CFormDfn *parentDfn;
		uint indexDfn;
		const NLGEORGES::CFormDfn *nodeDfn;
		const NLGEORGES::CType *nodeType;
		NLGEORGES::CFormElm *node;
		NLGEORGES::UFormDfn::TEntryType type;
		bool isArray;
		bool vdfnArray;
		NLGEORGES::CForm *form=static_cast<NLGEORGES::CForm*>(m_item->form());
		NLGEORGES::CFormElm *elm = static_cast<NLGEORGES::CFormElm*>(&form->getRootNode());
		nlverify ( elm->getNodeByName (m_item->formName().c_str (), &parentDfn, indexDfn, &nodeDfn, &nodeType, &node, type, isArray, vdfnArray, true, NLGEORGES_FIRST_ROUND) );
		if (node)
		{
			nlinfo("doing array rename");
				NLGEORGES::CFormElmArray* array = NLMISC::safe_cast<NLGEORGES::CFormElmArray*> (node->getParent ());
				if(array->Elements[m_elementId].Name.empty())
				{
					m_oldValue.append("#");
					m_oldValue.append(QString("%1").arg(m_elementId));
				}
				else
				{
					m_oldValue = array->Elements[m_elementId].Name.c_str();
				}

				array->Elements[m_elementId].Name = m_newValue.toStdString();
				m_item->setName(m_newValue.toStdString());
		}

	}

	void CUndoFormArrayRenameCommand::undo()
	{
		// Get the parent node
		const NLGEORGES::CFormDfn *parentDfn;
		uint indexDfn;
		const NLGEORGES::CFormDfn *nodeDfn;
		const NLGEORGES::CType *nodeType;
		NLGEORGES::CFormElm *node;
		NLGEORGES::UFormDfn::TEntryType type;
		bool isArray;
		bool vdfnArray;
		NLGEORGES::CForm *form=static_cast<NLGEORGES::CForm*>(m_item->form());
		NLGEORGES::CFormElm *elm = static_cast<NLGEORGES::CFormElm*>(&form->getRootNode());
		nlverify ( elm->getNodeByName (m_item->formName().c_str (), &parentDfn, indexDfn, &nodeDfn, &nodeType, &node, type, isArray, vdfnArray, true, NLGEORGES_FIRST_ROUND) );
		if (node)
		{
				NLGEORGES::CFormElmArray* array = NLMISC::safe_cast<NLGEORGES::CFormElmArray*> (node->getParent ());
				//m_oldValue = array->Elements[m_elementId].Name.c_str();
				array->Elements[m_elementId].Name = m_oldValue.toStdString();
				m_item->setName(m_oldValue.toStdString());
		}

	}

}