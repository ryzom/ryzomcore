// Object Viewer Qt - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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
#include "property_editor_widget.h"

// NeL includes
#include <nel/misc/debug.h>

// STL includes
#include <vector>
#include <string>

// Qt includes
#include <QtCore/QModelIndex>

namespace WorldEditor
{

PropertyEditorWidget::PropertyEditorWidget(QWidget *parent)
	: QWidget(parent)
{
	m_ui.setupUi(this);

	m_variantManager = new QtVariantPropertyManager(this);
	m_enumManager = new QtEnumPropertyManager(this);
	connect(m_variantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
			this, SLOT(valueChanged(QtProperty *, const QVariant &)));

	QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);
	QtEnumEditorFactory *enumFactory = new QtEnumEditorFactory(this);
	m_ui.treePropertyBrowser->setFactoryForManager(m_variantManager, variantFactory);
	m_ui.treePropertyBrowser->setFactoryForManager(m_enumManager, enumFactory);

	m_groupManager = new QtGroupPropertyManager(this);	
}

PropertyEditorWidget::~PropertyEditorWidget()
{
}

void PropertyEditorWidget::clearProperties()
{
	m_ui.treePropertyBrowser->clear();
}

void PropertyEditorWidget::updateSelection(const NodeList &selected, const NodeList &deselected)
{
	clearProperties();

	// The parameter list
	std::set<NLLIGO::CPrimitiveClass::CParameter> parameterList;

	for (int i = 0; i < selected.size(); ++i)
	{
		if (selected.at(i)->type() == Node::RootPrimitiveNodeType)
		{
			/*
			const_cast<IPrimitive*>(_PropDlgLocators[i].Primitive)->removePropertyByName("name");
			const_cast<IPrimitive*>(_PropDlgLocators[i].Primitive)->removePropertyByName("path");
			//TODO  faire une fonction dans CWorldDoc pour recup m_strPathName
			string name;
			getDocument()->getPrimitiveDisplayName(name,_PropDlgLocators[i].getDatabaseIndex());
			string path;
			getDocument()->getFilePath(_PropDlgLocators[i].getDatabaseIndex(),path);

			const_cast<IPrimitive*>(_PropDlgLocators[i].Primitive)->addPropertyByName("name",new CPropertyString (name));
			const_cast<IPrimitive*>(_PropDlgLocators[i].Primitive)->addPropertyByName("path",new CPropertyString (path));
			*/
		}
	
		if (selected.at(i)->type() == Node::PrimitiveNodeType)
		{
			PrimitiveNode *node = static_cast<PrimitiveNode *>(selected.at(i));
			const NLLIGO::IPrimitive *primitive = node->primitive();
			const NLLIGO::CPrimitiveClass *primClass = node->primitiveClass();
			
			// Use the class or not ?
			if (primClass)
			{
				QtProperty *groupNode;
				groupNode = m_groupManager->addProperty(node->data(Qt::DisplayRole).toString());
				m_ui.treePropertyBrowser->addProperty(groupNode);
	
				// For each properties of the class
				for (uint p = 0; p < primClass->Parameters.size(); p++)
				{
					// Is the parameter visible ?
					if (primClass->Parameters[p].Visible)
					{
						QtProperty *param;

						if (primClass->Parameters[p].Type == NLLIGO::CPrimitiveClass::CParameter::Boolean) 
							param = m_variantManager->addProperty(QVariant::Bool, primClass->Parameters[p].Name.c_str());
						else if (primClass->Parameters[p].Type == NLLIGO::CPrimitiveClass::CParameter::ConstString)
						{
							param = m_enumManager->addProperty(primClass->Parameters[p].Name.c_str());
						}
						else if (primClass->Parameters[p].Type == NLLIGO::CPrimitiveClass::CParameter::String) 
							param = m_variantManager->addProperty(QVariant::String, primClass->Parameters[p].Name.c_str());
						else
							param = m_variantManager->addProperty(QVariant::String, primClass->Parameters[p].Name.c_str());

						groupNode->addSubProperty(param);

						parameterList.insert(primClass->Parameters[p]);
					}
				}
			}
			else
			{
				// For each primitive property
				uint numProp = primitive->getNumProperty();
				for (uint p = 0; p < numProp; p++)
				{
					// Get the property
					std::string propertyName;
					const NLLIGO::IProperty *prop;
					nlverify(primitive->getProperty (p, propertyName, prop));

					// Add a default property
					NLLIGO::CPrimitiveClass::CParameter defProp(*prop, propertyName.c_str());
					parameterList.insert(defProp);
				}
			}
		}
	}

	// Remove property class
	std::set<NLLIGO::CPrimitiveClass::CParameter>::iterator ite = parameterList.begin ();
	while (ite != parameterList.end ())
	{
		// Next iterator
		std::set<NLLIGO::CPrimitiveClass::CParameter>::iterator next = ite;
		next++;

		// Property name ?
		if (ite->Name == "class")
		{
			// Remove it
			parameterList.erase (ite);
		}

		ite = next;
	}

	// Add the default parameter
	NLLIGO::CPrimitiveClass::CParameter defaultParameter;
	defaultParameter.Visible = true;
	defaultParameter.Filename = false;
}

} /* namespace WorldEditor */
