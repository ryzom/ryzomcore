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
#include "world_editor_misc.h"

// NeL includes
#include <nel/misc/debug.h>

// STL includes
#include <vector>
#include <string>

// Qt includes
#include <QtCore/QModelIndex>
#include <QMap>

#include "const_string_array_property.h"

namespace WorldEditor
{

struct PropertyEditorWidgetPrivate
{
	QMap< QtProperty*, NLLIGO::IPrimitive* > propToPrim;

	void clearPrimitives()
	{
		propToPrim.clear();
	}

	void addPrimitive( QtProperty *p, NLLIGO::IPrimitive *prim )
	{
		QMap< QtProperty*, NLLIGO::IPrimitive* >::const_iterator itr 
			= propToPrim.find( p );
		if( itr != propToPrim.end() )
			return;

		propToPrim[ p ] = prim;
	}

	NLLIGO::IPrimitive* getPrimitive( QtProperty *p )
	{
		NLLIGO::IPrimitive *prim = NULL;

		QMap< QtProperty*, NLLIGO::IPrimitive* >::const_iterator itr 
			= propToPrim.find( p );

		if( itr != propToPrim.end() )
		{
			prim = itr.value();
		}

		return prim;
	}
};

PropertyEditorWidget::PropertyEditorWidget(QWidget *parent)
	: QWidget(parent)
{
	m_ui.setupUi(this);

	m_stringManager = new QtStringPropertyManager(this);
	m_boolManager = new QtBoolPropertyManager(this);
	m_enumManager = new QtEnumPropertyManager(this);
	m_stringArrayManager = new QtTextPropertyManager(this);

	m_constStrArrPropMgr = new ConstStrArrPropMgr(this);
	m_constStrArrEditorFactory = new ConstStrArrEditorFactory(this);

	QtLineEditFactory *lineEditFactory = new QtLineEditFactory(this);
	QtCheckBoxFactory *boolFactory = new QtCheckBoxFactory(this);
	QtEnumEditorFactory *enumFactory = new QtEnumEditorFactory(this);
	QtTextEditorFactory *textFactory = new QtTextEditorFactory(this);

	m_ui.treePropertyBrowser->setFactoryForManager(m_stringManager, lineEditFactory);
	m_ui.treePropertyBrowser->setFactoryForManager(m_boolManager, boolFactory);
	m_ui.treePropertyBrowser->setFactoryForManager(m_enumManager, enumFactory);
	m_ui.treePropertyBrowser->setFactoryForManager(m_stringArrayManager, textFactory);
	m_ui.treePropertyBrowser->setFactoryForManager(m_constStrArrPropMgr, m_constStrArrEditorFactory);

	m_groupManager = new QtGroupPropertyManager(this);

	d_ptr = new PropertyEditorWidgetPrivate();

	connect(m_stringManager, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
	connect(m_boolManager, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
	connect(m_enumManager, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
	connect(m_stringArrayManager, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));
	connect(m_constStrArrPropMgr, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));

	connect(m_boolManager, SIGNAL(resetProperty(QtProperty *)), this, SLOT(resetProperty(QtProperty *)));
	connect(m_stringManager, SIGNAL(resetProperty(QtProperty *)), this, SLOT(resetProperty(QtProperty *)));
	connect(m_enumManager, SIGNAL(resetProperty(QtProperty *)), this, SLOT(resetProperty(QtProperty *)));
	connect(m_stringArrayManager, SIGNAL(resetProperty(QtProperty *)), this, SLOT(resetProperty(QtProperty *)));
}

PropertyEditorWidget::~PropertyEditorWidget()
{
	delete d_ptr;
	d_ptr = NULL;
}

void PropertyEditorWidget::clearProperties()
{
	d_ptr->clearPrimitives();
	m_ui.treePropertyBrowser->clear();
}

void PropertyEditorWidget::updateSelection(Node *node)
{
	clearProperties();

	if ((node == 0) || (node->type() != Node::PrimitiveNodeType))
		return;

	blockSignalsOfProperties(true);

	// The parameter list
	std::list<NLLIGO::CPrimitiveClass::CParameter> parameterList;

	PrimitiveNode *primNode = static_cast<PrimitiveNode *>(node);
	const NLLIGO::IPrimitive *primitive = primNode->primitive();
	const NLLIGO::CPrimitiveClass *primClass = primNode->primitiveClass();

	// Use the class or not ?
	if (primClass)
	{
		// For each properties of the class
		for (uint p = 0; p < primClass->Parameters.size(); p++)
		{
			// Is the parameter visible ?
			if (primClass->Parameters[p].Visible)
			{
				if (primClass->Parameters[p].Name == "name")
					parameterList.push_front(primClass->Parameters[p]);
				else
					parameterList.push_back(primClass->Parameters[p]);
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
			nlverify(primitive->getProperty(p, propertyName, prop));

			// Add a default property
			NLLIGO::CPrimitiveClass::CParameter defProp(*prop, propertyName.c_str());

			if (defProp.Name == "name")
				parameterList.push_front(defProp);
			else
				parameterList.push_back(defProp);
		}
	}

	// Remove property class
	std::list<NLLIGO::CPrimitiveClass::CParameter>::iterator ite = parameterList.begin ();
	while (ite != parameterList.end ())
	{
		std::list<NLLIGO::CPrimitiveClass::CParameter>::iterator next = ite;
		next++;
		if (ite->Name == "class")
		{
			parameterList.erase(ite);
		}
		ite = next;
	}

	QtProperty *groupNode;
	groupNode = m_groupManager->addProperty(QString("%1(%2)").arg(node->data(Qt::DisplayRole).toString()).arg(primClass->Name.c_str()));
	m_ui.treePropertyBrowser->addProperty(groupNode);

	ite = parameterList.begin();
	while (ite != parameterList.end())
	{
		NLLIGO::CPrimitiveClass::CParameter &parameter = (*ite);
		QtProperty *prop;
		NLLIGO::IProperty *ligoProperty = 0;
		primitive->getPropertyByName(parameter.Name.c_str(), ligoProperty);

		if (parameter.Type == NLLIGO::CPrimitiveClass::CParameter::ConstString)
			prop = addConstStringProperty(ligoProperty, parameter, primitive);
		else if (parameter.Type == NLLIGO::CPrimitiveClass::CParameter::String)
			prop = addStringProperty(ligoProperty, parameter, primitive);
		else if (parameter.Type == NLLIGO::CPrimitiveClass::CParameter::StringArray)
			prop = addStringArrayProperty(ligoProperty, parameter, primitive);
		else if (parameter.Type == NLLIGO::CPrimitiveClass::CParameter::ConstStringArray)
			prop = addConstStringArrayProperty(ligoProperty, parameter, primitive);
		else
			prop = addBoolProperty(ligoProperty, parameter, primitive);

		d_ptr->addPrimitive( prop, const_cast< NLLIGO::IPrimitive* >( primitive ) );

		// Default value ?
		if	((ligoProperty == NULL)	|| (ligoProperty->Default))
			prop->setModified(false);
		else
			prop->setModified(true);

		bool staticChildSelected = Utils::ligoConfig()->isStaticChild(*primitive);
		if (parameter.ReadOnly || (staticChildSelected && (parameter.Name == "name")))
			prop->setEnabled(false);

		// File ?
		if (parameter.Filename && (parameter.FileExtension.empty() || parameter.Type != NLLIGO::CPrimitiveClass::CParameter::StringArray))
		{
			// TODO: Create an edit box
			// CHECK: only for ConstString
		}

		groupNode->addSubProperty(prop);

		ite++;
	}

	blockSignalsOfProperties(false);
}

void PropertyEditorWidget::propertyChanged(QtProperty *p)
{
	nlinfo(QString("property %1 changed").arg(p->propertyName()).toUtf8().constData());
}

void PropertyEditorWidget::resetProperty(QtProperty *property)
{
	nlinfo(QString("property %1 reset").arg(property->propertyName()).toUtf8().constData());
}

NLLIGO::IProperty* PropertyEditorWidget::getLigoProperty( QtProperty *p )
{
	NLLIGO::IPrimitive *prim = d_ptr->getPrimitive( p );
	if( prim == NULL )
	{
		return NULL;
	}

	NLLIGO::IProperty *prop = NULL;
	prim->getPropertyByName( p->propertyName().toUtf8().constData(), prop );
	if( prop == NULL )
	{
		return NULL;
	}

	return prop;
}

void PropertyEditorWidget::onBoolValueChanged( QtProperty *p, bool v )
{
	NLLIGO::IProperty *prop = getLigoProperty( p );
	if( prop == NULL )
		return;

	NLLIGO::CPropertyString *pp = dynamic_cast< NLLIGO::CPropertyString* >( prop );
	if( pp == NULL )
		return;

	if( v )
		pp->String = "true";
	else
		pp->String = "false";

	pp->Default = false;
}

void PropertyEditorWidget::onStringValueChanged( QtProperty *p, const QString &v )
{
	NLLIGO::IProperty *prop = getLigoProperty( p );
	if( prop == NULL )
		return;

	NLLIGO::CPropertyString *pp = dynamic_cast< NLLIGO::CPropertyString* >( prop );
	if( pp == NULL )
		return;

	pp->String = v.toUtf8().constData();
	pp->Default = false;
}

void PropertyEditorWidget::onEnumValueChanged( QtProperty *p, int v )
{
	NLLIGO::IProperty *prop = getLigoProperty( p );
	if( prop == NULL )
		return;

	NLLIGO::CPropertyString *pp = dynamic_cast< NLLIGO::CPropertyString* >( prop );
	if( pp == NULL )
		return;

	pp->String = p->valueText().toUtf8().constData();
	pp->Default = false;
}

void PropertyEditorWidget::onStrArrValueChanged( QtProperty *p, const QString &v )
{
	NLLIGO::IProperty *prop = getLigoProperty( p );
	if( prop == NULL )
		return;

	NLLIGO::CPropertyStringArray *pp = dynamic_cast< NLLIGO::CPropertyStringArray* >( prop );
	if( pp == NULL )
		return;

	pp->StringArray.clear();

	QStringList l = v.split( '\n' );
	QStringListIterator itr( l );
	while( itr.hasNext() )
	{
		pp->StringArray.push_back( itr.next().toUtf8().constData() );
	}

	pp->Default = false;
}

void PropertyEditorWidget::onConstStrArrValueChanged( QtProperty *p, const QString &v )
{
	NLLIGO::IProperty *prop = getLigoProperty( p );
	if( prop == NULL )
		return;

	NLLIGO::CPropertyStringArray *pp = dynamic_cast< NLLIGO::CPropertyStringArray* >( prop );
	if( pp == NULL )
		return;

	pp->StringArray.clear();

	QStringList l = v.split( '\n' );
	QStringListIterator itr( l );
	while( itr.hasNext() )
	{
		pp->StringArray.push_back( itr.next().toUtf8().constData() );
	}

	pp->Default = false;
}


QtProperty *PropertyEditorWidget::addBoolProperty(const NLLIGO::IProperty *property,
		const NLLIGO::CPrimitiveClass::CParameter &parameter,
		const NLLIGO::IPrimitive *primitive)
{
	std::string value;
	std::string name = parameter.Name.c_str();
	primitive->getPropertyByName(name.c_str(), value);
	QtProperty *prop = m_boolManager->addProperty(name.c_str());
	// if (Default)
	{
		//DialogProperties->setDefaultValue (this, value);
		m_boolManager->setValue(prop, bool((value=="true")?1:0));
	}
	return prop;
}

QtProperty *PropertyEditorWidget::addConstStringProperty(const NLLIGO::IProperty *property,
		const NLLIGO::CPrimitiveClass::CParameter &parameter,
		const NLLIGO::IPrimitive *primitive)
{
	std::string value;
	std::string name = parameter.Name.c_str();

	// Get current value
	primitive->getPropertyByName(name.c_str(), value);

	// Create qt property
	QtProperty *prop = m_enumManager->addProperty(parameter.Name.c_str());

	QStringList listEnums = getComboValues(parameter);

	if (listEnums.isEmpty())
	{
		listEnums << QString(value.c_str()) + tr(" (WRN: Check leveldesign!)");
		m_enumManager->setEnumNames(prop, listEnums);
		m_enumManager->setValue(prop, 0);
		prop->setEnabled(false);
	}
	else
	{
		// TODO: check this logic
		if (parameter.DefaultValue.empty() || (parameter.DefaultValue[0].Name.empty()))
			listEnums.prepend("");

		// Fill qt property
		m_enumManager->setEnumNames(prop, listEnums);

		// Find index of current value
		for (int i = 0; i < listEnums.size(); i++)
		{
			if (value == std::string(listEnums[i].toUtf8().constData()))
			{
				m_enumManager->setValue(prop, i);
				break;
			}
		}
	}

	return prop;
}

QtProperty *PropertyEditorWidget::addStringProperty(const NLLIGO::IProperty *property,
		const NLLIGO::CPrimitiveClass::CParameter &parameter,
		const NLLIGO::IPrimitive *primitive)
{
	std::string value;
	std::string name = parameter.Name.c_str();
	primitive->getPropertyByName(name.c_str(), value);
	QtProperty *prop = m_stringManager->addProperty(parameter.Name.c_str());
	m_stringManager->setValue(prop, QString(value.c_str()));
	return prop;
}

QtProperty *PropertyEditorWidget::addStringArrayProperty(const NLLIGO::IProperty *property,
		const NLLIGO::CPrimitiveClass::CParameter &parameter,
		const NLLIGO::IPrimitive *primitive)
{
	std::string name = parameter.Name.c_str();
	QtProperty *prop = m_stringArrayManager->addProperty(parameter.Name.c_str());

	const NLLIGO::IProperty	*ligoProperty;
	std::vector<std::string> vectString;

	if	(primitive->getPropertyByName(parameter.Name.c_str (), ligoProperty))
	{
		const NLLIGO::CPropertyStringArray *const propStringArray = dynamic_cast<const NLLIGO::CPropertyStringArray *> (ligoProperty);
		if (propStringArray)
		{
			const std::vector<std::string> &vectString = propStringArray->StringArray;
			if (!vectString.empty())
			{
				std::string temp;
				for (size_t i = 0; i < vectString.size(); i++)
				{
					temp += vectString[i];
					if (i != (vectString.size() - 1))
						temp += '\n';
				}
				m_stringArrayManager->setValue(prop, temp.c_str());
				prop->setToolTip(temp.c_str());
			}
		}
		else
		{
			m_stringArrayManager->setValue(prop, "StringArray :(");
		}
	}

	// Create an "EDIT" button if the text is editable (FileExtension != "")
	if (parameter.FileExtension != "")
	{	
		// Create an edit box
		// TODO:
	}
	return prop;
}

QtProperty *PropertyEditorWidget::addConstStringArrayProperty(const NLLIGO::IProperty *property,
		const NLLIGO::CPrimitiveClass::CParameter &parameter,
		const NLLIGO::IPrimitive *primitive)
{
	std::string value;
	std::string name = parameter.Name.c_str();

	// Get current value
	primitive->getPropertyByName(name.c_str(), value);

	// Create qt property
	QtProperty *prop = m_constStrArrPropMgr->addProperty(parameter.Name.c_str());

	QStringList listEnums = getComboValues(parameter);

	if (listEnums.isEmpty())
	{
		prop->setEnabled(false);
	}
	else
	{
		// Fill qt property
		m_constStrArrPropMgr->setStrings(prop, listEnums);

		const NLLIGO::IProperty	*ligoProperty;
		std::vector<std::string> vectString;

		if	(primitive->getPropertyByName (parameter.Name.c_str(), ligoProperty))
		{
			const NLLIGO::CPropertyStringArray *const propStringArray = dynamic_cast<const NLLIGO::CPropertyStringArray *> (ligoProperty);
			if (propStringArray)
			{
				const std::vector<std::string> &vectString = propStringArray->StringArray;
				if (!vectString.empty())
				{
					std::string temp;
					for (size_t i = 0; i < vectString.size(); i++)
					{
						temp += vectString[i];
						if (i != (vectString.size() - 1))
							temp += '\n';
					}
					m_constStrArrPropMgr->setValue(prop, temp.c_str());
					prop->setToolTip(temp.c_str());
				}
			}
			else
			{
				m_constStrArrPropMgr->setValue(prop, "StringArray :(");
			}
		}

	}

	return prop;
}

QStringList PropertyEditorWidget::getComboValues(const NLLIGO::CPrimitiveClass::CParameter &parameter)
{
	// TODO: get context value from dialog
	std::string context("jungle");
	std::string defaultContext("default");

	std::vector<std::string> listContext;
	
	if (context != defaultContext)
		listContext.push_back(context);
	listContext.push_back(defaultContext);

	QStringList listEnums;

	// Correct fill properties with *both* contexts if the current context is not default and is valid.
	for (size_t j = 0; j < listContext.size(); j++)
	{
		std::map<std::string, NLLIGO::CPrimitiveClass::CParameter::CConstStringValue>::const_iterator ite = parameter.ComboValues.find(listContext[j].c_str());

		if (ite != parameter.ComboValues.end())
		{
			std::vector<std::string> pathList;

			// Fill pathList
			ite->second.appendFilePath(pathList);

			if (parameter.SortEntries)
				std::sort(pathList.begin(), pathList.end());

			for (size_t i = 0; i < pathList.size(); ++i)
				listEnums.append(pathList[i].c_str());
		}
	}

	return listEnums;
}

void PropertyEditorWidget::blockSignalsOfProperties(bool block)
{
	m_stringManager->blockSignals(block);
	m_boolManager->blockSignals(block);
	m_enumManager->blockSignals(block);
	m_stringArrayManager->blockSignals(block);
	
	if( block )
	{
		disconnect(m_constStrArrPropMgr, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));

		disconnect(m_boolManager, SIGNAL( valueChanged( QtProperty*, bool ) ),
			this, SLOT( onBoolValueChanged( QtProperty*, bool ) ) );
		
		disconnect(m_stringManager, SIGNAL( valueChanged( QtProperty*, const QString& ) ),
			this, SLOT( onStringValueChanged( QtProperty*, const QString& ) ) );

		disconnect(m_enumManager, SIGNAL( valueChanged( QtProperty*, int ) ),
			this, SLOT( onEnumValueChanged( QtProperty*, int ) ) );

		disconnect(m_stringArrayManager, SIGNAL( valueChanged( QtProperty*, const QString& ) ),
			this, SLOT( onStrArrValueChanged( QtProperty*, const QString& ) ) );

		disconnect(m_constStrArrPropMgr, SIGNAL( valueChanged( QtProperty*, const QString& ) ),
			this, SLOT( onConstStrArrValueChanged( QtProperty*, const QString& ) ) );
	}
	else
	{
		connect(m_constStrArrPropMgr, SIGNAL(propertyChanged(QtProperty *)), this, SLOT(propertyChanged(QtProperty *)));

		connect(m_boolManager, SIGNAL( valueChanged( QtProperty*, bool ) ),
			this, SLOT( onBoolValueChanged( QtProperty*, bool ) ) );

		connect(m_stringManager, SIGNAL( valueChanged( QtProperty*, const QString& ) ),
			this, SLOT( onStringValueChanged( QtProperty*, const QString& ) ) );

		connect(m_enumManager, SIGNAL( valueChanged( QtProperty*, int ) ),
			this, SLOT( onEnumValueChanged( QtProperty*, int ) ) );

		connect(m_stringArrayManager, SIGNAL( valueChanged( QtProperty*, const QString& ) ),
			this, SLOT( onStrArrValueChanged( QtProperty*, const QString& ) ) );

		connect(m_constStrArrPropMgr, SIGNAL( valueChanged( QtProperty*, const QString& ) ),
			this, SLOT( onConstStrArrValueChanged( QtProperty*, const QString& ) ) );
	}
}
} /* namespace WorldEditor */
