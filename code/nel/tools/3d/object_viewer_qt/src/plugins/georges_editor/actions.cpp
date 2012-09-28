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
#include "georgesform_model.h"

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

	CUndoFormArrayRenameCommand::CUndoFormArrayRenameCommand(CGeorgesFormModel *model, const QModelIndex &index, const QVariant &value, uint elementId, QUndoCommand *parent)
		: QUndoCommand(parent), m_model(model), m_elementId(elementId)	
	{
		m_row = index.row();
		m_col = index.column();

		m_newValue = value.toString();
	}

	void CUndoFormArrayRenameCommand::redo()
	{
		update(true);
	}

	void CUndoFormArrayRenameCommand::undo()
	{
		update(false);
	}

	void CUndoFormArrayRenameCommand::update(bool redo) 
	{
		QModelIndex index = m_model->index(m_row, m_col);
		CFormItem *item = m_model->getItem(index);

		// Get the parent node
		const NLGEORGES::CFormDfn *parentDfn;
		uint indexDfn;
		const NLGEORGES::CFormDfn *nodeDfn;
		const NLGEORGES::CType *nodeType;
		NLGEORGES::CFormElm *node;
		NLGEORGES::UFormDfn::TEntryType type;
		bool isArray;
		bool vdfnArray;
		NLGEORGES::CForm *form=static_cast<NLGEORGES::CForm*>(item->form());
		if(!form)
		{
			nlinfo("failed to convert form.");
			return;
		}

		NLGEORGES::CFormElm *elm = static_cast<NLGEORGES::CFormElm*>(&form->Elements);

		if(!elm)
			nlwarning("Failed to convert elm!");

		nlverify ( elm->getNodeByName (item->formName().c_str (), &parentDfn, indexDfn, &nodeDfn, &nodeType, &node, type, isArray, vdfnArray, true, NLGEORGES_FIRST_ROUND) );
		if (node)
		{
			std::string tmpName;
			node->getFormName(tmpName);
			nlinfo("doing array rename on '%s'", tmpName.c_str());

			NLGEORGES::CFormElmArray* array = static_cast<NLGEORGES::CFormElmArray*> (node->getParent ());
			if(!array)
				nlwarning("the array is invalid.");

			// In the redo stage save the old value, just in case.
			if(redo)
			{
				// If the name of the element is empty then give it a nice default.
				if(array->Elements[m_elementId].Name.empty())
				{
					m_oldValue.append("#");
					m_oldValue.append(QString("%1").arg(m_elementId));
				}
				else
				{
					m_oldValue = QString::fromStdString(array->Elements[m_elementId].Name);
				}
			}

			QString value;
			if(redo)
				value = m_newValue;
			else
				value = m_oldValue;


			array->Elements[m_elementId].Name = value.toStdString();
			item->setName(value.toStdString());

			m_model->emitDataChanged(index);
		}
	}
}